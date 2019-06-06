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

// #define KEY_VAL							251	//0xFB
#define KEY_VAL							KEY_RIGHTMETA
#define TAG								" <MCU-KEY> "
#define SET_LEVEL						KERN_INFO
#define usr_msg(fmt, args...)			printk(SET_LEVEL TAG fmt"\n", ##args)

#define KEY_NAME						"r100u_mcu_key"
#define  MCU_DEV_NAME 				    "r100u-mcu-dev"

#define HRT_TIMER				        0
#define DIRECT_CREATE_SYSFS				0

#define MS_TO_NS(x) (x * 1000000)      // ms to ns

typedef enum {
	low = 0,
	high = 1
}gpio_status;

typedef enum {
	in = 0,
	out = !in
}gpio_dir;

static int trigger_flag;

static int sn8p27113aa_set_value(int gpio, gpio_dir dir, gpio_status value);
static irqreturn_t mcu_trigger_handler(int irq, void *arg);

static const struct of_device_id mcu_key_match_table[] = {
	{ .compatible = "rockchip,r100u-mcu-key",},
	{},
};
MODULE_DEVICE_TABLE(of, mcu_key_match_table);

/*--------------------------------------------------------------------------------------------------------------*/

struct sn8p27113aa_info {
	int 						mcu_p53_gpio;
	int 						mcu_p42_gpio;
	unsigned int 				irq;
	struct input_dev 			*inputdev;
	struct mutex 				lock;
	struct workqueue_struct 	* wq;
	struct work_struct			work;
	int key_repeat;
#if HRT_TIMER
	/** hrt timer --------------------------------------------------*/
	ktime_t						tim_period;
	struct	hrtimer 			tim;
#endif
	struct mutex				tim_lock;
};

/**----------------------------------------------------------------------------------*/
#if DIRECT_CREATE_SYSFS
static struct kobject * sn8p27113aa_kobject;

struct d_attr {
    struct attribute attr;
    int value; /* This is our data */
};

static struct d_attr data_show = {
    .attr.name = "read_loops",
    .attr.mode = 0644,
    .value = 0,
};

static struct d_attr data_write = {
    .attr.name = "change_ticks",
    .attr.mode = 0644,
    .value = 0,
};

static struct attribute * attrs[] = {
    &data_show.attr,
    &data_write.attr,
    NULL
};
static ssize_t sn8p27113aa_status_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct d_attr *da = container_of(attr, struct d_attr, attr);
    usr_msg( "show called (%s), loop_counter = %d\n", da->attr.name, gpio_get_value(btn->mcu_p53_gpio););
	da->value = loop_counter;
    return scnprintf(buf, PAGE_SIZE, "%s: %d\n", da->attr.name, da->value);
}

static ssize_t sn8p27113aa_poweroff_set(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    struct d_attr *da = container_of(attr, struct d_attr, attr);
	
    sscanf(buf, "%d", &da->value);
	if(da->value <= 0 || da->value >= 65535)
		return sizeof(int);
	ticks = da->value;
    usr_msg("timer ticks has been setted to %d\n", da->value);
    return sizeof(int);
}

static struct sysfs_ops ops = {
    .show  = sn8p27113aa_status_show,
    .store = sn8p27113aa_poweroff_set,
};

static struct kobj_type k_type = {
    .sysfs_ops     = &ops,
    .default_attrs = attrs,
};
	
static int sn8p27113aa_create_sysfs(struct device * dev)
{
	int ret;
	usr_msg("moved in function: %s", __func__);
	sn8p27113aa_kobject = kzalloc(sizeof(struct kobject), GFP_KERNEL);
	if(IS_ERR(sn8p27113aa_kobject)) {
		err_msg("error : foo_kobject kzalloc");
		return -ENOMEM;
	}
	// kobject_init(struct kobject * kobj, struct kobj_type * ktype)
	kobject_init(sn8p27113aa_kobject, &k_type);
     // parent is NULL, create folder under /sys/ direction
    ret = kobject_add(sn8p27113aa_kobject, NULL, "%s", MCU_DEV_NAME);
	if(0 != ret ) {
		ret = -1;
		err_msg("error: kobject_add() failed\n");
		kobject_put(sn8p27113aa_kobject);
		sn8p27113aa_kobject = NULL;
		goto out;
	}
	return ret;
out:
	return ret;
}

/**
 * @brief delete created sysfs, when driver removed
 * 
 * @param dev 
 */
static void sn8p27113aa_remove_sysfs(struct device * dev)
{
	usr_msg("sn8p27113aa remove sysfs");
	kobject_put(sn8p27113aa_kobject);
	sn8p27113aa_kobject = NULL;
}

#endif /** end of #if DIRECT_CREATE_SYSFS */
/**----------------------------------------------------------------------------------*/

static int sn8p27113aa_set_value(int gpio, gpio_dir dir, gpio_status value)
{
	int ret;
	
	if(in == dir) {
		ret = gpio_direction_input(gpio);
	} else {
		ret = gpio_direction_output(gpio, value);
	}
	return ret;
}

#if 0
static int sn8p27113aa_normal_poweroff_loop(struct sn8p27113aa_info * info)
{
	usr_msg("moved in function: %s, ready to run power off loop", __func__);
	sn8p27113aa_set_value(info->mcu_p42_gpio, out, low);
	return 0;
}
#endif

#if 1 // close for clean build warning
static int sn8p27113aa_force_poweroff_loop(struct sn8p27113aa_info * info)
{
	usr_msg("moved in function: %s, ready to run power off loop", __func__);
	sn8p27113aa_set_value(info->mcu_p42_gpio, out, low);
	usr_msg("in function: %s, step 1", __func__);
	mdelay(100);
	sn8p27113aa_set_value(info->mcu_p42_gpio, out, high);
	usr_msg("in function: %s, step 2", __func__);
	mdelay(50);
	sn8p27113aa_set_value(info->mcu_p42_gpio, out, low);
	usr_msg("in function: %s, step 3", __func__);
	mdelay(100);
	sn8p27113aa_set_value(info->mcu_p42_gpio, out, high);
	usr_msg("in function: %s, step 4", __func__);
	return 0;
}
#endif

static int sn8p27113aa_chip_init(struct sn8p27113aa_info * info)
{
	int ret;
	ret = sn8p27113aa_set_value(info->mcu_p42_gpio, out, high);
	ret = ret || sn8p27113aa_set_value(info->mcu_p53_gpio, in, low);	
	if(ret < 0) {
		usr_msg("gpio: [%d], gpio: [%d] set direction failed.", info->mcu_p42_gpio, info->mcu_p53_gpio);
		return ret; 
	}
	info->key_repeat = 0;
	usr_msg("gpio set pass, current mcu_p53_gpio = %d", gpio_get_value(info->mcu_p53_gpio));
	return 0;
}

static void data_handler(struct work_struct * work)
{
	int val;
	struct sn8p27113aa_info * btn = container_of(work, struct sn8p27113aa_info, work);
	if(IS_ERR(btn)) {
		usr_msg("error: get struct usr_keys_button in function: %s", __func__);
		return ;
	}
	usr_msg("moved in function: %s", __func__);
	if(0 == trigger_flag) {
		usr_msg("handler button half schedule");
		return ;
	} else {
		mutex_lock(&btn->lock);
		val = gpio_get_value(btn->mcu_p53_gpio);
		//if(key_val) {
			//usr_msg("in function: %s, info->mcu_p42_gpio = %d\n", __func__ ,btn->mcu_p42_gpio);
		
			//usr_msg("release");
			//input_report_key(btn->inputdev, KEY_VAL, 1);
			//input_sync(btn->inputdev);
			//#if 0
			//usr_msg("in function: %s, current system_state = %s", __func__, (system_state == SYSTEM_POWER_OFF) ? "SYSTEM_POWER_OFF" : "NOT IN SYSTEM_POWER_OFF");
			//while(system_state != SYSTEM_POWER_OFF) {
			//	usr_msg("wait flag SYSTEM_POWER_OFF");
			//	mdelay(20);
			//}
			//usr_msg("system power off");
			//#endif
			//sn8p27113aa_force_poweroff_loop(btn);
		
		//} else {
	        usr_msg("pressed, report KEY_CODE = %d, KEY_VALUE = %d", KEY_VAL, val);
	        input_report_key(btn->inputdev, KEY_VAL, val);
			input_sync(btn->inputdev);
			sn8p27113aa_force_poweroff_loop(btn);
		//}
		trigger_flag = 0;
		mutex_unlock(&btn->lock);
	}
	return ;
}

static irqreturn_t mcu_trigger_handler(int irq, void *arg)
{
	struct sn8p27113aa_info * info = (struct sn8p27113aa_info *)arg;
	if(!info) {
		usr_msg("error: sn8p27113aa_info in function: %s", __func__);
		return IRQ_HANDLED;
	}
	usr_msg("triggred");
    trigger_flag += 1;
	schedule_work(&info->work);
    return IRQ_HANDLED; 
}



#if HRT_TIMER
static enum hrtimer_restart usr_hrtimer_callback(struct hrtimer * arg)
{
	ktime_t now;
	struct sn8p27113aa_info * btn = container_of(arg, struct sn8p27113aa_info, tim);
	if(!btn) {
		usr_msg("error: get struct timer_structure in function: %s", __func__);
	}	
	usr_msg("tick count, reset chip");
	mutex_lock(&btn->tim_lock);
    now = arg->base->get_time();
	/** do something -------------------------------------------------*/
	usr_msg("moved in function: %s", __func__);
    hrtimer_forward(arg, now, btn->tim_period);
	mutex_unlock(&btn->tim_lock);
    return HRTIMER_RESTART;
}

static void hrt_timer_init(struct sn8p27113aa_info	* btn, s64 seconds, unsigned long mseconds)
{
    btn->tim_period = ktime_set(seconds, MS_TO_NS(mseconds));    
    hrtimer_init(&btn->tim, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    btn->tim.function = usr_hrtimer_callback;
    hrtimer_start(&btn->tim, btn->tim_period, HRTIMER_MODE_REL);
	mutex_init(&btn->tim_lock);
}

static void hrt_timer_del(struct sn8p27113aa_info	* btn)
{
	while(hrtimer_try_to_cancel(&btn->tim));
	hrtimer_cancel(&btn->tim);
}
#endif	// end of #if HRT_TIMER

static int usr_get_dts_info(struct sn8p27113aa_info * info, struct device *dev)
{
	int ret;
	struct device_node *node = NULL;
	node = of_find_compatible_node(NULL, NULL, "rockchip,r100u-mcu-key");
	if (!node) {
		usr_msg("cannot find node name rockchip,r100u-mcu-key");
		return -ENODEV;
	}
	info->mcu_p42_gpio = of_get_named_gpio(node, "mcu-p42-gpio", 0);
	if(!gpio_is_valid(info->mcu_p42_gpio)) {
		usr_msg("error: get mcu_p42_gpio gpio info. ERROR code %d", info->mcu_p42_gpio);
		return -EIO;
	}
    info->mcu_p53_gpio = of_get_named_gpio(node, "mcu-p53-gpio", 0);
    if(!gpio_is_valid(info->mcu_p53_gpio)) {
        usr_msg("error: get mcu_p53_gpio gpio info.  ERROR code %d", info->mcu_p42_gpio);
        return -EIO;
    }
	ret = devm_gpio_request_one(dev, info->mcu_p42_gpio, GPIOF_OUT_INIT_HIGH, "mcu-p42-gpio");
	if(ret < 0) {
		usr_msg("error: gpio mcu-p42-gpio request.");
		return -EIO;
	}
	ret = devm_gpio_request_one(dev, info->mcu_p53_gpio, GPIOF_IN, "mcu-p53-gpio");
    if(ret < 0) {
		usr_msg("error: mcu-p53-gpio request.");
		ret = -EIO;
		goto out1;
	}
	usr_msg("requested mcu_p42_gpio number: %d, mcu_p53_gpio number: %d", info->mcu_p42_gpio, info->mcu_p53_gpio);
	ret = sn8p27113aa_chip_init(info);
	if(ret < 0)
		goto out2;
	
	return 0;

out2:
	devm_gpio_free(dev, info->mcu_p53_gpio);
out1:
	devm_gpio_free(dev, info->mcu_p42_gpio);
	return ret;
}

static int sn8p27113aa_probe(struct platform_device *pdev)
{
	int ret;

	struct sn8p27113aa_info * info = devm_kmalloc(&pdev->dev, sizeof(struct sn8p27113aa_info), GFP_KERNEL);
	if(IS_ERR(info)) {
		usr_msg("error: devm_kmalloc");
		return -ENOMEM;
	}
	usr_msg("moved in function: %s", __func__);
	mutex_init(&info->lock);

	platform_set_drvdata(pdev, info);
	dev_set_drvdata(&pdev->dev, info);
	

	info->inputdev = devm_input_allocate_device(&pdev->dev);
	if(IS_ERR(info->inputdev)){
		usr_msg("error: input_allocate_device failed!");
		return -ENODEV;
	}

	info->inputdev->name 		= KEY_NAME;				
	info->inputdev->phys 		= KEY_NAME;	
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

	
	trigger_flag = 0;
	
	info->wq = create_singlethread_workqueue("sn8p27113aa_data_handler");
	if (!info->wq) {
		usr_msg("error: can not create workqueue");
		goto error2;
	}
	flush_workqueue(info->wq);
	INIT_WORK(&info->work, data_handler);

	info->irq = gpio_to_irq(info->mcu_p53_gpio);
	ret = request_irq(info->irq, mcu_trigger_handler, IRQF_TRIGGER_RISING, "sn8p27113aa_irq", (void *)info);
	if(ret < 0) {
		usr_msg("error: request irq");
		goto error3;
	}
	usr_msg("requested irq number = %d", info->irq);
#if HRT_TIMER
	hrt_timer_init(info, 0, 1000);
#endif
	platform_set_drvdata(pdev, (void *) info);
	dev_set_drvdata(&pdev->dev, (void *) info);
	return 0;


error3:
	destroy_workqueue(info->wq);

error2:
	input_put_device(info->inputdev);
	input_free_device(info->inputdev);
error1:

   return ret;

}

static int sn8p27113aa_remove(struct platform_device *pdev)
{
	struct sn8p27113aa_info *btn = platform_get_drvdata(pdev);
	
	cancel_work_sync(&btn->work);
	flush_workqueue(btn->wq);
	destroy_workqueue(btn->wq);
#if HRT_TIMER
	hrt_timer_del(btn);
#endif
	input_unregister_device(btn->inputdev);
	
	return 0;
}

#if 0
#ifdef CONFIG_PM	
static int sn8p27113aa_suspend(struct device *dev)
{
	struct sn8p27113aa_info *btn = dev_get_drvdata(dev);
	usr_msg("ready to suspend");
	disable_irq_nosync(btn->irq);
#if HRT_TIMER
	hrt_timer_del(btn);
#endif
	cancel_work_sync(&btn->work);
	flush_workqueue(btn->wq);
	gpio_set_value(btn->touch_ic_rst, 0);
	usr_msg("customer key suspended.");
	return 0;
}

static int sn8p27113aa_resume(struct device *dev)
{
	struct sn8p27113aa_info *btn = dev_get_drvdata(dev);
	usr_msg("ready to resume");
	chip_cp2610_init(btn);
#if HRT_TIMER
	hrt_timer_init(btn, 10, 0);
#endif
	enable_irq(btn->irq);
	usr_msg("customer key resumed operation.");
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(sn8p27113aa_pm_ops, sn8p27113aa_suspend, sn8p27113aa_resume);
#endif

static struct platform_driver sn8p27113aa_driver = {
	.probe		= sn8p27113aa_probe,
	.remove		= sn8p27113aa_remove,
	.driver		= {
		.name	= MCU_DEV_NAME,
		.owner	= THIS_MODULE,
		// .pm	= &sn8p27113aa_pm_ops,
		.of_match_table = of_match_ptr(mcu_key_match_table),
	},
};


static int __init sn8p27113aa_init(void)
{	
	usr_msg("moved in function: %s\n", __func__);
    return platform_driver_register(&sn8p27113aa_driver);
}
 
static void __exit sn8p27113aa_exit(void)
{
	usr_msg("moved in function: %s\n", __func__);
	platform_driver_unregister(&sn8p27113aa_driver);
}

module_init(sn8p27113aa_init);
module_exit(sn8p27113aa_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("sn8p27113aa mcu control power loop");
MODULE_ALIAS("sn8p27113aa");

