#ifndef PROCESS_H
#define PROCESS_H
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/epoll.h>

#include "led_control.h"
#include "switch_control.h"
#include "timer_control.h"

void silly_process(long period);
void epoll_process(long period);

#endif // PROCESS_H
