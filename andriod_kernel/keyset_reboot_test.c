/**********************************************************************
* dts 

reboot_test: reboot-test {
	compatible = "rockchip, reboot-test";
	status = "okay";
	reboot-key = <&gpio5, RK_PC3, GPIO_ACTIVE_LOW>;
	detect_interval = <100>;		//ms
}

***********************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <uapi/linux/input.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/device.h>


#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/leds.h>


static int debug_status = 1;

// #define KEY_REBOOT_TEST		254	/* Elink None Screen Test Key */
#define KEY_VAL							KEY_REBOOT_TEST 
#define TAG								" <REBOOT-KEY> "
#define SET_LEVEL						KERN_INFO
#define usr_msg(fmt, args...)			printk(SET_LEVEL TAG fmt"\n", ##args)
#define debug_msg(fmt, args...)			do {                                                \
                                            if(debug_status)                                \
                                                printk(SET_LEVEL TAG fmt"\n", ##args);      \
                                        } while(0)

#define KEY_DEV_NAME						"r101u-test-key"			

#define USING_SYS_FS					0
#if USING_SYS_FS
static int reboot_status_flag;
#endif

typedef enum {
	low = 0,
	high = 1
}gpio_status;

typedef enum {
	in = 0,
	out = !in
}gpio_dir;

typedef enum {
	standby = 0,
	running = 1,
	pass	= 2,
	failed  = 3
}test_status;



static irqreturn_t reboot_trigger_handler(int irq, void *arg);

static const struct of_device_id reboot_key_match_table[] = {
	{ .compatible = "rockchip, reboot-test", },
	{/* KEEP THIS */},
};
MODULE_DEVICE_TABLE(of, reboot_key_match_table);

/*--------------------------------------------------------------------------------------------------------------*/
struct reboot_key_info {
	int 						reboot_trigger_io;
	int 						detect_interval;
	unsigned int 				irq;
	struct input_dev 			*inputdev;
	struct mutex 				lock;
	struct workqueue_struct 	* wq;
	struct work_struct			work;
	struct mutex				tim_lock;
	
};

/**----------------------------------------------------------------------------------*/

static void data_handler(struct work_struct * work)
{
	int val;
	struct reboot_key_info * info = container_of(work, struct reboot_key_info, work);
	if(IS_ERR(info)) {
		usr_msg("error: get struct usr_keys_button in function: %s", __func__);
		return ;
	}
	usr_msg("moved in function: %s", __func__);

    mutex_lock(&info->lock);
    mdelay(info->detect_interval);
    val = gpio_get_value(info->reboot_trigger_io);
	if(!val)
    	debug_msg("pressed, report KEY_CODE = %d, KEY_VALUE = %d", KEY_VAL, 1);
	else
		debug_msg("release, report KEY_CODE = %d, KEY_VALUE = %d", KEY_VAL, 0);
	
	input_report_key(info->inputdev, KEY_VAL, !val);
    input_sync(info->inputdev);
    
    mutex_unlock(&info->lock);
}

static irqreturn_t reboot_trigger_handler(int irq, void *arg)
{
	struct reboot_key_info * info = (struct reboot_key_info *)arg;
	if(!info) {
		usr_msg("error: reboot_key_info in function: %s", __func__);
		return IRQ_HANDLED;
	}
	debug_msg("triggered");
	schedule_work(&info->work);
    return IRQ_HANDLED; 
}

static int usr_get_dts_info(struct reboot_key_info * info, struct device *dev)
{
	int ret;
	struct device_node *node = NULL;
	node = of_find_compatible_node(NULL, NULL, "rockchip, reboot-test");
	if (!node) {
		usr_msg("cannot find node name rockchip, reboot-test");
		return -ENODEV;
	}
	info->reboot_trigger_io = of_get_named_gpio(node, "reboot-key", 0);
	if(!gpio_is_valid(info->reboot_trigger_io)) {
		usr_msg("error: get reboot-key gpio info. ERROR code %d", info->reboot_trigger_io);
		return -EIO;
	}
	ret = of_property_read_u32(node, "detect-interval", &info->detect_interval);
    if(ret < 0) {
        usr_msg("error: read detect-interval.  ERROR code %d", info->detect_interval);
        return -EINVAL;
    }
	ret = devm_gpio_request_one(dev, info->reboot_trigger_io, GPIOF_IN, "reboot-test-key-gpios");
    if(ret < 0) {
		usr_msg("error: reboot-key-gpios request.");
		return -EIO;
	}
	debug_msg("requested reboot_trigger_io number: %d, info->detect_interval = %d\n", info->reboot_trigger_io, info->detect_interval);
	
	return 0;

}

#if USING_SYS_FS
 static ssize_t reboot_running(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
 {
 	unsigned long state, timer_on_ticks, timer_off_ticks;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;
	timer_on_ticks = 200;
	timer_off_ticks = 2000;
	debug_msg("---> moved in function: %s", __func__);
	ret = kstrtoul(buf, 10, &state);
	if (ret)
		return ret;
	debug_msg("---> in function: %s, state = %d", __func__);
	if(state) {
		led_blink_set(led_cdev, &timer_on_ticks, &led_cdev->blink_delay_off);
		led_cdev->blink_delay_on = timer_on_ticks;
		led_blink_set(led_cdev, &led_cdev->blink_delay_on, &timer_off_ticks);
		led_cdev->blink_delay_off = timer_off_ticks;
	}
	reboot_status_flag = 1;
	return size;
 }

 static ssize_t reboot_pass(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	 struct led_classdev *led_cdev = dev_get_drvdata(dev);
	 unsigned long state, timer_on_ticks, timer_off_ticks;
	 ssize_t ret = -EINVAL;
	 timer_on_ticks = 2000;
	 timer_off_ticks = 2000;
	 debug_msg("---> moved in function: %s", __func__);
	 ret = kstrtoul(buf, 10, &state);
	 if (ret)
		 return ret;
	 debug_msg("---> in function: %s, state = %d", __func__);
	 if(state) {
		 led_blink_set(led_cdev, &timer_on_ticks, &led_cdev->blink_delay_off);
		 led_cdev->blink_delay_on = timer_on_ticks;
		 led_blink_set(led_cdev, &led_cdev->blink_delay_on, &timer_off_ticks);
		 led_cdev->blink_delay_off = timer_off_ticks;
	 }
	 reboot_status_flag = 2;
	 return size;

}
 
static ssize_t reboot_failed(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	 
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	unsigned long state, timer_on_ticks, timer_off_ticks;
	ssize_t ret = -EINVAL;
	timer_on_ticks = 200;
	timer_off_ticks = 200;
	debug_msg("---> moved in function: %s", __func__);
	ret = kstrtoul(buf, 10, &state);
	 if (ret)
		 return ret;
	 debug_msg("---> in function: %s, state = %d", __func__);
	 if(state) {
		 led_blink_set(led_cdev, &timer_on_ticks, &led_cdev->blink_delay_off);
		 led_cdev->blink_delay_on = timer_on_ticks;
		 led_blink_set(led_cdev, &led_cdev->blink_delay_on, &timer_off_ticks);
		 led_cdev->blink_delay_off = timer_off_ticks;
	 }
	 reboot_status_flag = 3;
	 return size;

}
static ssize_t reboot_status(struct device *dev, struct device_attribute*attr, char *buf)
{
	if(0 == reboot_status_flag)
		return sprintf(buf, "%s\n", "standby");
	else if(1 == reboot_status_flag)
		return sprintf(buf, "%s\n", "running");
	else if(2 == reboot_status_flag)
		return sprintf(buf, "%s\n", "pass");
	else if(3 == reboot_status_flag)
		return sprintf(buf, "%s\n", "failed");
	return 0;
}
static ssize_t reboot_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	 return size;
}

static ssize_t reboot_running_show(struct device *dev, struct device_attribute*attr, char *buf)
{
	return sprintf(buf, "%s\n", "not use");
}
static ssize_t reboot_pass_show(struct device *dev, struct device_attribute*attr, char *buf)
{
	return sprintf(buf, "%s\n", "not use");
}
static ssize_t reboot_failed_failed(struct device *dev, struct device_attribute*attr, char *buf)
{
	return sprintf(buf, "%s\n", "not use");
}

static DEVICE_ATTR(reboot_test_running,     0664, reboot_running_show, reboot_running);
static DEVICE_ATTR(reboot_test_pass,        0664, reboot_pass_show, reboot_pass);
static DEVICE_ATTR(reboot_test_failed,   	0664, reboot_failed_failed, reboot_failed);
static DEVICE_ATTR(reboot_test_status,  	0664, reboot_status, reboot_status_store);

static int reboot_key_sysfs_create(struct platform_device * pdev)
{
	int rc;
	
	rc = device_create_file(&pdev->dev, &dev_attr_reboot_test_running);
	if (rc) {
		debug_msg("---> in function: %s, create fs led_runing_reboot failed", __func__);
		return -1;
	}
	rc = device_create_file(&pdev->dev, &dev_attr_reboot_test_pass);
	if (rc) {
		debug_msg("---> in function: %s, create fs dev_attr_led_test_pass_reboot failed", __func__);
		return -1;
	}
	rc = device_create_file(&pdev->dev, &dev_attr_reboot_test_failed);
	if (rc) {
		debug_msg("---> in function: %s, create fs dev_attr_led_test_failed_reboot failed", __func__);
		return -1;
	}
	rc = device_create_file(&pdev->dev, &dev_attr_reboot_test_status);
	if (rc) {
		debug_msg("---> in function: %s, create fs dev_attr_reboot_status failed", __func__);
		return -1;
	}
	
	return 0;
	
}

static void reboot_key_sysfs_remove(struct platform_device * pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_reboot_test_running);
	device_remove_file(&pdev->dev, &dev_attr_reboot_test_pass);
	device_remove_file(&pdev->dev, &dev_attr_reboot_test_failed);
	device_remove_file(&pdev->dev, &dev_attr_reboot_test_status);
}
#endif

static int reboot_key_probe(struct platform_device *pdev)
{
	int ret;

	struct reboot_key_info * info = devm_kmalloc(&pdev->dev, sizeof(struct reboot_key_info), GFP_KERNEL);
	if(IS_ERR(info)) {
		usr_msg("error: devm_kmalloc");
		return -ENOMEM;
	}
	debug_msg("moved in function: %s", __func__);
	mutex_init(&info->lock);

	platform_set_drvdata(pdev, info);
	dev_set_drvdata(&pdev->dev, info);
	

	info->inputdev = devm_input_allocate_device(&pdev->dev);
	if(IS_ERR(info->inputdev)){
		usr_msg("error: input_allocate_device failed!");
		return -ENODEV;
	}

	info->inputdev->name 		= KEY_DEV_NAME;				
	info->inputdev->phys 		= KEY_DEV_NAME;	
	info->inputdev->dev.parent 	= &pdev->dev;
	info->inputdev->id.bustype 	= BUS_HOST;		
    info->inputdev->id.vendor 	= 0x0001;
    info->inputdev->id.product 	= 0x0001;
    info->inputdev->id.version 	= 0x0100;
    info->inputdev->evbit[0] = BIT_MASK(EV_KEY);

	set_bit(KEY_VAL, info->inputdev->keybit);
	input_set_capability(info->inputdev, EV_KEY, KEY_VAL);
	
	ret = input_register_device(info->inputdev);
	if(ret){
		usr_msg("error: input register device failed!");
		goto error1;
	}


	ret = usr_get_dts_info(info, &pdev->dev);
	if(ret < 0) {
		ret = -ENODEV;
		usr_msg("error: get gpio dts info");
		goto error1;
	}

	info->wq = create_singlethread_workqueue("reboot_key_data_handler");
	if (!info->wq) {
		usr_msg("error: can not create workqueue");
		goto error2;
	}
	flush_workqueue(info->wq);
	INIT_WORK(&info->work, data_handler);

	info->irq = gpio_to_irq(info->reboot_trigger_io);
	ret = request_irq(info->irq, reboot_trigger_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "reboot_key_irq", (void *)info);
	if(ret < 0) {
		usr_msg("error: request irq");
		goto error3;
	}
	debug_msg("requested irq number = %d", info->irq);
	platform_set_drvdata(pdev, (void *) info);
	dev_set_drvdata(&pdev->dev, (void *) info);
	
#if USING_SYS_FS
	reboot_key_sysfs_create(pdev);
#endif
	return 0;


error3:
	destroy_workqueue(info->wq);

error2:
	input_put_device(info->inputdev);
	input_free_device(info->inputdev);
error1:

   return ret;

}

static int reboot_key_remove(struct platform_device *pdev)
{
	struct reboot_key_info *btn = platform_get_drvdata(pdev);
	
	cancel_work_sync(&btn->work);
	flush_workqueue(btn->wq);
	destroy_workqueue(btn->wq);
	input_unregister_device(btn->inputdev);
	devm_gpio_free(&pdev->dev, btn->reboot_trigger_io);
	
#if USING_SYS_FS
	reboot_key_sysfs_remove(pdev);
#endif

	return 0;
}

#if 0
#ifdef CONFIG_PM	
static int reboot_key_suspend(struct device *dev)
{
	struct reboot_key_info *btn = dev_get_drvdata(dev);
	usr_msg("ready to suspend");
	disable_irq_nosync(btn->irq);

	cancel_work_sync(&btn->work);
	flush_workqueue(btn->wq);
	gpio_set_value(btn->touch_ic_rst, 0);
	usr_msg("customer key suspended.");
	return 0;
}

static int reboot_key_resume(struct device *dev)
{
	struct reboot_key_info *btn = dev_get_drvdata(dev);
	usr_msg("ready to resume");
	chip_cp2610_init(btn);

	enable_irq(btn->irq);
	usr_msg("customer key resumed operation.");
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(reboot_key_pm_ops, reboot_key_suspend, reboot_key_resume);
#endif

static struct platform_driver reboot_key_driver = {
	.probe		= reboot_key_probe,
	.remove		= reboot_key_remove,
	.driver		= {
		.name	= KEY_DEV_NAME,
		.owner	= THIS_MODULE,
		// .pm	= &reboot_key_pm_ops,
		.of_match_table = of_match_ptr(reboot_key_match_table),
	},
};


module_platform_driver(reboot_key_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("reboot_key control loop");
MODULE_ALIAS("reboot_key");

