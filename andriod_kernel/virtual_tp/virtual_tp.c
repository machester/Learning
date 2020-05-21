#if 1
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/of_gpio.h>
#include <linux/input/mt.h>
#include <linux/fs.h>  
#include <asm/uaccess.h>  
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/timer.h>  /*timer*/  
#include <asm/uaccess.h>  /*jiffies*/  

#define DEVICE_NAME      "virtual_input"
#define SCREEN_MAX_X     1920
#define SCREEN_MAX_Y     1080
#define PRESS_MAX        0xFF

#define SWAP_XY             1
#define INVERT_X            1
#define INVERT_Y            0

#define GTP_SWAP(x, y)                do{                   \
                                         typeof(x) z = x;   \
                                         x = y;             \
                                         y = z;             \
                                       }while (0)

#define GTP_INVERT_X(x)            do{                          \
                                         x = SCREEN_MAX_X - x;  \
                                       }while (0)

#define GTP_INVERT_Y(y)            do{                          \
                                         y = SCREEN_MAX_Y - y;  \
                                       }while (0)


#define UPDATE_COORDINATE  0x9981
#define NUM_COPY_INT 2

struct input_dev *inputDev;


static ssize_t virtual_write(struct file *file, const char __user *buff, size_t count, loff_t *offp)
{
    int x, y;
	unsigned char i = 0, finger = 20;
    printk(KERN_ERR "in func: %s\n", __func__);
	for(i=0; i<finger; i++){
        x = 100;
        y = 50 + i * 50;
    #ifdef SWAP_XY
        GTP_SWAP(x, y);
    #endif
    #if INVERT_X
        GTP_INVERT_X(x);
    #endif
    #if INVERT_Y
        GTP_INVERT_Y(y);
    #endif 
        printk(KERN_ERR "x = %d, y = %d\n", x, y);
		input_report_key(inputDev, BTN_TOUCH, 1); // press
		input_report_abs(inputDev, ABS_MT_POSITION_X, x);
		input_report_abs(inputDev, ABS_MT_POSITION_Y, y);
		input_mt_sync(inputDev);
        // ssleep(5);
        mdelay(10);
	}
	input_sync(inputDev);
    mdelay(500);
	for(i=0; i<finger; i++){
        y = 100;
        x = 50 + i * 50;
    #ifdef SWAP_XY
        GTP_SWAP(x, y);
    #endif
    #if INVERT_X
        GTP_INVERT_X(x);
    #endif
    #if INVERT_Y
        GTP_INVERT_Y(y);
    #endif 
        printk(KERN_ERR "x = %d, y = %d\n", x, y);
		input_report_key(inputDev, BTN_TOUCH, 1); // press
		input_report_abs(inputDev, ABS_MT_POSITION_X, x);
		input_report_abs(inputDev, ABS_MT_POSITION_Y, y);
		input_mt_sync(inputDev);
        mdelay(10);
	}
	input_sync(inputDev);
	mdelay(500);
	input_report_key(inputDev, BTN_TOUCH, 0); // release
	input_mt_sync(inputDev);
	input_sync(inputDev);
	return count;
}

static int virtual_open(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "vtp_cdev:vtp_cdev_open\n");
	return 0;
}

static long virtual_ioctl(struct file *filp, unsigned int cmd,
			    unsigned long arg)
{
	int ret = 0;
	int data[] = {0,0} ;
	int __user *argp = (int __user *)arg;
	printk(KERN_ERR "in func: %s\n", __func__);
    if (!argp)
		return -EINVAL;
	switch (cmd) {
        case UPDATE_COORDINATE:
            printk(KERN_ERR "vtp_ioctl UPDATE_COORDINATE\n");
            if (copy_from_user(data, argp, NUM_COPY_INT*sizeof(int)))
                return -EFAULT;
            #ifdef SWAP_XY
                GTP_SWAP(data[0], data[1]);
            #endif
			#if INVERT_X
				GTP_INVERT_X(data[0]);
			#endif
			#if INVERT_Y
				GTP_INVERT_Y(data[1]);
			#endif 
                printk(KERN_ERR "copy data[0]=%d,data[1]=%d\n",data[0],data[1]);
            if(inputDev){
                input_report_key(inputDev, BTN_TOUCH, 1); // 按下
                input_report_abs(inputDev, ABS_MT_POSITION_X, data[0]);
                input_report_abs(inputDev, ABS_MT_POSITION_Y, data[1]);
                input_mt_sync(inputDev);
                mdelay(10);
                input_report_key(inputDev, BTN_TOUCH, 0); // 弹起
                input_mt_sync(inputDev);
                input_sync(inputDev);
            }
            break;
        default:
		    break;
	}
	return ret;
}

static struct file_operations fileOps = {
	.owner = THIS_MODULE,
	.write = virtual_write,
    .unlocked_ioctl = virtual_ioctl,
    .open = virtual_open,
};

static struct miscdevice miscDev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &fileOps,
};

int __init virtual_input_init(void)
{
	int ret;
	
	inputDev = input_allocate_device();
	if (inputDev == NULL){
		printk("input_allocate_device fail\n");
		return -ENOMEM;
	}
	
	inputDev->name = DEVICE_NAME;
	set_bit(ABS_MT_TOUCH_MAJOR, inputDev->absbit);
	set_bit(ABS_MT_POSITION_X, inputDev->absbit);
	set_bit(ABS_MT_POSITION_Y, inputDev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, inputDev->absbit);	
	set_bit(BTN_TOUCH, inputDev->keybit);
	set_bit(EV_ABS, inputDev->evbit);
	set_bit(EV_KEY, inputDev->evbit);
	set_bit(INPUT_PROP_DIRECT, inputDev->propbit);
	
	input_set_abs_params(inputDev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(inputDev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(inputDev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(inputDev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(inputDev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(inputDev, ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);
	input_set_abs_params(inputDev, ABS_MT_TRACKING_ID, 0, 4, 0, 0);

	ret = input_register_device(inputDev);
	if (ret){
		printk("input_register_device fail\n");
		return ret;
	}

	// debug misc device registed
	ret = misc_register(&miscDev);
	if (ret){
		printk("misc_register fail\n");
		return ret;
	}
	
	return 0;
}

void __exit virtual_input_exit(void)
{	
	misc_deregister(&miscDev);
	input_unregister_device(inputDev);
	input_free_device(inputDev);
}

module_init(virtual_input_init);
module_exit(virtual_input_exit);
MODULE_AUTHOR("V <machester@aliyun.com>");
MODULE_DESCRIPTION("virtual multi touch panel type-A");
MODULE_LICENSE("GPL");



#else		// report virtual mouse
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/of_gpio.h>
#include <linux/input/mt.h>
#include <linux/fs.h>  
#include <asm/uaccess.h>  
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/timer.h>  /*timer*/  
#include <asm/uaccess.h>  /*jiffies*/  

#define DEVICE_NAME      "vtp"
#define SCREEN_MAX_X     1920
#define SCREEN_MAX_Y     1080
#define PRESS_MAX        0xFF

#define SWAP_XY             1

#define GTP_SWAP(x, y)                do{                   \
                                         typeof(x) z = x;   \
                                         x = y;             \
                                         y = z;             \
                                       }while (0)


struct virtual_tp {
	dev_t devno ;
	struct device* temp;
	struct class* cdev_class ;
	struct cdev* cdev;
	int cdev_major;
	int cdev_minor;
	
	struct input_dev  *vtp_input_dev ;
};
static struct virtual_tp* vtp ;

static struct timer_list timer1;

void timer1_function(unsigned long arg)
{
	static int repo_index;
	repo_index++;
    printk("---------- repo_index = %d ---------\n",repo_index);
    if(repo_index < 35) {
    	mod_timer(&timer1,jiffies + HZ);

        input_report_key(vtp->vtp_input_dev, BTN_TOUCH, 1); // 按下
        input_report_abs(vtp->vtp_input_dev, ABS_MT_POSITION_X, 1);
        input_report_abs(vtp->vtp_input_dev, ABS_MT_POSITION_Y, 1);
        input_sync(vtp->vtp_input_dev);
        ssleep(3);
        input_report_key(vtp->vtp_input_dev, BTN_TOUCH, 0); // 弹起
        input_sync(vtp->vtp_input_dev);

    } else {
		printk("---- repo_index = %d  del timer\n",repo_index);
		del_timer(&timer1);
	}
}

static ssize_t virtual_write(struct file *file, const char __user *buff, size_t count, loff_t *offp)
{
    int x, y;
	unsigned char i = 0, finger = 100;

	for(i=0; i<finger; i++){
        x = 100;
        y = 50 + i * 10;
    #ifdef SWAP_XY
        GTP_SWAP(x, y);
    #endif
        printk(KERN_ERR "x = %d, y = %d\n", x, y);
		input_report_key(vtp->vtp_input_dev, BTN_TOUCH, 1); // 按下
		input_report_abs(vtp->vtp_input_dev, ABS_MT_POSITION_X, x);
		input_report_abs(vtp->vtp_input_dev, ABS_MT_POSITION_Y, y);
		input_mt_sync(vtp->vtp_input_dev);
        ssleep(10);
	}
	input_sync(vtp->vtp_input_dev);
    mdelay(100);
	for(i=0; i<finger; i++){
        y = 100;
        x = 50 + i * 10;
    #ifdef SWAP_XY
        GTP_SWAP(x, y);
    #endif
        printk(KERN_ERR "x = %d, y = %d\n", x, y);
		input_report_key(vtp->vtp_input_dev, BTN_TOUCH, 1); // 按下
		input_report_abs(vtp->vtp_input_dev, ABS_MT_POSITION_X, x);
		input_report_abs(vtp->vtp_input_dev, ABS_MT_POSITION_Y, y);
		input_mt_sync(vtp->vtp_input_dev);
        ssleep(10);
	}
	input_sync(vtp->vtp_input_dev);
	mdelay(100);
	input_report_key(vtp->vtp_input_dev, BTN_TOUCH, 0); // 弹起
	input_mt_sync(vtp->vtp_input_dev);
	input_sync(vtp->vtp_input_dev);
	return count;
}

#define UPDATE_COORDINATE  0x9981
#define NUM_COPY_INT 2

static long vtp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int data[] = {0,0} ;
	int __user *argp = (int __user *)arg;
	if (!argp)
		return -EINVAL;
	switch (cmd) {
        case UPDATE_COORDINATE:
            printk(KERN_ERR "vtp_ioctl UPDATE_COORDINATE\n");
            if (copy_from_user(data, argp, NUM_COPY_INT*sizeof(int)))
                return -EFAULT;
            #ifdef SWAP_XY
                GTP_SWAP(data[0], data[1]);
            #endif
                printk(KERN_ERR "copy data[0]=%d,data[1]=%d\n",data[0],data[1]);
            if(vtp->vtp_input_dev){
                input_report_key(vtp->vtp_input_dev, BTN_TOUCH, 1); // 按下
                input_report_abs(vtp->vtp_input_dev, ABS_MT_POSITION_X, data[0]);
                input_report_abs(vtp->vtp_input_dev, ABS_MT_POSITION_Y, data[1]);
                input_mt_sync(vtp->vtp_input_dev);
                mdelay(10);
                input_report_key(vtp->vtp_input_dev, BTN_TOUCH, 0); // 弹起
                input_mt_sync(vtp->vtp_input_dev);
                input_sync(vtp->vtp_input_dev);
            }
            break;
        default:
		    break;
	}
	return ret;
}

static int vtp_cdev_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
	printk(KERN_ALERT "vtp_cdev:vtp_cdev_open\n");
    return ret;
}

struct file_operations vtp_cdev_fops = {
	.owner = THIS_MODULE,
    .open = vtp_cdev_open,
	.unlocked_ioctl = vtp_ioctl,
    .write = virtual_write,

};

int __init virtTp_init(void)
{
	int ret;
	printk(KERN_ERR "quan virtTp_init...start\n");

    vtp = kmalloc(sizeof(*vtp),GFP_KERNEL);
	memset(vtp,0,sizeof(*vtp));
	ret= alloc_chrdev_region(&vtp->devno, vtp->cdev_minor, 1, "vtp"); 
	if(ret < 0) 
	{
		printk(KERN_ALERT "vtp_cdev:Failed to alloc char dev region.\n");
		goto exit0;    
	}

	vtp->cdev_major= MAJOR(vtp->devno);
	vtp->cdev_minor = MINOR(vtp->devno);
	vtp->cdev = cdev_alloc();
    vtp->cdev->owner = THIS_MODULE;
    vtp->cdev->ops = &vtp_cdev_fops;

	if ((ret = cdev_add(vtp->cdev, vtp->devno, 1)))
    {
        printk(KERN_NOTICE "vtp_cdev:Error %d adding ctp_cdev.\n", ret);
        goto exit1;
    } else printk(KERN_ALERT "vtp_cdev:vtp_cdev register success.\n");
	
	vtp->cdev_class= class_create(THIS_MODULE, "vtp");
    if(IS_ERR(vtp->cdev_class)) {
        ret = PTR_ERR(vtp->cdev_class);
        printk(KERN_ALERT "Failed to create vtp_cdev class.\n");
        goto exit1;
    }        
    vtp->temp= device_create(vtp->cdev_class, NULL, vtp->devno, "%s", "vtp");
    if(IS_ERR(vtp->temp)) {
        ret = PTR_ERR(vtp->temp);
        printk(KERN_ALERT"Failed to create vtp_cdev device.");
        goto exit2;
    }   
	vtp->vtp_input_dev = input_allocate_device();

	if (vtp->vtp_input_dev == NULL) {
		printk(KERN_ERR "eliot Failed to allocate input device.\n");
		return -ENOMEM;
	}
    input_set_drvdata(vtp->vtp_input_dev, vtp);
	

	set_bit(ABS_MT_TOUCH_MAJOR, vtp->vtp_input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, vtp->vtp_input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, vtp->vtp_input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, vtp->vtp_input_dev->absbit);	
	set_bit(BTN_TOUCH, vtp->vtp_input_dev->keybit);
	set_bit(EV_ABS, vtp->vtp_input_dev->evbit);
	set_bit(EV_KEY, vtp->vtp_input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, vtp->vtp_input_dev->propbit);
	
	input_set_abs_params(vtp->vtp_input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(vtp->vtp_input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(vtp->vtp_input_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(vtp->vtp_input_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(vtp->vtp_input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(vtp->vtp_input_dev, ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);
	input_set_abs_params(vtp->vtp_input_dev, ABS_MT_TRACKING_ID, 0, 4, 0, 0);

    vtp->vtp_input_dev->name = DEVICE_NAME;
    vtp->vtp_input_dev->id.bustype = BUS_VIRTUAL;
    vtp->vtp_input_dev->id.vendor = 0xDEAD;
    vtp->vtp_input_dev->id.product = 0xBEEF;
    vtp->vtp_input_dev->id.version = 10;

	/*注册输入设备，在/dev/input/eventX*/
	ret = input_register_device(vtp->vtp_input_dev);
	if (ret){
		printk("input_register_device fail\n");
		goto exit3;
	}

    printk("virutal_key: create report timer");
    init_timer(&timer1);
    timer1.function = timer1_function;
    add_timer(&timer1);
    mod_timer(&timer1,jiffies + HZ);

	return ret;

exit3:
	input_free_device(vtp->vtp_input_dev);
exit2:
	class_destroy(vtp->cdev_class);
exit1:
	cdev_del(vtp->cdev);
exit0:
	kfree(vtp);
	return ret;
}

void __exit virtTp_exit(void)
{	
    cdev_del(vtp->cdev);
    class_destroy(vtp->cdev_class);
    kfree(vtp);
}

module_init(virtTp_init);
module_exit(virtTp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V@machester");
#endif
