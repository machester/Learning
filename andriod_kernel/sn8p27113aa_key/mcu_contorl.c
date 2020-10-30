#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/wakelock.h>
#include <asm/io.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

//add by heliang for systemclose_gpio 20190329 >>>>
#if 1
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#endif
//add end <<<<

//add by heliang for systemclose_gpio 20190329 >>>>
#if 1
#define MAX_BUFFER_SIZE   512
static struct miscdevice  miscdevice;		
static struct mutex       write_mutex;	
#endif
//add end <<<<



// #define INPUT_REPORT_KEY  KEY_POWER
#define INPUT_REPORT_KEY  0xFE

static unsigned int irq_gpio;
static unsigned int det2_gpio;
static unsigned int irq;

static struct delayed_work mcu_contorl_work;
static struct workqueue_struct *mcu_contorl_workqueue = NULL;

static struct input_dev *input_dev=NULL;

static struct timer_list timer;



//add by heliang for systemclose_en 20190329 >>>>
#if 1
static int systemclose_en_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int systemclose_en_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t systemclose_en_write(struct file *filp, const char __user *buf,
        size_t count, loff_t *offset)
{
	char rev_buf[2] = {0};
	unsigned long rev_cmd = 0;


    if (count > MAX_BUFFER_SIZE)
        count = MAX_BUFFER_SIZE;

    mutex_lock(&write_mutex);
	
    if (copy_from_user(rev_buf, buf, count)) {
        printk("%s : failed to copy from user space!!!\n", __func__);
        return -EFAULT;
    }

//	printk("*****************set systemclose  rev_buf=%s\n",rev_buf);
	rev_cmd = simple_strtoul(rev_buf,NULL,0);
//	printk("*****************set systemclose rev_cmd=%d\n",(int)rev_cmd);
	
	if(rev_cmd)
    {
	  gpio_direction_output(det2_gpio, 0);	
	  mdelay(100);
	  gpio_direction_output(det2_gpio, 1);
	  mdelay(50);
	  gpio_direction_output(det2_gpio, 0);
	  mdelay(100);
	  gpio_direction_output(det2_gpio, 1);
	} 

    mutex_unlock(&write_mutex);	
	
    return count;
}

static struct file_operations systemclose_en_fops = {
	.owner 		= THIS_MODULE,
	.llseek     = no_llseek,
	.open 		= systemclose_en_open,
	.release 	= systemclose_en_release,	
	.write  	= systemclose_en_write,
};
#endif 
//add end <<<<


static void mcu_contorl_worker_func(struct work_struct *work)
{
	printk("-------%s %d------\n", __func__,__LINE__);

	printk("[%s]: input report KEY_POWER\n",__func__);
	input_report_key(input_dev,INPUT_REPORT_KEY,1);
	input_sync(input_dev);

	input_report_key(input_dev,INPUT_REPORT_KEY,0);
	input_sync(input_dev);	
	
	//gpio_direction_output(det2_gpio, 0);//to mcu for close system	
	  gpio_direction_output(det2_gpio, 0);	
	  mdelay(100);
	  gpio_direction_output(det2_gpio, 1);
	  mdelay(50);
	  gpio_direction_output(det2_gpio, 0);
	  mdelay(100);
	  gpio_direction_output(det2_gpio, 1);
}

static irqreturn_t mcu_contorl_det_irq_handler(int irq, void *dev_id)
{
	printk("-------%s %d------\n", __func__,__LINE__);

	disable_irq_nosync(irq);
	
	queue_delayed_work(mcu_contorl_workqueue, &mcu_contorl_work, 300);	
	
    return IRQ_HANDLED;
}

static void mcu_contorl_timer_function(unsigned long para)
{
	printk("---------%s %d start MCU control-----\n", __func__,__LINE__);

	enable_irq(irq);	
}

static int mcu_contorl_power_idev(void)
{
	int rc = 0;
	
	input_dev = input_allocate_device();
	if(!input_dev){
		return -1;
	}

	input_dev->name = "mcu_contorl";
	input_dev->id.bustype = BUS_I2C;
	input_set_capability(input_dev,EV_KEY,INPUT_REPORT_KEY);

	rc = input_register_device(input_dev);
	if(rc){
		input_free_device(input_dev);
		return -1;
	}
	
	return 0;
}

static int mcu_contorl_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *node = NULL; 
	
	printk("%s : start!!!\n", __func__);		
	
	node = of_find_compatible_node(NULL, NULL, "mediatek,mcu_contorl");
	if(!node){
		printk("%s find mcu_contorl node failed!!\n", __func__);
		return -EINVAL;		
	}
	printk("%s find mcu_contorl node successed!!\n", __func__);
	
	irq_gpio = of_get_named_gpio(node, "mt8735_mcu_det", 0);
    if ((!gpio_is_valid(irq_gpio)))
            return -EINVAL;
		
	ret = gpio_request_one(irq_gpio, GPIOF_IN,"mt8735_mcu_det");	
    gpio_direction_input(irq_gpio);	

	det2_gpio = of_get_named_gpio(node, "mt8735_mcu_det2", 0);
    if ((!gpio_is_valid(det2_gpio)))
            return -EINVAL;	

	
	ret = gpio_request_one(det2_gpio, GPIOF_OUT_INIT_LOW, "mt8735_mcu_det2");	
	gpio_direction_output(det2_gpio, 1);
	
	ret = mcu_contorl_power_idev();
	if (ret) {
        printk("-------%s error------\n", __func__);
        return ret;
    }
	
	INIT_DELAYED_WORK(&mcu_contorl_work, mcu_contorl_worker_func);
	mcu_contorl_workqueue = create_singlethread_workqueue("mcu_contorl_workqueue");
	// queue_delayed_work(mcu_contorl_workqueue, &mcu_contorl_work, 300);	
	
	//delay 50s for into system open mcu control enable irq
	init_timer(&timer);	
	timer.data = (unsigned long)50;
	timer.expires = jiffies + (50 * HZ);
	timer.function = mcu_contorl_timer_function;
	add_timer(&timer);
	
	irq = gpio_to_irq(irq_gpio);
	// irq = irq_of_parse_and_map(node, 0);
   // printk("%s : requesting IRQ %d det2_gpio %d\n", __func__, irq,det2_gpio);	
    ret = request_irq(irq, mcu_contorl_det_irq_handler,IRQF_TRIGGER_HIGH, "mcu_contorl", NULL);        
    if (ret) {
        printk("request_irq failed!!!\n");
        return ret;
    } 
	disable_irq(irq);
	
	//add by heliang for systemclose_en 20190329 >>>>
#if 1
	mutex_init(&write_mutex);	
    miscdevice.minor = MISC_DYNAMIC_MINOR;
    miscdevice.name = "systemclose_en";
    miscdevice.fops = &systemclose_en_fops;		
	ret = misc_register(&miscdevice);
	if (ret) 
	{
		printk("%s: misc_register systemclose_en failed\n", __func__);
	}
#endif	
//add end <<<<

	printk("%s : end!!!\n", __func__);	
	
	return 0;
}

static int mcu_contorl_remove(struct platform_device *pdev)
{
	printk("-------%s %d------\n", __func__,__LINE__);
	
	return 0;
}

static const struct of_device_id of_match[] = {
	{.compatible = "mediatek,mcu_contorl"},
};

static struct platform_driver mcu_contorl_driver = {
	.probe      = mcu_contorl_probe,
	.remove     = mcu_contorl_remove,
	.driver = {
		.name = "mcu_contorl",
		.of_match_table = of_match,
	},
};

static int __init mcu_contorl_init(void)
{	
	printk("-------%s %d------\n", __func__,__LINE__);

    return platform_driver_register(&mcu_contorl_driver);
}
 
static void __exit mcu_contorl_exit(void)
{
	printk("-------%s %d------\n", __func__,__LINE__);
	
	platform_driver_unregister(&mcu_contorl_driver);
}
 
late_initcall(mcu_contorl_init);
module_exit(mcu_contorl_exit);

MODULE_AUTHOR("Elink HeLiang 20190108 for SN8P27113AA MCU");
MODULE_DESCRIPTION("Mcu Contorl System Power Driver");
MODULE_LICENSE("GPL");
