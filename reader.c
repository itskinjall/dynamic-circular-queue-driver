#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define DRIVER_NAME "/dev/vicharak"
#define POP_DATA _IOR('a', 'c', struct data_packet *)

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

    printf("Waiting to receive data...\n");

    if (ioctl(fd, POP_DATA, &packet) < 0) {
        perror("ioctl POP");
        free(packet.buffer);
        close(fd);
        return -1;
    }

    printf("Received: %.*s\n", packet.length, packet.buffer);

    free(packet.buffer);
    close(fd);
    return 0;
}
