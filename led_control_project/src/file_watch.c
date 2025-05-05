#include "file_watch.h"
int open_epoll() {
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        return -1;
    }
    return epfd;
}

void add_watch(int epfd, int fd){
    struct epoll_event event = {
        .events = EPOLLPRI, // Edge-triggered
        .data.fd = fd,
    };

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1) {
        perror("epoll_ctl");
        close(epfd);
    }
    //read the file to clear any previous event
    read_switch(fd);
}

// Fonction pour surveiller les modifications d'un fichier
int file_polling(int epfd, int fd) {
    printf("polling file\n");
    struct epoll_event events[MAX_EVENTS]; //tableau pour stocker les événements
    int gpio_val = -1;
    int nr = epoll_wait(epfd, events, MAX_EVENTS, -1);
    if (nr == -1){
        /* error*/
        perror("epoll_wait");
    }
    
    for (int i=0; i<nr; i++) {
        printf ("event=%u on fd=%d\n", events[i].events, events[i].data.fd);
        //make sure event come from the correct fd
        if (events[i].data.fd == fd) { 
            printf("Event on fd %d\n", events[i].data.fd);
            gpio_val = read_switch(events[i].data.fd);
            break;
        }
        // operation on events[i].data.fd can be performed without blocking...
    }
    return gpio_val;
    
}