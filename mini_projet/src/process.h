#ifndef PROCESS_H
#define PROCESS_H
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <stdarg.h>

#include "led_control.h"
#include "switch_control.h"
#include "timer_control.h"
#include "ssd1306.h"
#include "module_com.h"

#define LED_ON_TIME 50000000
#define POLLING_PERIOD 10000


// Define an enum for mode and mode_flag
typedef enum {
    MANUAL_MODE = 0,
    AUTOMATIC_MODE = 1
} Mode;

void silly_process(long period);
void epoll_process();
void select_process(long period);

#endif // PROCESS_H