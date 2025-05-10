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
    period *= 1000000;  // in ns

    int timer_on_fd, timer_off_fd;
    int i, epoll_status, tmp_fd;
    long tmp_long;
    struct epoll_event tmp_event;
    struct epoll_event events[MAX_EVENT_FOR_SINGLE_LOOP];
    int epoll_fd = epoll_create1(0);
    // create fd and define associated events
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, new_fd_events);
    //timer to activate the led
    timer_on_fd = start_timer(period, 0);
    tmp_event.events = (EPOLLIN|EPOLLPRI);
    tmp_event.data.fd = timer_on_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_on_fd, &tmp_event);
    //timer to deactivate the led
    timer_off_fd = start_timer(period, DUTY_CYCLE_ON);
    tmp_event.events = (EPOLLIN|EPOLLPRI);
    tmp_event.data.fd = timer_off_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_off_fd, &tmp_event);
    while(1){
        epoll_status = epoll_wait(epoll_fd, events, MAX_EVENT_FOR_SINGLE_LOOP, EPOLL_WAIT_TIMEOUT);
        if(epoll_status > 0){
            for(i=0;i<epoll_status;i++){
                tmp_fd = events[i].data.fd;
                // execute behaviour for fd
                if((tmp_fd == timer_on_fd) || (tmp_fd == timer_off_fd)){
                    read(tmp_fd, &tmp_long, sizeof(tmp_long));
                }else{
                    break;
                }
                //syslog(LOG_NOTICE, "event for %d fd id", tmp_fd);
                printf("event for %d\n", tmp_fd);
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
    // good practice would require to unregister file descriptors and free them
    return;
}
