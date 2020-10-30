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
#include <linux/spinlock.h>
#include <linux/io.h>
#include <uapi/linux/input.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/delay.h>


// define in /include/uapi/linux/input.h : #define #define KEY_USR_CUSTOMER_DEF    249 

#define KEY_VAL				KEY_BACK
#define KEY_NAME			"usr_home_button"
#define TAG					" <KEY> "
#define SET_LEVEL			KERN_INFO
#define usr_msg(fmt, args...)			printk(SET_LEVEL TAG fmt"\n", ##args)

typedef enum {
	low = 0,
	high = !low
}gpio_status;





static struct of_device_id key_dts_table[] = {
	{ .compatible = "usr-home-button", },
	{ },
};
MODULE_DEVICE_TABLE(of, key_dts_table);

/*--------------------------------------------------------------------------------------------------------------*/

#define MS_TO_NS(x) (x * 1000000)      // ms to ns

struct usr_keys_button {
	int 						touch_home_btn;
	int 						touch_ic_rst;
	unsigned int 				irq;
	struct input_dev 			*inputdev;
	struct mutex 				lock;
};
struct usr_keys_button * btn;

struct timer_strcuture {
	ktime_t				tim_period;
	struct 	mutex		tim_mutex;
	struct	hrtimer 	timer;
	struct mutex		tim_lock;
};

static struct timer_strcuture tim;

static enum hrtimer_restart foo_hrtimer_callback(struct hrtimer * arg)
{
	static unsigned int loop_ticks;
	int key_val;
	ktime_t now;

	loop_ticks++;
    now = arg->base->get_time();

	key_val = gpio_get_value(btn->touch_home_btn);
	if(key_val) {
		usr_msg("read pin: %d, get key_val = %d", btn->touch_home_btn, key_val);
		input_report_key(btn->inputdev, KEY_VAL, 1);
		input_sync(btn->inputdev);
	}else {
        usr_msg("read pin: %d, get key_val = %d", btn->touch_home_btn, key_val);
		input_report_key(btn->inputdev, KEY_VAL, 0);
		input_sync(btn->inputdev);
    }
	if(0 == (loop_ticks % 5)) {
		usr_msg("gpio touch_ic_rst set 1");
		gpio_set_value(btn->touch_ic_rst, 1);
	}
	if(loop_ticks > 65535)
		loop_ticks = 0;
	
    hrtimer_forward(arg, now, tim.tim_period);
	
    return HRTIMER_RESTART;
}


static void hrt_timer_init(s64 seconds, unsigned long mseconds)
{
    tim.tim_period = ktime_set(seconds, MS_TO_NS(mseconds));    
    hrtimer_init(&tim.timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    tim.timer.function = foo_hrtimer_callback;
    hrtimer_start(&tim.timer, tim.tim_period, HRTIMER_MODE_REL);
	mutex_init(&tim.tim_lock);
}

static void chip_cp2610_init(struct usr_keys_button * btn)
{
	gpio_set_value(btn->touch_ic_rst, 1);
	mdelay(200);
}

static int usr_get_dts_info(struct usr_keys_button * btn, struct device *dev)
{
	int ret;
	struct device_node *node;
	
	node = of_find_node_by_name(NULL, "usr-home-button"); 
	if (!node) {
		usr_msg("cannot find node name usr-home-button");
		return -ENODEV;
	}
	btn->touch_home_btn = of_get_named_gpio(node, "touch-home-btn", 0);
	if(!gpio_is_valid(btn->touch_home_btn)) {
		usr_msg("error: get touch_home_btn gpio info. ERROR code %d", btn->touch_home_btn);
		return -EIO;
	}
    btn->touch_ic_rst = of_get_named_gpio(node, "touch-rst-ctrl", 0);
    if(!gpio_is_valid(btn->touch_ic_rst)) {
        usr_msg("error: get touch_ic_rst gpio info.  ERROR code %d", btn->touch_ic_rst);
        return -EIO;
    }
	ret = devm_gpio_request_one(dev, btn->touch_home_btn, GPIOF_OUT_INIT_HIGH, "touch_home_btn");
	if(ret < 0) {
		usr_msg("error: gpio touch_home_btn request.");
		return -EIO;
	}
	ret = devm_gpio_request_one(dev, btn->touch_ic_rst, GPIOF_IN, "touch_rst_ctrl");
    if(ret < 0) {
		usr_msg("error: touch_ic_rst request.");
		ret = -EIO;
		goto out1;
	}
	gpio_direction_input(btn->touch_home_btn);
	gpio_direction_output(btn->touch_ic_rst, 1);
	usr_msg("requested home_btn number: %d, home_ctrl_rst number: %d", btn->touch_home_btn, btn->touch_ic_rst);

	return 0;

out1:
	devm_gpio_free(dev, btn->touch_home_btn);
	return ret;
}

static int key_probe(struct platform_device *pdev)
{
	int ret;
#if 0
	struct usr_keys_button * button = devm_kmalloc(&pdev->dev, sizeof(struct usr_keys_button), GFP_KERNEL);
	if(IS_ERR(button)) {
		usr_msg("error: devm_kmalloc");
		return -ENOMEM;
	}
#endif

	btn = devm_kmalloc(&pdev->dev, sizeof(struct usr_keys_button), GFP_KERNEL);
	if(IS_ERR(btn)) {
		usr_msg("error: devm_kmalloc");
		return -ENOMEM;
	}

	mutex_init(&btn->lock);

	platform_set_drvdata(pdev, btn);
	dev_set_drvdata(&pdev->dev, btn);
	
	btn->inputdev = devm_input_allocate_device(&pdev->dev);
	if(IS_ERR(btn->inputdev)){
		usr_msg("error: input_allocate_device failed!");
		return -ENODEV;
	}

	btn->inputdev->name 		= KEY_NAME;				
	btn->inputdev->phys 		= KEY_NAME;	
	btn->inputdev->dev.parent 	= &pdev->dev;
	btn->inputdev->id.bustype 	= BUS_HOST;		
    btn->inputdev->id.vendor = 0x0001;
    btn->inputdev->id.product = 0x0001;
    btn->inputdev->id.version = 0x0100;
    btn->inputdev->evbit[0] = BIT_MASK(EV_KEY);
	// set_bit(EV_KEY, btn->inputdev->evbit);		
	// set_bit(KEY_VAL, btn->inputdev->keybit); 
	set_bit(KEY_VAL, btn->inputdev->keybit);
	input_set_capability(btn->inputdev, EV_KEY, KEY_VAL);

	ret = usr_get_dts_info(btn, &pdev->dev);
	if(ret < 0) {
		usr_msg("error: get gpio dts info");
		ret = -ENODEV;
		goto error1;
	}
	chip_cp2610_init(btn);
	ret = input_register_device(btn->inputdev);
	if(ret){
		usr_msg("error: input register device failed!");
		goto error1;
	}
	hrt_timer_init(0, 800);	// set ticks to 1s

	// dev_set_drvdata(&pdev->dev, (void *)btn);
	return 0;  
	
error1:
   input_free_device(btn->inputdev);
   kfree(btn);
   return ret;

}

static int key_remove(struct platform_device *pdev)
{
	// struct usr_keys_button *btn = platform_get_drvdata(pdev);
	input_unregister_device(btn->inputdev);
	
	while(hrtimer_try_to_cancel(&tim.timer));
	// while(hrtimer_cancel(&tim.timer));
	kfree(btn);
	return 0;
}

static int key_suspend (struct device *dev)
{
	// struct usr_keys_button *button = dev_get_drvdata(dev);
	gpio_set_value(btn->touch_ic_rst, 0);
	mdelay(500);
	return 0;
}

static int key_resume (struct device *dev)
{
	// struct usr_keys_button *button = dev_get_drvdata(dev);
	gpio_direction_input(btn->touch_home_btn);
	gpio_direction_output(btn->touch_ic_rst, 1);
	gpio_set_value(btn->touch_ic_rst, 1);
	mdelay(500);
	return 0;

}

static struct dev_pm_ops customer_keys_pm_ops = {
	.suspend = key_suspend,
	.resume = key_resume,
};

// struct platform_device_id key_id[] = {
//	{.name = "key", .driver_data = 0, },
// };

static struct platform_driver keys_pdrv = {
	.probe		= key_probe,
	.remove		= key_remove,
	.driver		= {
		.name	= KEY_NAME,
		.owner	= THIS_MODULE,
		// #ifdef CONFIG_PM
		.pm	= &customer_keys_pm_ops,
		// #endif
		.of_match_table = of_match_ptr(key_dts_table),
	},
	// .id_table = key_id,
};

module_platform_driver(keys_pdrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("customer gpio key define");
MODULE_ALIAS("customer-keys");
