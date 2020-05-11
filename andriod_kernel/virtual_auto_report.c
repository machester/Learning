#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
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


#define VIRTUAL_KEY				BTN_0

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
	
    printk("---------- repo_index = %d ---------\n",repo_index);
    if(repo_index < 50) {
    	mod_timer(&timer1,jiffies + HZ);
	    input_report_key(vtp->vtp_input_dev, VIRTUAL_KEY, 1);
	    input_sync(vtp->vtp_input_dev);
	    input_report_key(vtp->vtp_input_dev, VIRTUAL_KEY, 0);
    } else {
		printk("---- repo_index = %d  del timer\n",repo_index);
		del_timer(&timer1);
	}
}


static int __init virtual_auto_report_init(void)
{
	int ret = 0 ;
	printk("moved in func: %s\n", __func__);
	vtp = kmalloc(sizeof(*vtp),GFP_KERNEL);
	memset(vtp,0,sizeof(*vtp));
	ret= alloc_chrdev_region(&vtp->devno, vtp->cdev_minor, 1, "virutal_key_auto_report_key"); 
	if(ret < 0) 
	{
		printk(KERN_ALERT "virutal_key:Failed to alloc char virutal_auto_report_key region.\n");
		goto exit0;    
	}
	vtp->cdev_major= MAJOR(vtp->devno);
	vtp->cdev_minor = MINOR(vtp->devno);
	vtp->cdev = cdev_alloc();
    vtp->cdev->owner = THIS_MODULE;
	
	if ((ret = cdev_add(vtp->cdev, vtp->devno, 1)))
    {
        printk(KERN_NOTICE "virutal_key:Error %d adding virutal_key.\n", ret);
        goto exit1;
    } else printk(KERN_ALERT "virutal_key:virutal register success.\n");
	
	vtp->cdev_class= class_create(THIS_MODULE, "virutal_key");
    if(IS_ERR(vtp->cdev_class)) {
        ret = PTR_ERR(vtp->cdev_class);
        printk(KERN_ALERT "Failed to create virutal_key class.\n");
        goto exit1;
    }        
    vtp->temp= device_create(vtp->cdev_class, NULL, vtp->devno, "%s", "virutal_key");
    if(IS_ERR(vtp->temp)) {
        ret = PTR_ERR(vtp->temp);
        printk(KERN_ALERT"Failed to create virutal_key device.");
        goto exit2;
    }   
	
	vtp->vtp_input_dev = input_allocate_device();

	if (vtp->vtp_input_dev == NULL) {
		printk(KERN_ERR "virutal_key Failed to allocate input device.\n");
		return -ENOMEM;
	}
	input_set_drvdata(vtp->vtp_input_dev, vtp);
	
	vtp->vtp_input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY)| BIT_MASK(EV_SYN);
	input_set_capability(vtp->vtp_input_dev, EV_KEY, VIRTUAL_KEY);
	
	vtp->vtp_input_dev->name = "virutal_key";

	vtp->vtp_input_dev->id.bustype = BUS_VIRTUAL;
	vtp->vtp_input_dev->id.vendor = 0x0001;
	vtp->vtp_input_dev->id.product = 0x0001;
	vtp->vtp_input_dev->id.version = 1;

	ret = input_register_device(vtp->vtp_input_dev);
	if (ret) {
		printk(KERN_ERR"virutal_key:Register %s input device failed.\n",vtp->vtp_input_dev->name);
		goto exit3;
	}
	else
		printk(KERN_ERR "virutal_key:Register %s input device register success.\n",vtp->vtp_input_dev->name);


	printk("virutal_key: create report timer");
	init_timer(&timer1);
	timer1.function = timer1_function;
	add_timer(&timer1);
	mod_timer(&timer1,jiffies + HZ);

	printk(KERN_ERR "virutal_key %s...end\n", __func__);
	return ret;
	
exit3:
	input_free_device(vtp->vtp_input_dev);
exit2:
	class_destroy(vtp->cdev_class);
exit1:
	cdev_del(vtp->cdev);
exit0:
	kfree(vtp);
	return ret ;

}

static void __exit virtual_auto_report_exit(void)
{
	printk("moved in func: %s\n", __func__);
	cdev_del(vtp->cdev);
	class_destroy(vtp->cdev_class);
	kfree(vtp);
    del_timer(&timer1);

}


module_init(virtual_auto_report_init);
module_exit(virtual_auto_report_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("V:machester@aliyun.com");

