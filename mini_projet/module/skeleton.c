// skeleton.c
#include <linux/module.h>       // needed by all modules
#include <linux/init.h>         // macros
#include <linux/kernel.h>       // debugging and vsnprintf
#include <linux/moduleparam.h>  // module/driver parameters
#include <linux/cdev.h>         // char driver
#include <linux/fs.h>           // driver
#include <linux/uaccess.h>      // copy data to/from the user
#include <linux/ioport.h>	    // memory region handling
#include <linux/io.h>		    // mmio handling
#include <linux/kthread.h>      // thread
#include <linux/semaphore.h>    // semaphore
#include <linux/delay.h>        // ssleep
#include <linux/gpio.h>         // led management

// led management example can be found here : 
//  https://forum.allaboutcircuits.com/threads/kernel-module-for-fleshing-led.143831/

//--- global/static values

static const int NOT_FOUND = 0;
static const int IS_FOUND = 1;

//note: below are enum and array linked to them. While this construction is 
//      a bit of tinkering in C (because modification requires extra care
//      to not miss an enum element), it becomes trivial and 
//      straightforward when using map (in C++ for instance)

typedef enum{
    automatic = 0,
    manual,
    state_last, //iteration purpose
}state;
char* const STATE_NAME[] = {
    "automatic",
    "manual"
};
static state current_state = automatic;

typedef enum{
    BELOW_35 = 0,
    BELOW_40,
    BELOW_45,
    ABOVE_45,
    temp_class_last,    //iteration purpose
}temp_class;
static temp_class current_temp_class = BELOW_35;
//implicitly linked to temp_class enum
static int FREQUENCY[] = {
    2, 5, 10, 20
};

typedef enum{
    lower = 0,
    higher,
    manual_input_last,  //iteration_purpose
}manual_input;
char* const MANUAL_INPUT_NAME[] = {
    "lower",
    "higher"
};

static struct semaphore sema_auto;
static struct task_struct* automatic_thread;
static struct task_struct* blink_thread;
static const int AUTOMATIC_SLEEP_TIME = 1;  // [s]
//implicitly linked to temp_class enum
static const int BLINK_DELAY_SLEEP[] = {
    500, 200, 100, 50
};
static const int GPIO_LED = 10;

#define             BUFFER_MAX_SZ 1000
static char tmp_str[BUFFER_MAX_SZ];
static const char*  READ_FORMAT = "%d.%d, %s, %d\n"; //processor_temp (2 decimal precision), current_mode, frequency
static const long TEMP_ADDR_START = 0x01c25000;
static const int TEMP_ADDR_CONF = 0x1000;

static struct resource* res = 0;
static unsigned char* reg = 0;
static int current_temp_i = -50;   //small enough to be obviously wrong
static int current_temp_f = 0;

static dev_t skeleton_dev;
static struct cdev skeleton_cdev;

//--- utils

//from : https://stackoverflow.com/a/12264427
int snprintf(char *buf, size_t size, const char *fmt, ...){
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return i;
}

//from : https://stackoverflow.com/q/17559996
int strcmp(const char* s1, const char* s2){
    while(*s1 && (*s1==*s2)){
        s1++;
        s2++;
    }
    return (*(const unsigned char*)s1-*(const unsigned char*)s2);
}

//--- modules dedicated functions and values

static void set_current_temp_class(int new_temp_class){
    current_temp_class = new_temp_class;
}

static int interpret_change_state(void){
    int ret = NOT_FOUND;
    int input;
    for(input=0;input<state_last;input++){
        if(0 == strcmp(STATE_NAME[input], tmp_str)){
            if(input != current_state){
                current_state = input;
                if(manual == input){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
// rationale :  if result is 0, great news. Otherwise it simply mean that the module is removed.
//              Either way we can handle ignoring the check of value
                    down_interruptible(&sema_auto);
#pragma GCC diagnostic pop
                }else if(automatic == input){   //coule be just "else", explicit for future features
                    up(&sema_auto);
                }else{}
            }else{}
            ret = IS_FOUND;
            break;
        }
    }
    return ret;
}

static int interpret_manual_input(void){
    int ret = NOT_FOUND;
    int input;
    for(input=0;input<manual_input_last;input++){
        if(0 == strcmp(MANUAL_INPUT_NAME[input], tmp_str)){
            switch(input){
                case lower:
                    ret = IS_FOUND;
                    //if allowed to decrease
                    if(current_temp_class != BELOW_35){
                        set_current_temp_class(current_temp_class-1);
                    }else{}
                    break;
                case higher:
                    ret = IS_FOUND;
                    //if allowed to increase
                    if(current_temp_class != ABOVE_45){
                        set_current_temp_class(current_temp_class+1);
                    }else{}
                    break;
                default:
                    ret = NOT_FOUND;
                    break;
            }
            break;
        }else{}
    }
    return ret;
}

static int interpret_automatic_input(void){
    int ret = NOT_FOUND;
    //no automatic input, leaving the function for future modifications
    return ret;
}

static void interpret_input(void){
    int status = interpret_change_state();
    if(NOT_FOUND == status){
        switch(current_state){
            case automatic:
                status = interpret_automatic_input();
                break;
            case manual:
                status = interpret_manual_input();
                break;
            default:
                break;
        }
    }else{}
    if(NOT_FOUND == status){
    }else{}
    return;
}

static void update_temp(void){
    long tmp;
    if(0 != reg){
        tmp = ((-1191 * (int)ioread32(reg+0x80)) / 10) + 223000;
        current_temp_f = (tmp/10) % 100;
        current_temp_i = (tmp/1000);
    }else{} 
    return;
}

static int thread_auto_func(void* data){
    int tmp_class;
    while(!kthread_should_stop()){
        if(0 == down_interruptible(&sema_auto)){
            update_temp();
            //mapping between {35, 40, 45, above} to {0, 1, 2, 3}
            tmp_class = (current_temp_i-35)/5;
            set_current_temp_class(tmp_class < temp_class_last ? tmp_class : (temp_class_last-1));
            up(&sema_auto);
            ssleep(AUTOMATIC_SLEEP_TIME);
        }else{}
    }
    return 0;
}

static int thread_blink_func(void* data){
    static int led_status = 0;
    while(!kthread_should_stop()){
        led_status = ((led_status + 1) & 0x1);
        gpio_set_value(GPIO_LED, led_status);
        msleep(BLINK_DELAY_SLEEP[current_temp_class]);
    }    
    return 0;
}

void init_led(void){
    gpio_request(GPIO_LED, "sysfs");
    gpio_direction_output(GPIO_LED, true);
    gpio_export(GPIO_LED, false);   //prevents gpio direction from being changed
    return;
}

void release_led(void){
    gpio_set_value(GPIO_LED, 0);
    gpio_unexport(GPIO_LED);
    gpio_free(GPIO_LED);
    return;
}

//--- driver functions

//does nothing
static int skeleton_open(struct inode* i, struct file* f){
    return 0;
}

//does nothing
static int skeleton_release(struct inode* i, struct file* f){
    return 0;
}

static ssize_t skeleton_read(
    struct file* f,
    char __user* buf, 
    size_t count,
    loff_t* off
){
    int nb_char = -EFAULT;

    ssize_t remaining = BUFFER_MAX_SZ - (ssize_t)(*off);                                                                    
    char* ptr         = tmp_str + *off;                                                                                
    if (count > remaining) count = remaining;                                                                           
    *off += count;

    if(manual == current_state){
        update_temp();
    }else{}
    nb_char = snprintf(
        tmp_str, 
        BUFFER_MAX_SZ, 
        READ_FORMAT, 
        current_temp_i, 
        current_temp_f,
        STATE_NAME[current_state], 
        FREQUENCY[current_temp_class]
    );

    if(0 != copy_to_user(buf, ptr, nb_char)){
        nb_char = -EFAULT;
    }else{}
    return count;
}

static ssize_t skeleton_write(struct file*f, const char __user* buf, size_t count, loff_t* off){
    ssize_t remaining = BUFFER_MAX_SZ - (ssize_t)(*off);
    if(count >= remaining){
        count = -EIO;
    }else{}
    if(count > 0){
        char* ptr = tmp_str + *off;
        *off += count;
        ptr[count] = 0; //ensures that last char is \0
        if(copy_from_user(ptr, buf, count)){
            count = -EFAULT;
        }else{} 
        //removing trailing newline if any
        if(ptr[count-1] == '\n'){
            ptr[count-1] = '\0';
        }else{}
        interpret_input();
    }else{}
    return count;
}

static struct file_operations skeleton_fops = {
    .owner = THIS_MODULE,
    .open = skeleton_open,
    .read = skeleton_read,
    .write = skeleton_write,
    .release = skeleton_release,
};

//--- module functions

// init module (modprobe, insmod, ...)
static int __init skeleton_init(void){
    int status;
    res = request_mem_region(TEMP_ADDR_START, TEMP_ADDR_CONF, "allwinner h5 ths");
    reg = ioremap(TEMP_ADDR_START, TEMP_ADDR_CONF);
    if(0 == reg){
    }else{}

    status = alloc_chrdev_region(&skeleton_dev, 0, 1, "mini_project");
    if(0 == status){
        cdev_init(&skeleton_cdev, &skeleton_fops);
        skeleton_cdev.owner = THIS_MODULE;
        status = cdev_add(&skeleton_cdev, skeleton_dev, 1);
        sema_init(&sema_auto, 1);
        init_led();
        automatic_thread = kthread_run(thread_auto_func, 0, "s/thread");
        blink_thread = kthread_run(thread_blink_func, 0, "s/thread");
    }else{}

    return 0;
}

// exit module (rmmod, ...)
static void __exit skeleton_exit(void){
    up(&sema_auto);
    kthread_stop(automatic_thread);
    kthread_stop(blink_thread);
    release_led();
    if(0 != res){
        release_mem_region(TEMP_ADDR_START, TEMP_ADDR_CONF);
    }else{}
    cdev_del(&skeleton_cdev);
    unregister_chrdev_region(skeleton_dev, 1);
    return;
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Adrien Balleyguier <adrien.balleyguier@hefr.ch>");
MODULE_DESCRIPTION("prime computation skeleton");
MODULE_LICENSE("GPL");
