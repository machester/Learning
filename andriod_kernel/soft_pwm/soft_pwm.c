
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>                       // ioctl and file_operations support
#include <linux/gpio.h>                     /* For Legacy integer based GPIO */
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/major.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinctrl-state.h>
#include <linux/platform_device.h>          /* For platform devices */
#include <linux/printk.h>
#include <linux/mutex.h>
#include <linux/time.h>				// for get system current time
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fb.h>
#include <linux/err.h>
#include <linux/timer.h>        //for timer_list, jiffy timer, standard timer
#include <linux/kthread.h>
#include <linux/bcd.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>

#include <linux/regulator/consumer.h>


static int debug_status = 1;

#define TAG "<SOFT_PWM> "
#define SET_LEVEL KERN_NOTICE
#define dbg_msg(fmt, args...)                          \
    do {                                               \
        if (debug_status)                              \
            printk(SET_LEVEL TAG fmt "\n", ##args);    \
    } while (0)

#define err_msg(fmt, args...)   printk(KERN_ERR TAG "ERROR: " fmt "\n", ##args)


#define SOFT_PWM_DEV_NAME 				"soft_pwm"
#define SOFT_PWM_DRIVER_COMPATIBALE		"mediatek,soft_pwm"

#define MS_TO_NS(x) (x * 1000000)      // ms to ns

#define DEFAULT_PWM_FREQ				((unsigned int)1000)	// 1KHz
#define DEFAULT_PWM_DUTY_CYCLE			((unsigned int)50)	// duty 50%

#define BLANK		1
#define UNBLANK		0



typedef enum {
	low = 0,
	high = !low
} gpio_mode;

struct Sotf_PWM_Info {
	unsigned int system_led_gpio;
	unsigned int pulse;
	unsigned int freq;
	unsigned int status;

	int old_screen_status;
	struct notifier_block 	bl_notifier;

	struct timer_list 		pulse_mod_timer;
	struct hrtimer 			hrtimer_ticks;
	struct task_struct		*soft_timer_struct;
	struct platform_device 	*pdev_bk;

	// timer
	struct ktime_t hr_tim_period;


	struct mutex lock;
};

// struct Sotf_PWM_Info *soft_pwm_info;


static inline struct Sotf_PWM_Info *to_timer(struct hrtimer *tim)
{
	return container_of(tim, struct Sotf_PWM_Info, tim);
}

static inline struct Sotf_PWM_Info *to_pdev(struct platform_device *pdev)
{
	return container_of(platform_device, struct Sotf_PWM_Info, pdev_bk);
}

static ssize_t soft_pwm_freq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dbg_msg("[soft_pwm_freq_show] soft_pwm_freq_show = %d\n", soft_pwm_freq);
	return sprintf(buf, "%u\n", soft_pwm_freq);
}

static ssize_t soft_pwm_freq_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int ret = 0;
	char *pvalue = NULL;
	unsigned int val = 0;
	/*soft_pwm_freq_store*/
	printk("[soft_pwm_freq_store]\n");
	if (buf != NULL && size != 0) {
		pvalue = (char *)buf;
		ret = kstrtou32(pvalue, 10, (unsigned int *)&val);
		if (3 == val) {	// set usb status to default status
			dbg_msg("set usb status to default status");
		} else
			soft_pwm_freq = val;
	}
	dbg_msg("[soft_pwm_freq_store] soft_pwm_freq = %d\n", soft_pwm_freq);

	return size;
}

static ssize_t soft_pwm_pulse_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dbg_msg("[soft_pwm_pulse_show] soft_pwm_pulse_show = %d\n", soft_pwm_pulse);
	return sprintf(buf, "%u\n", soft_pwm_pulse);
}

static ssize_t soft_pwm_pulse_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int ret = 0;
	char *pvalue = NULL;
	unsigned int val = 0;
	/*soft_pwm_pulse_store*/
	dbg_msg("[soft_pwm_pulse_store]\n");

	if (buf != NULL && size != 0) {
		dbg_msg("[soft_pwm_pulse_store] buf is %s and size is %zu\n", buf, size);
		/*val = simple_strtoul(buf, &pvalue, 16);*/
		pvalue = (char *)buf;
		ret = kstrtou32(pvalue, 10, (unsigned int *)&val);
		if (val == 1) {
			gpio_set_value(usb_ctrl_gpio.gpio_usb_upgrade_mode, 1);
			gpio_set_value(usb_ctrl_gpio.gpio_exit_12, 0);
		}
		soft_pwm_pulse = val;
		dbg_msg("[soft_pwm_pulse_store] soft_pwm_pulse = %d\n", soft_pwm_pulse);
	}
	return size;
}

static ssize_t soft_pwm_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{


	dbg_msg("[soft_pwm_pulse_show] soft_pwm_pulse_show = %d\n", soft_pwm_pulse);
	return sprintf(buf, "%u\n", soft_pwm_pulse);
}

static ssize_t soft_pwm_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int ret = 0;
	char *pvalue = NULL;
	unsigned int val = 0;
	/*soft_pwm_pulse_store*/
	dbg_msg("[soft_pwm_pulse_store]\n");

	if (buf != NULL && size != 0) {
		dbg_msg("[soft_pwm_pulse_store] buf is %s and size is %zu\n", buf, size);
		/*val = simple_strtoul(buf, &pvalue, 16);*/
		pvalue = (char *)buf;
		ret = kstrtou32(pvalue, 10, (unsigned int *)&val);
		if (val == 1) {
			gpio_set_value(usb_ctrl_gpio.gpio_usb_upgrade_mode, 1);
			gpio_set_value(usb_ctrl_gpio.gpio_exit_12, 0);
		}
		soft_pwm_pulse = val;
		dbg_msg("[soft_pwm_pulse_store] soft_pwm_pulse = %d\n", soft_pwm_pulse);
	}
	return size;
}

static DEVICE_ATTR (soft_pwm_freq, (S_IRUGO | S_IWUSR | S_IWGRP), soft_pwm_freq_show, soft_pwm_freq_store);
static DEVICE_ATTR (soft_pwm_pulse, (S_IRUGO | S_IWUSR | S_IWGRP), soft_pwm_pulse_show, soft_pwm_pulse_store);
static DEVICE_ATTR (soft_pwm_status, (S_IRUGO | S_IWUSR | S_IWGRP), soft_pwm_status_show, soft_pwm_status_store);

static struct attribute *soft_pwm_attributes[] = {

	&dev_attr_soft_pwm_freq.attr,
	&dev_attr_soft_pwm_pulse.attr,
	&dev_attr_soft_pwm_status.attr,
	NULL
};

static struct attribute_group sys_led_attribute_group = {
	.name = SOFT_PWM_DEV_NAME,
	.attrs = soft_pwm_attributes,
};



static int soft_pwm_create_fs(struct platform_device *pdev)
{
	int ret;
	ret = sysfs_create_group(&pdev->dev.kobj, &sys_led_attribute_group);
	if (ret) {
		dbg_msg("in function: %s, create sysfs failed", __func__);
		return -1;
	}

	return 0;
}

static int soft_pwm_remove_fs(struct platform_device *pdev)
{
	int ret;
	ret = sysfs_remove_group(&pdev->dev.kobj, &sys_led_attribute_group);
	if (ret) {
		dbg_msg("in function: %s, create sysfs failed", __func__);
		return -1;
	}
	return 0;
}


static void get_dts_gpio_info(struct Sotf_PWM_Info *soft_pwm_info)
{
	int ret;
	struct device_node *node = &pdev->dev->of_node;
	if (!node) {
		dbg_msg("in func: %s, cannot found dts node", __func__);
		return -EIO;
	}
	soft_pwm_info->system_led_gpio = of_get_named_gpio(node, "system_led_gpio", 0);
	if (!gpio_is_valid(soft_pwm_info->system_led_gpio)) {
		err_msg("not valid soft_pwm_info->system_led_gpio");
	} else {
		ret = gpio_request_one(soft_pwm_info->system_led_gpio, GPIOF_OUT_INIT_LOW, "system_led_gpio");
		if (ret < 0) {
			err_msg("quest failed soft_pwm_info->system_led_gpio");
			return -EIO;
		} else {
			dbg_msg("soft_pwm_info->system_led_gpio = %d", soft_pwm_info->system_led_gpio);
		}
	}
}

static void set_gpio_default_mode(void)
{
	dbg_msg("moved in fuc: % s", __func__);

	if (gpio_info.system_led > 0) {
		gpio_set_value(gpio_info.system_led, (gpio_mode)high);
		msleep(10);
		dbg_msg("gpio_info.system_led set to hight");

	}
}

static enum hrtimer_restart soft_pwm_freq_hrtimer_callback(struct hrtimer * arg)
{
	ktime_t now = arg->base->get_time();
	usr_msg("timer running at jiffies = % ld\n", jiffies);
	hrtimer_forward(arg, now, tim_period);
	return HRTIMER_RESTART;
}

static void pulse_timer_function(unsigned long data)
{
	struct Sotf_PWM_Info *soft_pwm_info = (void *)data;
	static unsigned long duty;
	static bool plus_invert = false;

	// dbg_msg("moved in func: % s, ticks = % d, plus_invert = % s\n", __func__, ticks, (plus_invert ? "true" : "false"));

	if (duty <= TOTAL_BREATH_PULS_WIDTH && !plus_invert) {
		duty += BREATH_BLINK_STEP;
	} else if (duty > 0 && plus_invert) {
		duty = duty - BREATH_BLINK_STEP;
	} else if (duty < 0) {
		duty = 0;
		plus_invert = false;
		// dbg_msg("moved in func: % s, duty = % d, plus_invert = % s\n", __func__, duty, (plus_invert ? "true" : "false"));
	} else { //change direction
		plus_invert = !plus_invert;
		// dbg_msg("moved in func: % s, duty = % d, plus_invert = % s\n", __func__, duty, (plus_invert ? "true" : "false"));
	}

	mod_timer(&breath_timer, jiffies + msecs_to_jiffies(100));

}


static int soft_pwm_create_timer(struct Sotf_PWM_Info *soft_pwm_info)
{
	// hrtimer
	unsigned int hr_tim_val = (1 * 1000) / (DEFAULT_PWM_FREQ);					 // unit: mseconds
	soft_pwm_info->hr_tim_period = ktime_set(0, MS_TO_NS(hr_tim_val));
	hrtimer_init(&soft_pwm_info->hrtimer_ticks , CLOCK_REALTIME, HRTIMER_MODE_REL);
	soft_pwm_info->hrtimer_ticks.function = soft_pwm_freq_hrtimer_callback;
	// hrtimer_start(&soft_pwm_info->hrtimer_ticks, soft_pwm_info->hr_tim_period, HRTIMER_MODE_REL);

	// standard timer
	setup_timer(&soft_pwm_info->pulse_mod_timer , pulse_timer_function, (unsigned long)soft_pwm_info);
	// mod_timer(&breath_timer, jiffies + msecs_to_jiffies(100));
}

static int soft_pwm_start_timer(struct Sotf_PWM_Info *soft_pwm_info)
{

}

static void soft_pwm_stop_timer(struct Sotf_PWM_Info *soft_pwm_info)
{

}

static int fb_notifier_callback(struct notifier_block *p,
                                unsigned long event, void *data)
{
	struct Sotf_PWM_Info *soft_pwm_info = container_of(p, struct Sotf_PWM_Info, bl_notifier);
	struct fb_event *fb_event = data;
	int *blank;
	int new_status;

	/* If we aren't interested in this event, skip it immediately ... */
	if (event != FB_EVENT_BLANK)
		return 0;

	blank = fb_event->data;
	new_status = *blank ? BLANK : UNBLANK;

	if (new_status == soft_pwm_info->old_screen_status)
		return 0;

	if (soft_pwm_info->old_screen_status == UNBLANK) {
		kthread_stop(soft_pwm_info->soft_timer_struct);
		
	} else {
		wake_up_process(soft_pwm_info->soft_timer_struct);
	}

	soft_pwm_info->old_screen_status = new_status;

	return 0;
}

static int soft_pwm_register_bl_notifi(struct Sotf_PWM_Info *soft_pwm_info)
{
	int ret;

	soft_pwm_info->old_screen_status = UNBLANK;
	soft_pwm_info->bl_notifier.notifier_call = fb_notifier_callback;

	ret = fb_register_client(&soft_pwm_info->bl_notifier);
	if (ret)
		err_msg("unable to register backlight trigger\n");
}

static int soft_pwm_thread_entry(void *data)
{
	uint16_t loop_addr_index;
	uint16_t value;

	struct Sotf_PWM_Info *soft_pwm_info = (struct Sotf_PWM_Info *)data;
	if (IS_ERR(soft_pwm_info)) {
		err_msg("in function: % s, get struct struct Sotf_PWM_Info failed", __func__);
		return PTR_ERR_OR_ZERO(soft_pwm_info);
	}



	while (1) {
		if (kthread_should_stop()) {
			break;
		}
		else if (soft_pwm_info->ctrl_param.auto_report_data) {
			mutex_lock(&soft_pwm_info->lock);

			mutex_unlock(&soft_pwm_info->lock);
			schedule_timeout(jiffies +  msecs_to_jiffies(soft_pwm_info->ctrl_param.data_polling_interval));
		}
		else {
			schedule_timeout(jiffies +  msecs_to_jiffies(soft_pwm_info->ctrl_param.data_polling_interval));
		}
	}
	return 0;
}


static int soft_pwm_probe(struct platform_device *pdev)
{
	struct Sotf_PWM_Info *soft_pwm_info = kmalloc(sizeof(struct Sotf_PWM_Info), GFP_KERNEL);
	if (IS_ERR(soft_pwm_info)) {
		err_msg("error : usr_tim struct malloc");
		return PTR_ERR_OR_ZERO(soft_pwm_info);
	}
	dbg_msg("moved in fuc: % s", __func__);

	soft_pwm_info->pdev_bk = pdev;

	get_dts_gpio_info(soft_pwm_info);


	set_gpio_default_mode();

	soft_pwm_create_fs(pdev);

	soft_pwm_info->soft_timer_struct = kthread_create(soft_pwm_thread_entry, (void *)soft_pwm_info, SOFT_PWM_DEV_NAME);
	


	return 0;
}

static int soft_pwm_remove(struct platform_device *pdev)
{


	int retval;
	retval = hrtimer_cancel(&soft_pwm_info->hrtimer_ticks);
	if (retval) {
		dbg_msg("The timer is still in use...\n");
	}


	sysfs_remove_group(&pdev->dev.kobj, &sys_led_attribute_group);
	return 0;
}

static const struct of_device_id soft_pwm_match_table[] = {
	{.compatible = SOFT_PWM_DRIVER_COMPATIBALE},
	{},
};

static struct platform_driver soft_pwm = {
	.probe      = soft_pwm_probe,
	.remove     = soft_pwm_remove,
	.driver = {
		.name = SOFT_PWM_DEV_NAME,
		.of_match_table = of_match_ptr(soft_pwm_match_table),
	},
};

module_platform_driver(soft_pwm);

MODULE_DESCRIPTION("software simulate pwm driver");
MODULE_AUTHOR("jiangq < jiang.quan@elinktek.com > ");
MODULE_LICENSE("GPL v2");
