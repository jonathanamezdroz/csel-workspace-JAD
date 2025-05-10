#ifndef SILLY_LED_CONTROL_H
#define SILLY_LED_CONTROL_H
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_LED      "/sys/class/gpio/gpio10"
#define LED           "10"


int open_led(void);

#endif//SILLY_LED_CONTROL_H
