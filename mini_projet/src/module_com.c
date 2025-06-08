#include "module_com.h"


/*outpout is
"tmp, mode, led_freq[Hz]"
"36.84, manual, 2"
*/
int read_device(char* buffer){
    int fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    ssize_t bytes_read = read(fd, buffer, 128);
    if (bytes_read < 0) {
        perror("Failed to read from device");
        close(fd);
        return -1;

    }
    close(fd);
    return 0;
}


int write_device(char* buffer){
    int fd = open(DEVICE_PATH, O_WRONLY);
    if (fd <0 ){
        perror("Failed to open device for writing");
        return -1;
    }
    ssize_t bytes_write = write(fd, buffer, strlen(buffer));
    if (bytes_write < 0) {
        perror("Failed to write to device");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int read_user_comm(int fd, char* buffer){
    ssize_t bytes_read = read(fd, buffer, 128);
    buffer[bytes_read] = 0;
    if (bytes_read < 0) {
        perror("Failed to read from device");
        return -1;
    }
    return (int)(bytes_read);
}
