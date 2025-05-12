#include "process.h"

static int DUTY_CYCLE_ON = 50;

//--- silly approach starts here

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

//--- silly approach ends here
//--- select approach starts here

static int get_max_fd(int nb_fd, ...){
    int ret = -1;   //expecting any fd >= 1
    int i, tmp;
    va_list fds;
    va_start(fds, nb_fd);
    for(i = 0;i<nb_fd;i++){
        tmp = va_arg(fds, int);
        if(ret < tmp){
            ret = tmp;
        }else{}
    }
    va_end(fds);
    return ret;
}

static void prepare_select_loop(fd_set *select_fd, int nb_fd, ...){
    int i, tmp;
    FD_ZERO(select_fd);
    va_list fds;
    va_start(fds, nb_fd);
    for(i = 0;i<nb_fd;i++){
        tmp = va_arg(fds, int);
        FD_SET(tmp, select_fd);
    }
    va_end(fds);
}

void select_process(long period){
    long default_period = period;
    long current_period = default_period;
    long delta_period = default_period * 10 / 100;  //10% of default period
    int select_status;
    int timer_on_fd, timer_off_fd, led_fd, k1_fd, k2_fd, k3_fd, largest_fd;
    char buf;
    long tmp_long;
    fd_set fd_in;
    struct timeval timeout = {.tv_sec = 5, .tv_usec = 0};
    FD_ZERO(&fd_in);
    // create fd and define associated events
    //switch K1
    k1_fd = open_switch(K1, GPIO_K1);
    lseek(k1_fd, 0, SEEK_SET);
    read(k1_fd, &buf, 1);
    //switch K2
    k2_fd = open_switch(K2, GPIO_K2);
    lseek(k2_fd, 0, SEEK_SET);
    read(k2_fd, &buf, 1);
    //switch K3
    k3_fd = open_switch(K3, GPIO_K3);
    lseek(k3_fd, 0, SEEK_SET);
    read(k3_fd, &buf, 1);
    //timer to activate the led
    timer_on_fd = start_timer(current_period, 0);
    //timer to deactivate the led
    timer_off_fd = start_timer(current_period, DUTY_CYCLE_ON);
    //led fd
    led_fd = open_led();
    largest_fd = get_max_fd(5, timer_on_fd, timer_off_fd, k1_fd, k2_fd, k3_fd);
    while(1){
        prepare_select_loop(&fd_in, 5, timer_on_fd, timer_off_fd, k1_fd, k2_fd, k3_fd);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        select_status = select(largest_fd+1, &fd_in, NULL, NULL, &timeout);
        if(select_status < 0){
            syslog(LOG_NOTICE, "error occured with select\n");
            break;
        }else if(0 == select_status){
            //timeout
        }else{
            if(FD_ISSET(timer_on_fd, &fd_in)){
                read(timer_on_fd, &tmp_long, sizeof(tmp_long));
                write(led_fd, "1", 1);
            }else{}
            if(FD_ISSET(timer_off_fd, &fd_in)){
                read(timer_off_fd, &tmp_long, sizeof(tmp_long));
                write(led_fd, "0", 1);
            }else{}
            if(FD_ISSET(k1_fd, &fd_in)){
                lseek(k1_fd, 0, SEEK_SET);
                read(k1_fd, &buf, 1);
                current_period -= delta_period;
                update_timer(timer_on_fd, current_period, 0);
                update_timer(timer_off_fd, current_period, DUTY_CYCLE_ON);
            }else{}
            if(FD_ISSET(k2_fd, &fd_in)){
                lseek(k2_fd, 0, SEEK_SET);
                read(k2_fd, &buf, 1);
                lseek(k2_fd, 0, SEEK_SET);
                current_period = default_period;
                update_timer(timer_on_fd, current_period, 0);
                update_timer(timer_off_fd, current_period, DUTY_CYCLE_ON);
            }else{}
            if(FD_ISSET(k3_fd, &fd_in)){
                lseek(k3_fd, 0, SEEK_SET);
                read(k3_fd, &buf, 1);
                current_period += delta_period;
                update_timer(timer_on_fd, current_period, 0);
                update_timer(timer_off_fd, current_period, DUTY_CYCLE_ON);
            }else{}
        }
    }
    //unregister fd and close them
    return;
}

//--- select approach ends here
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

void epoll_process(long period){
    long default_period = period;   //ns
    long current_period = default_period;
    long delta_period = default_period * 10 / 100;  //10% of default period
    char buf;
    int led_status;
    int timer_on_fd, timer_off_fd, led_fd, k1_fd, k2_fd, k3_fd;
    int i, epoll_status, tmp_fd;
    long tmp_long;
    struct epoll_event events[MAX_EVENT_FOR_SINGLE_LOOP];
    int epoll_fd = epoll_create1(0);
    // create fd and define associated events
    //switch K1
    k1_fd = open_switch(K1, GPIO_K1);
    add_to_epoll(epoll_fd, k1_fd, EPOLLPRI);
    //switch K2
    k2_fd = open_switch(K2, GPIO_K2);
    add_to_epoll(epoll_fd, k2_fd, EPOLLPRI);
    //switch K3
    k3_fd = open_switch(K3, GPIO_K3);
    add_to_epoll(epoll_fd, k3_fd, EPOLLPRI);
    //timer to activate the led
    timer_on_fd = start_timer(current_period, 0);
    add_to_epoll(epoll_fd, timer_on_fd, EPOLLIN | EPOLLPRI);
    //timer to deactivate the led
    timer_off_fd = start_timer(current_period, DUTY_CYCLE_ON);
    add_to_epoll(epoll_fd, timer_off_fd, EPOLLIN | EPOLLPRI);
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

//--- epoll approache ends here
