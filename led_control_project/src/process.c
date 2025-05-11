#include "process.h"

void silly_process(long period){
    long duty   = 2;    
    period *= 1000000;  // in ns

    // compute duty period...
    long p1 = period / 100 * duty;
    long p2 = period - p1;

    int led = open_led();
    pwrite(led, "1", sizeof("1"), 0);

    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    int k = 0;
    while (1) {
        struct timespec t2;
        clock_gettime(CLOCK_MONOTONIC, &t2);

        long delta =
            (t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec);

        int toggle = ((k == 0) && (delta >= p1)) | ((k == 1) && (delta >= p2));
        if (toggle) {
            t1 = t2;
            k  = (k + 1) % 2;
            if (k == 0)
                pwrite(led, "1", sizeof("1"), 0);
            else
                pwrite(led, "0", sizeof("0"), 0);
        }
    }
    return;
}

static int MAX_EVENT_FOR_SINGLE_LOOP = 8;
static int EPOLL_WAIT_TIMEOUT = -1; // wait forever until an event occurs
#define READ_BUFFER_SIZE 128
static int DUTY_CYCLE_ON = 50;

void epoll_process(long period){
    long default_period = period *= 1000000;  // in ns
    long current_period = default_period;
    long delta_period = default_period * 10 / 100;  //10% of default period
    char buf;
    int led_status;
    int timer_on_fd, timer_off_fd, led_fd, k1_fd, k2_fd, k3_fd;
    int i, epoll_status, tmp_fd;
    long tmp_long;
    struct epoll_event tmp_event;
    struct epoll_event events[MAX_EVENT_FOR_SINGLE_LOOP];
    int epoll_fd = epoll_create1(0);
    // create fd and define associated events
    tmp_event.events = EPOLLPRI;
    //switch K1
    k1_fd = open_switch(K1, GPIO_K1);
    tmp_event.data.fd = k1_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, k1_fd, &tmp_event);
    //switch K2
    k2_fd = open_switch(K2, GPIO_K2);
    tmp_event.data.fd = k2_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, k2_fd, &tmp_event);
    //switch K3
    k3_fd = open_switch(K3, GPIO_K3);
    tmp_event.data.fd = k3_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, k3_fd, &tmp_event);
    tmp_event.events = EPOLLIN | EPOLLPRI;
    //timer to activate the led
    timer_on_fd = start_timer(current_period, 0);
    tmp_event.data.fd = timer_on_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_on_fd, &tmp_event);
    //timer to deactivate the led
    timer_off_fd = start_timer(current_period, DUTY_CYCLE_ON);
    tmp_event.data.fd = timer_off_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_off_fd, &tmp_event);
    //led fd
    led_fd = open_led();
    led_status = 1;
    while(1){
        epoll_status = epoll_wait(epoll_fd, events, MAX_EVENT_FOR_SINGLE_LOOP, EPOLL_WAIT_TIMEOUT);
        if(epoll_status > 0){
            for(i=0;i<epoll_status;i++){
                tmp_fd = events[i].data.fd;
                // execute behaviour for fd
                if((tmp_fd == timer_on_fd) || (tmp_fd == timer_off_fd)){
                    read(tmp_fd, &tmp_long, sizeof(tmp_long));
                    write(led_fd, (led_status ? "1" : "0"), 1);
                    led_status = !led_status;
                }else if((tmp_fd == k1_fd) || (tmp_fd == k2_fd) || (tmp_fd == k3_fd)){
                    lseek(tmp_fd, 0, SEEK_SET);
                    read(tmp_fd, &buf, 1);
                    //reset if k2, reduce period if k1, increase period if k3
                    if(tmp_fd == k1_fd){
                        current_period -= delta_period;
                        syslog(LOG_NOTICE, "increasing blinking frequency\n");
                    }else if(tmp_fd == k2_fd){
                        current_period = default_period;
                        syslog(LOG_NOTICE, "reset blinking frequency\n");
                    }else{
                        current_period += delta_period;
                        syslog(LOG_NOTICE, "lowering blinking frequency\n");
                    }
                    update_timer(timer_on_fd, current_period, 0);
                    update_timer(timer_off_fd, current_period, DUTY_CYCLE_ON);
                }else{
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
