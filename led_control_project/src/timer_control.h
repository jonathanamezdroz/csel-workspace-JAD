#ifndef TIMER_H
#define TIMER_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/timerfd.h>

//https://stackoverflow.com/questions/63241720/how-to-use-timerfd-properly
int start_timer(long period, int delay_percentage);
int update_timer(int fd, long period, int delay_percentage);

#endif // TIMER_H
