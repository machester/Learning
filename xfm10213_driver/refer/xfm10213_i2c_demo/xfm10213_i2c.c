#include <stdio.h>
#include <stdint.h> 
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#define xfm10213_ADDR           (0x3E)
#define DELAY_MS(ms)			usleep((ms) * 1000)				
#define I2C_DEFDEV_NAME			"/dev/i2c-0"
#define I2C_BUF_LEN				(128)
#define MAX_PATH				(260)

static int i2c_write_proc(int fd, unsigned char addr, unsigned char reg[], unsigned char *val, unsigned char len) {
	struct i2c_rdwr_ioctl_data cmd;
	struct i2c_msg msg[2];
	msg[0].addr	= addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = reg;
	
	msg[1].addr	= addr;
	msg[1].flags = 0 /* | I2C_M_NOSTART*/;
	msg[1].len = len;
	msg[1].buf = val;
	
	/* Construct the i2c_rdwr_ioctl_data struct */
	cmd.msgs =  msg;
	cmd.nmsgs = sizeof(msg) / sizeof(struct i2c_msg);

	/* Send the request to the kernel and get the result back */
	if (ioctl(fd, I2C_RDWR, &cmd) < 0) {
		printf("Unable to send data!\n");
		return -1;
	}

	return 0;
}

static int i2c_read_proc(int fd, unsigned char addr, unsigned char reg[], unsigned char *val, unsigned char len) {
	struct i2c_rdwr_ioctl_data cmd;
	struct i2c_msg msg[2];

	msg[0].addr	= addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = reg;
	
	msg[1].addr	= addr;
	msg[1].flags = I2C_M_RD /* | I2C_M_NOSTART*/;
	msg[1].len = len;
	msg[1].buf = val;
	
	/* Construct the i2c_rdwr_ioctl_data struct */
	cmd.msgs = msg;
	cmd.nmsgs = sizeof(msg) / sizeof(struct i2c_msg);

	/* Send the request to the kernel and get the result back */
	if (ioctl(fd, I2C_RDWR, &cmd) < 0) {
		printf("Unable to send data!\n");
		return -1;
	}

	return 0;
}

//
static int i2c_download_proc(int fd,unsigned char addr,unsigned char *val,unsigned char len){

	struct i2c_rdwr_ioctl_data cmd;
	struct i2c_msg msg[1];
	msg[0].addr = addr;
	msg[0].flags = 0;
	msg[0].len = len;
	msg[0].buf = val;
	
	cmd.msgs = msg;
	cmd.nmsgs = sizeof(msg) / sizeof(struct i2c_msg);

	/* Send the request to the kernel and get the result back */
	if (ioctl(fd, I2C_RDWR, &cmd) < 0) {
		printf("Unable to send data!\n");
		return -1;
	}

	return 0;

}

int main (void) {
    int fd; 

 fd = open(I2C_DEFDEV_NAME, O_RDWR);
    if (fd < 0) {
        printf("Error opening file: %s\n", strerror(errno));
        return 1;
    }   
    if (ioctl(fd, I2C_SLAVE, xfm10213_ADDR) < 0) {
        printf("ioctl error: %s\n", strerror(errno));
        return 1;
    }else{
        printf("open success\n"); 
        } 

    //open xfm10213.ldr   
    FILE *src_fp = fopen("./upgrade.pak","rb");
	if(src_fp == NULL)  
	{
		printf("xfm10213.ldr file not found\n");
		return 0;  
	}
	fseek(src_fp, 0L, SEEK_END);
	unsigned int src_fp_len = ftell(src_fp);
	char *buf = (char *)malloc(src_fp_len);
	memset(buf,0,src_fp_len);
	fseek(src_fp,0L,SEEK_SET);

	fread(buf,1,src_fp_len,src_fp);
	unsigned char r2w = 255;
	while(1)
	{
		if(src_fp_len >= r2w)
		{
			i2c_download_proc(fd,xfm10213_ADDR,buf,r2w);
			src_fp_len -= r2w;
			buf += r2w;
		}
		else
		{
			i2c_download_proc(fd,xfm10213_ADDR,buf,src_fp_len);
			break;
		}
	 }
    DELAY_MS(2000);
	
	unsigned int temp=0x0000;
	//
    unsigned int data_mode=0x0200;      //写入的是0002
	unsigned int data_mode2=0x0400;     //写入的是0004
    unsigned int data_chan=0x5612;		//写入的是1256
    unsigned char reg101[2]={0x01,0x01};
    unsigned char reg110[2]={0x01,0x10};
    unsigned char reg111[2]={0x01,0x11};
    //example
        i2c_read_proc(fd, xfm10213_ADDR, reg101,(unsigned char *)&temp,2);
        DELAY_MS(200);
        printf("101 data is %08X\n",temp);
        DELAY_MS(200);
        i2c_write_proc(fd,xfm10213_ADDR,reg110,(char *)&data_mode2,2);
        DELAY_MS(200);
        i2c_read_proc(fd, xfm10213_ADDR, reg101,(unsigned char *)&temp,2);
        DELAY_MS(200);
        printf("101 data is %08X\n",temp);
        i2c_read_proc(fd, xfm10213_ADDR, reg110,(unsigned char *)&temp,2);
        printf("110 data is%08X\n",temp);
        DELAY_MS(200);
        i2c_write_proc(fd,xfm10213_ADDR,reg111,(unsigned char *)&data_chan,2);
        DELAY_MS(200);
        i2c_read_proc(fd, xfm10213_ADDR, reg110,(unsigned char *)&temp,2);
        printf("110 data is%08X\n",temp);
        DELAY_MS(200);
        i2c_read_proc(fd, xfm10213_ADDR, reg111,(unsigned char *)&temp,2);
        printf("111 data is%08X\n",temp);
        DELAY_MS(200);
        i2c_read_proc(fd, xfm10213_ADDR, reg101,(unsigned char *)&temp,2);
        printf("101 data is%08X\n",temp);
        DELAY_MS(200);
		
		
		i2c_write_proc(fd,xfm10213_ADDR,reg110,(unsigned char *)&data_mode,2);
        DELAY_MS(200);
        i2c_read_proc(fd, xfm10213_ADDR, reg101,(unsigned char *)&temp,2);
        DELAY_MS(200);
        printf("101 data is %08X\n",temp);
        i2c_read_proc(fd, xfm10213_ADDR, reg110,(char *)&temp,2);
        printf("110 data is%08X\n",temp);
        DELAY_MS(200);
		i2c_read_proc(fd, xfm10213_ADDR, reg111,(char *)&temp,2);
        printf("111 data is%08X\n",temp);
        DELAY_MS(200);
    return 0;

}
