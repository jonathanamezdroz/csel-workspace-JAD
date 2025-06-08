
#ifndef MODULE_COM_H
#define MODULE_COM_H
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


#define DEVICE_PATH   "/dev/mini_project"
#define COMM_FILE_PATH "/tmp/comm_file"
#define READ_BUFFER_SIZE 128

/*outpout is
"tmp, mode, n"
*/
int read_device(char* buffer);

int write_device(char* buffer);

int read_user_comm(int fd, char* buffer);


#endif//MODULE_COM_H
