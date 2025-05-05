#include <stdlib.h>
#include <stdio.h>

#include "silly_led_control.h"
#include "switch_control.h"
#include "file_watch.h"

#include "process.h"



int main(int argc, char** argv){
    int fd_K1 = open_switch(K1, GPIO_K1);
    int fd_K2 = open_switch(K2, GPIO_K2);
    int switch_status;

    int epfd = open_epoll();
    //printf("epoll fd: %d\n", epfd);
    add_watch(epfd, fd_K1);
    add_watch(epfd, fd_K2);

    //printf("Value on %s: %d\n", GPIO_K1, read_switch(fd_K1));
    //printf("Value on %s: %d\n", GPIO_K2, read_switch(fd_K2));
    while(1){
        switch_status = file_polling(epfd, fd_K1);
        printf("switch value on %s: %d\n",GPIO_K1, switch_status);
        if(file_polling(epfd, fd_K2) == 1){
            printf("leaving loop\n");
            break;
        }
    }
    

    close_switch(K1);
    close_switch(K2);
    printf("hello there\n");
    long period=1000;
    if(argc>=2){
        period = atoi(argv[1]);
    }else{}
    silly_process(period);
    printf("have a nice day\n");
    return 0;
}
