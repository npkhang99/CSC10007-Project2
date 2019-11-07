#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

char rng[256];

int main() {
    int ret, fd;
    printf("Starting device test code example...\n");
    // fd = open("/dev/rng", O_RDWR);             // Open the device with read/write access
    // if (fd < 0){
    //   perror("Failed to open the device...");
    //   return errno;
    // }

    while (1) {
        printf("Press ENTER to read from the device...");
        fd = open("/dev/rng", O_RDWR);             // Open the device with read/write access
        if (fd < 0){
            perror("Failed to open the device...");
            return errno;
        }
        char c = getchar();

        if (c != 10) {
            close(fd);
            break;
        }

        printf("Reading from the device...\n");
        ret = read(fd, rng, 256);        // Read the response from the LKM
        if (ret < 0){
            close(fd);
            perror("Failed to read the message from the device.");
            return errno;
        }

        printf("The received random number is: [%s]\n\n", rng);
        close(fd);
    }

    printf("End of the program\n");
    return 0;
}
