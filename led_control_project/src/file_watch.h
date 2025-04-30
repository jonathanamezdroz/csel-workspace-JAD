#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>

extern int surveiller_fichier(const char *filename);

