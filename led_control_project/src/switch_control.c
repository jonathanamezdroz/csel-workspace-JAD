/**
 * Copyright 2018 University of Applied Sciences Western Switzerland / Fribourg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Project: HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: System programming -  file system
 *
 * Purpose: NanoPi silly status led control system
 *
 * Author:  Jonathan Amez-Droz
 * Date:    11.04.2025
 */

#include "switch_control.h"

extern int open_switch(const char *pin, const char *gpio_path){
    
    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, pin, strlen(pin));
    close(f);

    // config pin
    char gpio_dir[50];
    snprintf(gpio_dir, 50, "%s/direction", gpio_path);
    f = open(gpio_dir, O_WRONLY);
    write(f, "in", 3);
    close(f);

    char gpio_value[50];
    snprintf(gpio_value, 50, "%s/value",  gpio_path);
    // open gpio value attribute
    f = open(gpio_value, O_RDONLY);
    //Ici ça print le file descriptor et non le contenu du fichier!!
    //Faut que ça retourne le contenu et non le file descriptor
    return f;

}

extern int close_switch(const char *pin){
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, pin, strlen(pin));
    close(f);
    return f;
}

extern int read_switch(const char *pin, const char *gpio_path){
    int fd = open_switch(pin, gpio_path);
    char buff[100];

    while(42){ // J'ai perdu!
        ssize_t nr = read (fd, buff, sizeof(buff));
        if (nr == 0) break;  // --> all data have been read
        if (nr == -1) {
            if (errno == EINTR) continue;  // --> continue reading
            char estr[100] = {
                [0] = 0,
            };
            strerror_r(errno, estr, sizeof(estr) - 1);
            fprintf(stderr, "ERROR: %s\n", estr);
            break;  // --> error: stop reading
        }
    }

    printf("switch value: %s\n", buff);
    close(fd);

    return 0;
}