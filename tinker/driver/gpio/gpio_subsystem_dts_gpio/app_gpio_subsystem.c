/**
 * @Description : file for linux driver test.
 * @Test function: open, read, write, close, ioctl and o on.
 * @Author :
 * @Date :
 * @Version :
 * ----------------------------------------------------------------------
 * @ioctl param description:
 * @CMD:
 * @Magic number :
 *
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int ret;
    int fd;
    int on = 0;
    int counter = 0;

    printf("--> open /dev/laser, permisson read and write\n");
    fd = open("/dev/laser", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }
    printf("--> open success. ready to send data:\n");

    while (counter < 50)
    {
        on = 1;
        printf("--> open success. ready to send data, data = %d\n", counter);
        //ret = write(fd, &on, 4);
        ret = write(fd, &counter, 4);
        if (ret < 0)
        {
            perror("write");
            exit(1);
        }
        sleep(2);
        counter++;
        if (3 == counter)
            counter = 0;
    }

    printf("--> close fd\n");
    close(fd);
    return 0;
}
