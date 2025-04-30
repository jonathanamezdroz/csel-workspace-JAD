#include <stdlib.h>
#include <stdio.h>

#include "silly_led_control.h"
#include "switch_control.h"

#include "process.h"



int main(int argc, char** argv){

    int switch_status;
    switch_status = read_switch(K1, GPIO_K1);
    printf("switch value on %s: %d\n",GPIO_K1, switch_status);
    close_switch(K1);
    printf("hello there\n");
    long period=1000;
    if(argc>=2){
        period = atoi(argv[1]);
    }else{}
    silly_process(period);
    printf("have a nice day\n");
    return 0;
}
