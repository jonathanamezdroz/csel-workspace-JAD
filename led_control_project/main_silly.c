#include <stdlib.h>
#include <stdio.h>

#include "led_control.h"
#include "switch_control.h"

#include "process.h"

int main(int argc, char** argv){
    printf("hello there\n");
    long period=1000;
    if(argc>=2){
        period = atoi(argv[1]);
    }else{}
    silly_process(period);
    printf("have a nice day\n");
    return 0;
}
