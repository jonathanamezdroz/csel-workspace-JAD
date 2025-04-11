#include <stdio.h>

#include "silly_led_control.h"
#include "switch_control.h"
#include "process.h"

int main(int argc, char** argv){
    printf("hello there\n");
    long period=1000;
    if(argc>=2){
        period = atoi(argv[1]);
    }else{}
    led_process(period);
    printf("have a nice day\n");
    return 0;
}
