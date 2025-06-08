#include "process.h"

//--- epoll approach starts here

static int MAX_EVENT_FOR_SINGLE_LOOP = 8;
static int EPOLL_WAIT_TIMEOUT = -1; // wait forever until an event occurs
#define READ_BUFFER_SIZE 128

static void add_to_epoll(int epoll_fd, int fd, uint32_t flags){
    long tmp_long;
    struct epoll_event events = {
        .events = flags,
        .data.fd = fd
    };
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &events) < 0){
        perror("epoll_ctl");
        close(epoll_fd);
    }else{
        //consume any prior event
        read(fd, &tmp_long, sizeof(tmp_long));
    }
}

void epoll_process(){
    
    char buf;
    int power_led_fd, k1_fd, k2_fd, k3_fd;
    int user_comm_fd;
    int timer_led_fd, timer_polling_fd;
    int i, epoll_status, tmp_fd, tmp_int;
    long tmp_long;
    struct epoll_event events[MAX_EVENT_FOR_SINGLE_LOOP];
    int epoll_fd = epoll_create1(0);
    char* mode_string="null"; 
    Mode current_mode = MANUAL_MODE; // Default mode is manual
    char* speed="null"; // "higer" or "lower"
    char read_buffer[READ_BUFFER_SIZE];
    char user_input_buffer[READ_BUFFER_SIZE];
    char temperature[16], mode[16], speed_value[16];
    // create fd and define associated events


    ssd1306_init();
    if(read_device(read_buffer) == 0){
        sscanf(read_buffer, "%15[^,], %15[^,], %15s", temperature, mode, speed_value);
        mode_string = mode; // Use the mode read from the device
        current_mode = (strcmp(mode, "manual") == 0) ? MANUAL_MODE : AUTOMATIC_MODE;
    }
    else{
        syslog(LOG_ERR, "Failed to read initial device state");
    }
    

    //switch K1
    k1_fd = open_switch(K1, GPIO_K1);
    add_to_epoll(epoll_fd, k1_fd, EPOLLPRI);
    //switch K2
    k2_fd = open_switch(K2, GPIO_K2);
    add_to_epoll(epoll_fd, k2_fd, EPOLLPRI);
    //switch K3
    k3_fd = open_switch(K3, GPIO_K3);
    add_to_epoll(epoll_fd, k3_fd, EPOLLPRI);

    //timer to blink the led after presses
    timer_led_fd = start_timer(0, 0);
    add_to_epoll(epoll_fd, timer_led_fd, EPOLLIN | EPOLLPRI);
    timer_polling_fd = start_timer(POLLING_PERIOD, 0);
    add_to_epoll(epoll_fd, timer_polling_fd, EPOLLIN | EPOLLPRI);

    //open IPC with user process
    mkfifo(COMM_FILE_PATH, 0666);
    user_comm_fd = open(COMM_FILE_PATH, O_RDONLY| O_NONBLOCK);
    add_to_epoll(epoll_fd, user_comm_fd, EPOLLPRI);

    //led fd
    power_led_fd = open_led(POWER_LED, GPIO_POWER_LED);

    while(1){
        epoll_status = epoll_wait(epoll_fd, events, MAX_EVENT_FOR_SINGLE_LOOP, EPOLL_WAIT_TIMEOUT);
        if(epoll_status > 0){
            for(i=0;i<epoll_status;i++){
                tmp_fd = events[i].data.fd;
                // execute behaviour for fd
                if((tmp_fd == k1_fd) || (tmp_fd == k2_fd) || (tmp_fd == k3_fd)){
                    lseek(tmp_fd, 0, SEEK_SET);
                    read(tmp_fd, &buf, 1);
                    
                    //reset if k2, reduce period if k1, increase period if k3
                    if(tmp_fd == k1_fd){
                        syslog(LOG_NOTICE, "increase fan frequency\n");
                        write(power_led_fd, "1", 1); // notifiy user of button press
                        update_timer(timer_led_fd, LED_ON_TIME, 0);
                        speed = "higher";
                        write_device(speed);

                    }else if(tmp_fd == k2_fd){
                        syslog(LOG_NOTICE, "lower fan frequency\n");
                        write(power_led_fd, "1", 1); // notifiy user of button press
                        update_timer(timer_led_fd, LED_ON_TIME, 0);
                        speed = "lower";
                        write_device(speed);
                    }else{
                        syslog(LOG_NOTICE, "change mode\n");
                        write(power_led_fd, "1", 1); // notifiy user of button press
                        update_timer(timer_led_fd, LED_ON_TIME, 0);
                        // Change current mode
                        current_mode = (current_mode == MANUAL_MODE) ? AUTOMATIC_MODE : MANUAL_MODE;
                        
                        if (current_mode == MANUAL_MODE){
                            mode_string = "manual";
                        } else {
                            mode_string = "automatic";
                        }
                        write_device(mode_string);
                    }
                    // Clear display after each button press
                    ssd1306_clear_display();
                } 
                else if (tmp_fd == user_comm_fd){ 
                    tmp_int = read_user_comm(user_comm_fd, user_input_buffer);
                    if(tmp_int > 0){
                        write_device(user_input_buffer);
                        
                        if(strcmp(user_input_buffer, "manual") == 0){
                            current_mode = MANUAL_MODE;
                            mode_string = "manual";
                        }
                        else if(strcmp(user_input_buffer, "automatic") == 0){
                            current_mode = AUTOMATIC_MODE;
                            mode_string = "automatic";
                        }
                        else{
                            current_mode = current_mode;
                            mode_string = mode_string;
                        }
                        ssd1306_clear_display();
                    }else if(0 == tmp_int){
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, user_comm_fd, 0);
                        close(user_comm_fd);
                        user_comm_fd = open(COMM_FILE_PATH, O_RDONLY | O_NONBLOCK);
                        add_to_epoll(epoll_fd, user_comm_fd, EPOLLPRI);
                    }
                }
                else if(tmp_fd == timer_led_fd){
                    read(tmp_fd, &tmp_long, sizeof(tmp_long));
                    write(power_led_fd, "0", 1);
                    update_timer(timer_led_fd, 0, 0);
                    //printf("timer led off\n");
                    // Need to be fixed, timer never gets its actual value
                }
                else if(tmp_fd == timer_polling_fd){
                    read(tmp_fd, &tmp_long, sizeof(tmp_long));
                    
                    ssd1306_set_position (0,0);
                    ssd1306_puts("- mini_projet -");
                    ssd1306_set_position (1,1);
                    ssd1306_puts("- AB et JAD -");
                    ssd1306_set_position (0,2);
                    ssd1306_puts("---------------");
                    if(read_device(read_buffer) == 0){
                        sscanf(read_buffer, "%15[^,], %15[^,], %15s", temperature, mode, speed_value);
                        char tmp_buf[100];
                        snprintf(tmp_buf, 100, "Temp: %sC", temperature);
                        ssd1306_set_position (0,4);
                        ssd1306_puts(tmp_buf);

                        snprintf(tmp_buf, 50, "Mode: %s", mode);
                        ssd1306_set_position (0,5);
                        ssd1306_puts(tmp_buf);

                        snprintf(tmp_buf, 50, "Freq: %sHz", speed_value);
                        ssd1306_set_position (0,6);
                        ssd1306_puts(tmp_buf);
                    }
                    else{
                        syslog(LOG_ERR, "Failed to read initial device state");
                        ssd1306_set_position (0,4);
                        ssd1306_puts("Failed to");
                        ssd1306_set_position (0,5);
                        ssd1306_puts("read device");
                    }
                }
                else{
                    break;
                }
            }
            // manage event here

        }else if(0 == epoll_status){
            //timeout
        }else{
            if( EINTR == errno ){
                // ignorable errors
                continue;
            }else{
                // other errors
                break;
            }
        }
    }
    // good practice would require to unregister file descriptors and close them
    return;
}

//--- epoll approache ends here
