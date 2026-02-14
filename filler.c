#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#define DRIVER_NAME "/dev/vicharak"
#define PUSH_DATA _IOW('a', 'b', struct data_packet *)

struct data_packet {
    int length;
    char *buffer;
};

int main()
{
    int fd = open(DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    struct data_packet packet;

    packet.length = 5;
    packet.buffer = malloc(packet.length);

    memcpy(packet.buffer, "hello", 5);

    if (ioctl(fd, PUSH_DATA, &packet) < 0) {
        perror("ioctl PUSH");
    }

    free(packet.buffer);
    close(fd);
    return 0;
}
