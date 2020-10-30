/**
 * @file hex_analysis.c
 * @author V
 * @brief 
 *      read hex file and set items to arry store in file hex.txt
 * @example
 *      01,00,00,68,00,50,FC,C1,01,00,00,
        14,E5,C1,1C,71,FB,80,11,59,FC,80,
        01,00,00,5D,00,50,FC,C1,01,00,00,
        14,E5,C1,1C,71,FB,80,51,59,FC,80,
        01,00,00,52,00,50,FC,C1,01,00,00,
        14,E5,C1,1C,71,FB,80,91,59,FC,80,
        01,00,00,47,00,50,FC,C1,01,00,00,
        14,E5,C1,1C,71,FB,80,D1,59,FC,80,
        01,00,00,3C,00,50,FC,C1,01,00,00,
        14,E5,C1,1C,72,FB,80,11,59,FC,80,
        01,00,00,14,00,50,FC,C1,01,00,00,
        14,E5,C1,1C,72,FB,80,51,A0,D4,01,
        59,59,FC,80,01,FF,FF,80,FF,50,FC,
        C1,01,00,00,14,E5,C1,1C,72,FB,80,
 * @version 0.1
 * @date 2019-06-03
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>

#define READ_BUFF_SIZE      2
#define WRITE_BUFF_SIZE     (20 * 1024)
#define WRITE_FILE              "hex.txt"

int main(int argc, char **argv) {
    int fd, write_fd;
    int read_counter, write_counter;
    unsigned char read_buff[READ_BUFF_SIZE + 1] = {'\0'};
    unsigned char write_buff[WRITE_BUFF_SIZE + 1] = {'\0'};
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
        exit(1);
    }
    write_fd = open(WRITE_FILE, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if(fd < 0) {
        printf("cannot open file: %s\n", WRITE_FILE);
        exit(1);
    }
    printf("---> file: %s, and file: %s opened\n", argv[1], WRITE_FILE);
    lseek(fd, SEEK_SET, 0);
    read_counter = 10;
    while(read_counter) {
        read_counter = 0;
        read_counter = read(fd, read_buff, READ_BUFF_SIZE);
        if (read_counter < 0){
            printf("---> error: read file: %s\n", argv[1]);
            close(fd);
            exit(1);
        }
        printf("---> read_counter = %d, read_buff: %s \n", read_counter, read_buff);
        write_buff[write_counter] = read_buff[0];
        write_buff[write_counter + 1] = read_buff[1];
        write_buff[write_counter + 2] = ',';
        write_counter += 3;
        if(write_counter >= 3 * 10) {
            write(write_fd, write_buff, write_counter);
            memset(write_buff, '\0', write_counter);
            write_counter = 0;
            write(write_fd, "\n", 1);
        }
        memset(read_buff, '\0', sizeof(read_buff)/sizeof(unsigned char));
    }
    // write data staged in write_buff
    write(write_fd, write_buff, write_counter);
    write(write_fd, "\n", 1);
    
    close(fd);
    close(write_fd);
    return 0;
}
