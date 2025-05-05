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
        .events = EPOLLIN | EPOLLET,
        .data.fd = fd,
    };

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1) {
        perror("epoll_ctl");
        close(epfd);
    }
}

// Fonction pour surveiller les modifications d'un fichier
int file_polling(int epfd) {

    struct epoll_event events[2]; //tableau pour stocker les événements
    int nr = epoll_wait(epfd, events, 2, -1);
    if (nr == -1)
        /* error*/
        perror("epoll_wait");
    for (int i=0; i<nr; i++) {
        printf ("event=%u on fd=%d\n", events[i].events, events[i].data.fd);
        // operation on events[i].data.fd can be performed without blocking...
    }
    return 0;
    close(epfd);
}