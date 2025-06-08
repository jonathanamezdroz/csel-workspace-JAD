#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "led_control.h"
#include "switch_control.h"

#include "process.h"
void daemonize(){
    pid_t pid = fork(); 
    if (pid == 0){
        setsid(); // set child process as session leader
        umask(0); // clear file mode creation mask
        chdir("/"); // change working directory to root
        close(pid);
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO); // redirect stdin to /dev/null
        dup2(fd, STDOUT_FILENO); // redirect stdout to /dev/null
        dup2(fd, STDERR_FILENO); // redirect stderr to /dev/null  
        close(fd); 
    }
    else{
        //exit parent process
        exit(EXIT_FAILURE); 
    }
}

int main(int argc, char** argv){
    // Create daemon
    daemonize();
    printf("hello there\n");
    setlogmask(LOG_UPTO(LOG_NOTICE));   //discards info and debug
    openlog("app_epoll", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    long period=0;
    if(argc>=2){
        period = atoi(argv[1]) * 1000000;
    }else{
        period = 50000000; // default to 50ms
    }
    printf("period: %ld\n", period);


    epoll_process();
    printf("have a nice day\n");
    return 0;
}
