#include "timer_control.h"

static int NS_TO_S = 1000000;

int start_timer(long period, int delay_percentage){
    int ret = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec ts;
    if(ret != -1){
        ts.it_value.tv_sec = (period / NS_TO_S) * delay_percentage / 100;
        ts.it_value.tv_nsec = 1 + ((period % NS_TO_S) * delay_percentage / 100);
        ts.it_interval.tv_sec = period / NS_TO_S;
        ts.it_interval.tv_nsec = period % NS_TO_S;
        if(timerfd_settime(ret, 0, &ts, NULL) < 0){
            close(ret);
            return EXIT_FAILURE;
        }else{}
    }else{}
    return ret;
}

int update_timer(int fd, long period, int delay_percentage){
    struct itimerspec ts;
    ts.it_value.tv_sec = (period / NS_TO_S) * delay_percentage / 100;
    ts.it_value.tv_nsec = 1 + ((period % NS_TO_S) * delay_percentage / 100);
    ts.it_interval.tv_sec = period / NS_TO_S;
    ts.it_interval.tv_nsec = period % NS_TO_S;
    if(timerfd_settime(fd, 0, &ts, NULL) < 0){
        close(fd);
        return EXIT_FAILURE;
    }else{}
    return EXIT_SUCCESS;
}
