
#ifndef LED_CONTROL_H
#define LED_CONTROL_H
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_STATUS_LED      "/sys/class/gpio/gpio10"
#define GPIO_POWER_LED       "/sys/class/gpio/gpio362"
#define STATUS_LED    "10"
#define POWER_LED     "362"

int open_led(char *led, char *gpio_led);

#endif//LED_CONTROL_H