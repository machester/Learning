/*****************************************************************************
 * ; key define in dts
 *  [usr-home-button]
 *  compatible = "usr-home-button"
 *  touch-home-btn = port:PL10<6><1><default><default>
 *  touch-rst-ctrl = port:PL8<1><1><default><1>
 ******************************************************************************/
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


// define in /include/uapi/linux/input.h : #define #define KEY_HOMEPAGE 

// #define KEY_VAL						KEY_BACK
#define KEY_VAL				    		KEY_HOMEPAGE		// android home key
#define KEY_NAME						"usr_home_button"
#define TAG								" <KEY> "
#define SET_LEVEL						KERN_INFO
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
#define HRT_TIMER				0

#define MS_TO_NS(x) (x * 1000000)      // ms to ns

struct usr_keys_button {
	int 						touch_home_btn;
	int 						touch_ic_rst;
	unsigned int 				irq;
	struct input_dev 			*inputdev;
	struct mutex 				lock;
	struct workqueue_struct 	* wq;
	struct work_struct			work;
// #if HRT_TIMER
// 	/** hrt timer --------------------------------------------------*/
// 	ktime_t						tim_period;
// 	struct	hrtimer 			tim;
// #else
// 	/** standard timer ---------------------------------------------*/
// 	struct timer_list 			tim;
// 	unsigned int 				tim_seconds;	
// #endif
// 	struct mutex				tim_lock;
};

static int trigger_flag;

static void data_handler(struct work_struct * work)
{
	int key_val;
	struct usr_keys_button * btn = container_of(work, struct usr_keys_button, work);
	if(IS_ERR(btn)) {
		usr_msg("error: get struct usr_keys_button in function: %s", __func__);
		return ;
	}
	usr_msg("moved in %s", __func__);
	if(0 == trigger_flag) {
		usr_msg("handler button half schedule");
		return ;
	} else {
		mutex_lock(&btn->lock);
		key_val = gpio_get_value(btn->touch_home_btn);
		if(key_val) {
			mdelay(150);
			if(key_val == gpio_get_value(btn->touch_home_btn)) {
				usr_msg("release");
				input_report_key(btn->inputdev, KEY_VAL, 1);
				input_sync(btn->inputdev);
			} else
				goto out;
		} else {
			mdelay(150);
			if(key_val == gpio_get_value(btn->touch_home_btn)) {
	        	usr_msg("pressed");
	        	input_report_key(btn->inputdev, KEY_VAL, 0);
				input_sync(btn->inputdev);
			} else
				goto out;
		}
		trigger_flag = 0;
		mutex_unlock(&btn->lock);
	}

	return ;
out:
	trigger_flag = 0;
	mutex_unlock(&btn->lock);
	return ;
}

static irqreturn_t usr_key_handler(int irq, void *arg)
{
	struct usr_keys_button * btn = (struct usr_keys_button *)arg;
	if(!btn) {
		usr_msg("error: usr_keys_button in function: %s", __func__);
		return IRQ_HANDLED;
	}
	usr_msg("triggred");
    trigger_flag += 1;
	schedule_work(&btn->work);
    return IRQ_HANDLED; 
}

static void chip_cp2610_init(struct usr_keys_button * btn)
{
	gpio_set_value(btn->touch_ic_rst, 0);
	mdelay(500);
	gpio_set_value(btn->touch_ic_rst, 1);
	mdelay(200);
}

// #if HRT_TIMER
// static enum hrtimer_restart usr_hrtimer_callback(struct hrtimer * arg)
// {
// 	ktime_t now;
// 	struct usr_keys_button * btn = container_of(arg, struct usr_keys_button, tim);
// 	if(!btn) {
// 		usr_msg("error: get struct timer_structure in function: %s", __func__);
// 	}	
// 	usr_msg("tick count, reset chip");
// 	mutex_lock(&btn->tim_lock);
//     now = arg->base->get_time();
// 	chip_cp2610_init(btn);
//     hrtimer_forward(arg, now, btn->tim_period);
// 	mutex_unlock(&btn->tim_lock);
//     return HRTIMER_RESTART;
// }

// static void hrt_timer_init(struct usr_keys_button	* btn, s64 seconds, unsigned long mseconds)
// {
//     btn->tim_period = ktime_set(seconds, MS_TO_NS(mseconds));    
//     hrtimer_init(&btn->tim, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
//     btn->tim.function = usr_hrtimer_callback;
	
//     hrtimer_start(&btn->tim, btn->tim_period, HRTIMER_MODE_REL);
// 	mutex_init(&btn->tim_lock);
// }

// static void hrt_timer_del(struct usr_keys_button	* btn)
// {
// 	while(hrtimer_try_to_cancel(&btn->tim));
// 	hrtimer_cancel(&btn->tim);
// }
// #else
// static void usr_standard_timer_callback(unsigned long arg)
// {
// 	struct usr_keys_button * btn = (struct usr_keys_button *)arg;
// 	if(!btn) {
// 		usr_msg("error: get struct address in function: %s", __func__);
// 		return ;
// 	}
// 	usr_msg("tick count, reset chip");
// 	chip_cp2610_init(btn);
// 	mod_timer(&btn->tim, (jiffies + btn->tim_seconds * HZ));
// }

// static void standard_timer_init(struct usr_keys_button	* btn, s64 seconds)
// {
// 	mutex_init(&btn->tim_lock);
// 	init_timer(&btn->tim);
// 	btn->tim.function = usr_standard_timer_callback;
// 	btn->tim.data = (unsigned long)btn;
// 	btn->tim.expires = jiffies + seconds * HZ;
// 	btn->tim_seconds = seconds;
// 	add_timer(&btn->tim);
// }

// static void standard_timer_del(struct usr_keys_button	* btn)
// {
// 	// del_timer_sync(&btn->tim);
// 	del_timer(&btn->tim);
// }
// #endif	// end of #if HRT_TIMER

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

	struct usr_keys_button * btn = devm_kmalloc(&pdev->dev, sizeof(struct usr_keys_button), GFP_KERNEL);
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
    btn->inputdev->id.vendor 	= 0x0001;
    btn->inputdev->id.product 	= 0x0001;
    btn->inputdev->id.version 	= 0x0100;
    btn->inputdev->evbit[0] = BIT_MASK(EV_KEY);

	set_bit(KEY_VAL, btn->inputdev->keybit);
	input_set_capability(btn->inputdev, EV_KEY, KEY_VAL);
	
	ret = input_register_device(btn->inputdev);
	if(ret){
		usr_msg("error: input register device failed!");
		goto error1;
	}
	
	ret = usr_get_dts_info(btn, &pdev->dev);
	if(ret < 0) {
		ret = -ENODEV;
		usr_msg("error: get gpio dts info");
		goto error2;
	}
	chip_cp2610_init(btn);
	
	trigger_flag = 0;
	btn->wq = create_singlethread_workqueue("key_data_handler");
	if (!btn->wq) {
		usr_msg("error: can not create workqueue");
		goto error2;
	}
	flush_workqueue(btn->wq);
	INIT_WORK(&btn->work, data_handler);

	btn->irq = gpio_to_irq(btn->touch_home_btn);
	ret = request_irq(btn->irq, usr_key_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "usr_home_btn_irq", (void *)btn);
	if(ret < 0) {
		usr_msg("error: request irq");
		goto error3;
	}
	usr_msg("requested irq number = %d", btn->irq);
// #if HRT_TIMER
// 	hrt_timer_init(btn, 10, 0);
// #else
// 	standard_timer_init(btn, 30);
// #endif
	platform_set_drvdata(pdev, (void *) btn);
	dev_set_drvdata(&pdev->dev, (void *) btn);
	return 0;

error3:
	destroy_workqueue(btn->wq);
error2:
	input_put_device(btn->inputdev);
error1:
   input_free_device(btn->inputdev);
   return ret;
}

static int key_remove(struct platform_device *pdev)
{
	struct usr_keys_button *btn = platform_get_drvdata(pdev);
	
	cancel_work_sync(&btn->work);
	flush_workqueue(btn->wq);
	destroy_workqueue(btn->wq);
// #if HRT_TIMER
// 	hrt_timer_del(btn);
// #else
// 	standard_timer_del(btn);
// #endif
	input_unregister_device(btn->inputdev);
	
	return 0;
}


#ifdef CONFIG_PM	
static int key_suspend(struct device *dev)
{
	struct usr_keys_button *btn = dev_get_drvdata(dev);
	usr_msg("ready to suspend");
	disable_irq_nosync(btn->irq);
// #if HRT_TIMER
// 	hrt_timer_del(btn);
// #else
// 	standard_timer_del(btn);
// #endif
	cancel_work_sync(&btn->work);
	flush_workqueue(btn->wq);
	gpio_set_value(btn->touch_ic_rst, 0);
	usr_msg("customer key suspended.");
	return 0;
}

static int key_resume(struct device *dev)
{
	struct usr_keys_button *btn = dev_get_drvdata(dev);
	usr_msg("ready to resume");
	chip_cp2610_init(btn);
// #if HRT_TIMER
// 	hrt_timer_init(btn, 10, 0);
// #else
// 	standard_timer_init(btn, 30);
// #endif
	enable_irq(btn->irq);
	usr_msg("customer key resumed operation.");
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(customer_keys_pm_ops, key_suspend, key_resume);

static struct platform_driver keys_pdrv = {
	.probe		= key_probe,
	.remove		= key_remove,
	.driver		= {
		.name	= KEY_NAME,
		.owner	= THIS_MODULE,
		.pm	= &customer_keys_pm_ops,
		.of_match_table = of_match_ptr(key_dts_table),
	},
};

module_platform_driver(keys_pdrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("customer gpio key define");
MODULE_ALIAS("customer-keys");
