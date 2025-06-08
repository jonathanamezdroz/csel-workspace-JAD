#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define COMM_FILE_PATH "/tmp/comm_file"
typedef enum {
    STATE_WAIT = -1,
    STATE_EXIT = 0,
    STATE_MODE_AUTO = 1,
    STATE_MODE_MANUAL = 2,
    STATE_INCREASE_FREQUENCY = 3,
    STATE_DECREASE_FREQUENCY = 4
} State;

int read_int_from_keyboard(){
    int value = -1;
    while ((scanf("%d", &value) != 1) || (value < 0) || (value > 4)) {
        printf("Invalid input ");
        while(getchar() != '\n'); // clear the input buffer
    }
    return value;
}

int write_file(char* buffer){
    int fd = open(COMM_FILE_PATH, O_WRONLY | O_TRUNC, 0644); // open, and truncate file
    if (fd < 0) {
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

void print_menu(){
    printf("\n");
    printf("---------------------------------\n");
    printf("-------CSEL - mini projet-------\n");
    printf("----------user menu-------------\n");
    printf("Enter a number to choose an option\n");
    printf("0: EXIT\n");
    printf("1: Automatic mode\n");
    printf("2: Manual mode\n");
    printf("3: Increase frequency\n");
    printf("4: Decrease frequency\n");
    printf("---------------------------------\n");
    printf("Choose an option: ");
}

int main(){
    State user_input = STATE_WAIT;
    char *user_input_str = NULL;
    
    mkfifo(COMM_FILE_PATH, 0666);

    while(1){
        switch (user_input){
            case STATE_WAIT:
                print_menu();
                user_input = read_int_from_keyboard();
                user_input_str = NULL; // Reset the string for new input
                break;
            case STATE_EXIT:
                printf("Exiting\n");
                return 0;
            case STATE_MODE_AUTO:
                printf("Mode automatic\n");
                user_input_str = "automatic";
                user_input = STATE_WAIT;
                break;
            case STATE_MODE_MANUAL:
                printf("Mode manual\n");
                user_input_str = "manual";
                user_input = STATE_WAIT;
                break;
            case STATE_INCREASE_FREQUENCY:
                printf("Increase frequency\n");
                user_input_str = "higher";
                user_input = STATE_WAIT;
                break;
            case STATE_DECREASE_FREQUENCY:
                printf("Decrease frequency\n");
                user_input_str = "lower";
                user_input = STATE_WAIT;
                break;
            default:
                printf("Invalid option. Please try again.\n");
                user_input = STATE_WAIT;
                break;
        }

        // Write the user input to the communication file
        if (user_input_str != NULL) {
            if (write_file(user_input_str) < 0) {
                fprintf(stderr, "Failed to write user input to communication file\n");
            } else {
                printf("User input '%s' written to communication file\n", user_input_str);
            }
            user_input_str = NULL; // Reset the string after writing
        }

    }

}
