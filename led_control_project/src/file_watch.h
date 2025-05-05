#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/epoll.h>

extern int open_epoll();
extern int file_polling(int epfd);
extern void add_watch(int epfd, int fd);

