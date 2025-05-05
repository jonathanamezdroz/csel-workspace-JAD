#include <stdlib.h>
#include <stdio.h>

#include "silly_led_control.h"
#include "switch_control.h"
#include "file_watch.h"

#include "process.h"



int main(int argc, char** argv){
    int fd_K1 = open_switch(K1, GPIO_K1);
    int fd_K2 = open_switch(K2, GPIO_K2);
    int fd_K3 = open_switch(K3, GPIO_K3);
   // int switch_status;

    int epfd = open_epoll();
    //printf("epoll fd: %d\n", epfd);
    add_watch(epfd, fd_K1);
    add_watch(epfd, fd_K2);
    add_watch(epfd, fd_K3);

    while(1){

        if(file_polling(epfd, fd_K1) == 1){
            printf("Increase freq\n");
        }
        else if(file_polling(epfd, fd_K2) == 1){
            printf("Reset freq \n");
        }
        else if(file_polling(epfd, fd_K3) == 1){
            printf("Lower freq \n");
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
