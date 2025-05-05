#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/epoll.h>
#include "switch_control.h"

#define MAX_EVENTS 10

extern int open_epoll();
extern int file_polling(int epfd, int fd);
extern void add_watch(int epfd, int fd);

