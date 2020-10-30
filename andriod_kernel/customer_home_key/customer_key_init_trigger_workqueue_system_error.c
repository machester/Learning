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
#include <linux/pinctrl/pinconf-sunxi.h>
#include <linux/sys_config.h>
#include <uapi/linux/input.h>
#include <linux/workqueue.h>
#include <linux/wait.h>

// define in /include/uapi/linux/input.h : #define #define KEY_USR_CUSTOMER_DEF    249 

#define KEY_VAL				KEY_HOME		
#define KEY_NAME			"Usr_Home_Button"
#define TAG					" <KEY> "
#define SET_LEVEL			KERN_INFO
#define usr_msg(fmt, args...)			printk(SET_LEVEL TAG fmt"\n", ##args)

#if 0


#define MS_TO_NS(x) (x * 1000000)      // ms to ns

static ktime_t  tim_period;
struct mutex    foo_mutex;
static struct   hrtimer     foo_timer;
int 				gpio_num;

static struct of_device_id key_dts_table[] = {
	{ .compatible = "usr-home-button", },
	{ },
};
MODULE_DEVICE_TABLE(of, key_dts_table);

static enum hrtimer_restart foo_hrtimer_callback(struct hrtimer * arg)
{
	static int loop_ticks = 0;
	
    ktime_t now = arg->base->get_time();
    loop_ticks += 1;
	if(0 == (loop_ticks %2 )) {
		usr_msg("set gpio to high");
		gpio_set_value(gpio_num, 1);
	} else {
		usr_msg("set gpio to low");
		gpio_set_value(gpio_num, 0);
	}
    hrtimer_forward(arg, now, tim_period);
	
    return HRTIMER_RESTART;
}

static int usr_get_dts_info(struct device *dev)
{
	struct device_node *node;
	
	node = of_find_node_by_name(NULL, "usr-home-button"); 
	if (!node) {
		usr_msg("cannot find node name usr-home-button");
		return -ENODEV;
	}
	gpio_num = of_get_named_gpio(node, "home_button", 0);
	if(gpio_num < 0) {
		usr_msg("error: get dts gpio info");
		return -EIO;
	}
	if(gpio_request(gpio_num, "home_button") < 0) {
		usr_msg("error: gpio request.");
		return -EIO;
	}
	gpio_direction_output(gpio_num, 0);
	usr_msg("requested gpio number: %d", gpio_num);

	return 0;
}

static int key_probe(struct platform_device *pdev)
{
	int ret;
	usr_msg("moved int funciton %s", __func__);
	ret = usr_get_dts_info(&pdev->dev);
	if(ret < 0) {
		usr_msg("error: get dts info");
		ret = -EIO;
	}
    mutex_init(&foo_mutex);
    tim_period = ktime_set(2, 0);     // set 2second
    hrtimer_init(&foo_timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    foo_timer.function = foo_hrtimer_callback;
    hrtimer_start(&foo_timer, tim_period, HRTIMER_MODE_REL);
	usr_msg("moved out funciton %s", __func__);
	return 0;  
}

static int key_remove(struct platform_device *pdev)
{
	gpio_free(gpio_num);
	while(hrtimer_cancel(&foo_timer));
	return 0;
}

static struct platform_driver keys_pdrv = {
	.probe		= key_probe,
	.remove		= key_remove,
	.driver		= {
		.name	= KEY_NAME,
		.owner	= THIS_MODULE,
		// .pm	= &gpio_keys_pm_ops,
		.of_match_table = of_match_ptr(key_dts_table),
	},
};

static int __init customer_key_init(void)
{	
	usr_msg("moved  int funciton %s", __func__);
	return platform_driver_register(&keys_pdrv);
}

static void __exit customer_key_exit(void)
{
	usr_msg("moved  int funciton %s", __func__);
	platform_driver_unregister(&keys_pdrv);
}


#else

struct usr_keys_button {
	int 				gpio_num;
	unsigned int 		irq;
	struct work_struct  work;
	struct input_dev 	*inputdev;
	struct mutex 		lock;
};

static int work_trigger_flag;

static DECLARE_WAIT_QUEUE_HEAD(key_queue_head);

static struct of_device_id key_dts_table[] = {
	{ .compatible = "usr-home-button", },
	{ },
};
MODULE_DEVICE_TABLE(of, key_dts_table);

static void queue_recall_function(struct work_struct *work)
{
	int key_val;
	struct usr_keys_button * btn = container_of(work, struct usr_keys_button, work);
	if(IS_ERR(btn)) {
		usr_msg("error : get struct usr_keys_button");
		
		work_trigger_flag = 0;
		return ;
	}
	mutex_lock(&btn->lock);
	usr_msg("handler dealing button half");
 
    key_val = gpio_get_value(btn->gpio_num);

    usr_msg("key_val = %d", key_val);
    if(key_val == 0){
        usr_msg("pressed");
        input_report_key(btn->inputdev, KEY_VAL, 0);
        input_sync(btn->inputdev);

    }else if(key_val == 1){
        usr_msg("release");
        input_report_key(btn->inputdev, KEY_VAL, 1);
        input_sync(btn->inputdev);
    }
	wake_up_interruptible(&key_queue_head);
	work_trigger_flag = 0;
	mutex_unlock(&btn->lock);
	// schedule();
}

static irqreturn_t usr_key_handler(int irq, void *arg)
{
	usr_msg("triggred");
    work_trigger_flag += 1;
    return IRQ_HANDLED; 
}

static int usr_get_dts_info(struct usr_keys_button * button, struct device *dev)
{
	int ret;
	struct device_node *node;
	
	node = of_find_node_by_name(NULL, "usr-home-button"); 
	if (!node) {
		usr_msg("cannot find node name usr-home-button");
		return -ENODEV;
	}
	button->gpio_num = of_get_named_gpio(node, "home_button", 0);
	if(button->gpio_num < 0) {
		usr_msg("error: get dts gpio info");
		return -EIO;
	}
	if(gpio_request(button->gpio_num, "home_button") < 0) {
		usr_msg("error: gpio request.");
		return -EIO;
	}
	usr_msg("requested gpio number: %d", button->gpio_num);
	gpio_direction_input(button->gpio_num);

	button->irq = gpio_to_irq(button->gpio_num);
	if(button->irq < 0) {
		usr_msg("error: map gpio to irq");
		ret = -EIO;
		goto err_map_irq;
	}
	ret = request_irq(button->irq, usr_key_handler, IRQF_TRIGGER_FALLING, "usr-home-button", NULL);

err_map_irq:
	gpio_free(button->gpio_num);
	return ret;
}

static int key_probe(struct platform_device *pdev)
{
	int ret;

	struct usr_keys_button * button = devm_kmalloc(&pdev->dev, sizeof(struct usr_keys_button), GFP_KERNEL);
	if(IS_ERR(button)) {
		usr_msg("error: devm_kmalloc");
		return -ENOMEM;
	}

	mutex_init(&button->lock);
	
    button->inputdev = input_allocate_device();
    if(IS_ERR(button->inputdev)){
        usr_msg("error: input_allocate_device failed!");
        return = -ENODEV;
    }

    button->inputdev->name = KEY_NAME;  			
    button->inputdev->phys = "input3"; 				
    button->inputdev->id.bustype = BUS_HOST;  		
    button->inputdev->id.vendor = 0x0001;
    button->inputdev->id.product = 0x0001;
    button->inputdev->id.version = 0x0100;

    set_bit(EV_KEY, button->inputdev->evbit);		
    set_bit(KEY_VAL, button->inputdev->keybit);		

	ret = usr_get_dts_info(button, &pdev->dev);
	if(ret < 0) {
		usr_msg("error: get dts info");
		ret = -ENODEV;
		goto error1;
	}
    ret = input_register_device(button->inputdev);
    if(ret){
        usr_msg("error: input register device failed!");
        goto error2;
    }

    work_trigger_flag = 0;
    init_waitqueue_head(&key_queue_head);
    INIT_WORK(&button->work, queue_recall_function);
    schedule_work(&button->work);
    wait_event_interruptible(key_queue_head, work_trigger_flag != 0);
    usr_msg("init work_trigger_flag =  %d", work_trigger_flag);

	platform_set_drvdata(pdev, button);

	return 0;  
	
error2:
	gpio_free(button->gpio_num);    
error1:
   input_free_device(button->inputdev);
   return ret;
}

static int key_remove(struct platform_device *pdev)
{
	struct usr_keys_button *btn = platform_get_drvdata(pdev);
	input_unregister_device(btn->inputdev);
	while(false == cancel_work_sync(&btn->work));
	destroy_work_on_stack(&btn->work);
	kfree(btn);
	return 0;
}


static struct platform_driver keys_pdrv = {
	.probe		= key_probe,
	.remove		= key_remove,
	.driver		= {
		.name	= KEY_NAME,
		.owner	= THIS_MODULE,
		// .pm	= &gpio_keys_pm_ops,
		.of_match_table = of_match_ptr(key_dts_table),
	}
};

static int __init customer_key_init(void)
{	
	usr_msg("moved  int funciton %s", __func__);
	return platform_driver_register(&keys_pdrv);
}

static void __exit customer_key_exit(void)
{
	usr_msg("moved  int funciton %s", __func__);
	platform_driver_unregister(&keys_pdrv);
}


#endif
// module_platform_driver(keys_pdrv);
module_init(customer_key_init);
module_exit(customer_key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("customer gpio key define");
MODULE_ALIAS("customer-keys");