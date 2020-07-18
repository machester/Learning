#ifndef __SL3S4011_H
#define __SL3S4011_H

&i2cx@addrsss {
	compatibale = "rfdi,sl3s4011";
	status = "okay";
	
}




static int get_dts_info(const char *compat);
static int dev_open(struct inode *inode, struct file *filp);
static int dev_release(struct inode *inode, struct file *filp);
static ssize_t dev_write(struct file *flip, const char __user *buff,
                         size_t counter, loff_t *fops) ;
static long dev_ioctl(struct file *flip, unsigned int cmd,
                      unsigned long param);


#endif




