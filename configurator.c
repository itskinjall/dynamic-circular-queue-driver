#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DRIVER_NAME "/dev/vicharak"
#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int *)

int main()
{
    int fd = open(DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    int size = 100;

    if (ioctl(fd, SET_SIZE_OF_QUEUE, &size) < 0) {
        perror("ioctl");
        close(fd);
        return -1;
    }

    printf("Queue size set to %d\n", size);

    close(fd);
    return 0;
}
