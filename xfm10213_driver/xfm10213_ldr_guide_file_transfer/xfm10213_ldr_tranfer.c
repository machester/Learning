#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>

#define read_buff_size      2
int main(int argc, char **argv){
    int fd;
    int index, counter, total;
    unsigned char ldr_buff[read_buff_size + 1] = {'\0'};
    unsigned char temp_buff[4096 * 5];
    unsigned char temp;
    index = counter = total = 0;
    if(2 != argc) {
        printf("----------------------------------------------------\n");
        printf("input format error:\n");
        printf("\t format: ./<app_name> <txt file path>\n");
        printf("----------------------------------------------------\n");
        return -1;
    }
    printf("transfered in: argv[0] = %s, argv[1] = %s\n", argv[0], argv[1]);
    printf("---------------------- start ------------------\n");
    fd = open(argv[1], O_RDONLY);
    if(fd < 0) {
        printf("cannot open file: %s\n", argv[1]);
        return -1;
    }
    printf("---> file: %s opened\n", argv[1]);
    lseek(fd, SEEK_SET, 0);
    counter = 10;
    while(counter){
        counter = 0;
        counter = read(fd, ldr_buff, read_buff_size);
        if (counter < 0){
            printf("---> error: read file: %s\n", argv[1]);
            close(fd);
            exit(1);
        }
        printf("-----------------------------------------------------------------------------------------------\n");
        printf("---> counter = %d, ldr_buff: %s \n", counter, ldr_buff);
        printf("---> ldr_buff[0]: 0x%x, ldr_buff[1]: 0x%x \n", ldr_buff[0], ldr_buff[1]);
        printf("-----------------------------------------------------------------------------------------------\n");
            if(ldr_buff[0] < '9') {
                ldr_buff[0] = ldr_buff[0] - '0';
            } else if(ldr_buff[0] <= 'F' && ldr_buff[0] >= 'A') {
                ldr_buff[0] = (ldr_buff[0] - 'A') + 0x0A;
            }
            if(ldr_buff[1] < '9') {
                ldr_buff[1] = ldr_buff[1] - '0';
            } else if(ldr_buff[1] <= 'F' && ldr_buff[1] >= 'A') {
                ldr_buff[1] = (ldr_buff[1] - 'A') + 0x0A;
            }
            printf("---> current ldr_buff[%d]: 0x%x\t, ldr_buff[%d]: 0x%x \n", 0, ldr_buff[0], 1, ldr_buff[1]);
            temp_buff[total] = (ldr_buff[0] << 4) | ldr_buff[1];
            printf("---> current temp_buff[%d] = 0x%x\n", total, temp_buff[total]);
            memset(ldr_buff, '\0', sizeof(ldr_buff));
            total++;
            index++;
    }
    close(fd);
    printf("--------------------------------------------------------------------------------\n");
    int index2 = 0;
    int len = strlen(temp_buff);
    printf("len = %d\n", len);
    for(int index2 = 0; index2 < total; index2++) {
        printf("temp_buff[%d] = 0x%-8x", index2, temp_buff[index2]);
        if(index2 > 4 && (0 == (index2 % 5)))
            printf("\n");
    }
    printf("-------------------------------------------------------------------------------\n");
	printf("-------- write to file -----------\n");
    int write_fd = open("hex.c", O_CREAT, 0666);
    // unsigned char write_buff[1024 * 20];
    // for(int index = 0; index < counter; index++) {
    //     sprintf(write_buff, "%x, ", temp_buff[index])
    // }
    write(write_fd, temp_buff, total - 1);
    close(write_fd);
    return 0;
}
