/**********************************************************************
* dts 

	usr_gpio_ctrl: usr-gpio-ctrl {
		compatible = "rockchip,usr-gpio-ctrl";
		status = "okay";
		
		dc_out1-gpio = <&gpio7 RK_PA4 GPIO_ACTIVE_HIGH>;
		dc_out2-gpio = <&gpio0 RK_PA7 GPIO_ACTIVE_HIGH>;
		beep-ctrl    = <&gpio5 RK_PC1 GPIO_ACTIVE_HIGH>;
		power-led    = <&gpio0 RK_PA1 GPIO_ACTIVE_HIGH>;
		uart-pwr     = <&gpio8 RK_PA1 GPIO_ACTIVE_HIGH>;
		usb-pwr      = <&gpio7 RK_PB7 GPIO_ACTIVE_HIGH>;

	};


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
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/sysfs.h>
#include <linux/device.h>

#include <linux/kthread.h>
#include <linux/err.h>

#include <linux/kernel.h>
#include <linux/ctype.h>

#define USING_SYS_FS					1

static int debug_status = 1;


#define TAG								" <USR_GPIO_CTRL> "
#define SET_LEVEL						KERN_INFO
#define err_msg(fmt, args...)			printk(KERN_ERR TAG "error " fmt"\n", ##args)
#define usr_msg(fmt, args...)			do {                                                \
                                            if(debug_status)                                \
                                                printk(SET_LEVEL TAG fmt"\n", ##args);      \
                                        } while(0)

#define USR_GPIO_CTRL_NAME						"r101-vol-ouput-ctrl"	


#if USING_SYS_FS
	static int pwr_en_ctrl_status;
	static int beep_en_ctrl_status;
	static int uart_en_ctrl_status;
#endif




typedef enum {
	low = 0,
	high = 1
}gpio_status;


static const struct of_device_id usr_gpio_ctrl_match_table[] = {
	{ .compatible = "rockchip,usr-gpio-ctrl", },
	{/* KEEP THIS */},
};
MODULE_DEVICE_TABLE(of, usr_gpio_ctrl_match_table);

/*------------------------------------------------------------------------------------*/

struct usr_ctrl_gpio {
	int dc_12v_out1;
	int dc_12v_out2;
	int beep_gpio;
	int power_led;
	int uart_pwr;
	int usb_pwr;
};

struct pwr_info {
	struct usr_ctrl_gpio	gpio;
	struct platform_device 	* pdev_bk;
	struct task_struct * beep_alarm_task;
};

struct pwr_info * info;
struct mutex 	lock;

/**----------------------------------------------------------------------------------*/
extern bool beep_alarm_flag;		// in gt9xx.c file

static int beep_alarm(void * data)
{
	struct pwr_info * info = (struct pwr_info *)data;
	if(!info) {
		usr_msg("in function: %s, get struct pwr_info failed", __func__);
		return -ENOMEM;
	}
	usr_msg("in func;%s, beep_en_ctrl_status = %d", __func__, beep_en_ctrl_status);

	while(1) {
		if(kthread_should_stop()) {
			break;
		} else if(beep_en_ctrl_status){
				if(beep_alarm_flag) {
					gpio_set_value(info->gpio.beep_gpio, 1);
					mdelay(20);
					gpio_set_value(info->gpio.beep_gpio, 0);
					beep_alarm_flag = false;
				} else 
					schedule_timeout(1000 * HZ);
		} else
			schedule_timeout(1000 * HZ);
	}
	return 0;
}

static int create_beep_alarm_thread(struct pwr_info * info)
{	
	info->beep_alarm_task = kthread_create(beep_alarm, (void *)info, "beep_alarm");
	if(IS_ERR(info->beep_alarm_task)) {
		usr_msg("failed: create beep_alarm_task");
		return -1;
	}
	wake_up_process(info->beep_alarm_task); 
	return 0;
}


static int usr_get_dts_info(struct pwr_info * info)
{
	int ret;
	struct device_node *node = info->pdev_bk->dev.of_node;
	if(!node) {
		err_msg("in func: %s, cannot find node", __func__);
		return -ENXIO;
	}
	
	info->gpio.beep_gpio = of_get_named_gpio(node, "beep-ctrl", 0);
	if(!gpio_is_valid(info->gpio.beep_gpio)) {
		err_msg("get gpio.beep_gpio.");
	} else {
		ret = gpio_request_one(info->gpio.beep_gpio, GPIOF_OUT_INIT_LOW, "beep_gpio");
		if(ret < 0) {
			err_msg("beep_gpio request.");
		}
		gpio_set_value(info->gpio.beep_gpio, 0);
	}
	
	info->gpio.power_led = of_get_named_gpio(node, "power-led", 0);
	if(!gpio_is_valid(info->gpio.power_led)) {
		err_msg("get: info->gpio.power_led.");
	} else {
		ret = gpio_request_one(info->gpio.power_led, GPIOF_OUT_INIT_LOW, "power_led");
		if(ret < 0) {
			err_msg("request: info->gpio.power_led");
		}
		gpio_set_value(info->gpio.power_led, 0);
	}
	
	
	info->gpio.dc_12v_out1 = of_get_named_gpio(node, "dc_out1-gpio", 0);
	if(!gpio_is_valid(info->gpio.dc_12v_out1)) {
		err_msg("get: gpio.dc_12v_out1.");
	} else {
		ret = gpio_request_one(info->gpio.dc_12v_out1, GPIOF_OUT_INIT_LOW, "dc_out1");
		if(ret < 0) {
			err_msg("request: info->gpio.dc_12v_out1.");
		}
		gpio_set_value(info->gpio.dc_12v_out1, 0);
	}

	info->gpio.dc_12v_out2 = of_get_named_gpio(node, "dc_out2-gpio", 0);
	if(!gpio_is_valid(info->gpio.dc_12v_out2)) {
		err_msg("get: info->gpio.dc_12v_out2.");
	} else {
		ret = gpio_request_one(info->gpio.dc_12v_out2, GPIOF_OUT_INIT_LOW, "dc_out2");
		if(ret < 0) {
			err_msg("request: info->gpio.dc_12v_out2.");
		}
		gpio_set_value(info->gpio.dc_12v_out1, 0);
	}

	info->gpio.uart_pwr = of_get_named_gpio(node, "uart-pwr", 0);
	if(!gpio_is_valid(info->gpio.uart_pwr)) {
		err_msg("get: info->gpio.uart_pwr.");
	} else {
		ret = gpio_request_one(info->gpio.uart_pwr, GPIOF_OUT_INIT_LOW, "uart_pwr");
		if(ret < 0) {
			err_msg("request: info->gpio.uart_pwr.");
		}
		gpio_set_value(info->gpio.uart_pwr, 0);
	}

	info->gpio.usb_pwr = of_get_named_gpio(node, "usb-pwr", 0);
	if(!gpio_is_valid(info->gpio.usb_pwr)) {
		err_msg("get: info->gpio.usb_pwr.");
	} else {
		ret = gpio_request_one(info->gpio.usb_pwr, GPIOF_OUT_INIT_LOW, "usb_pwr");
		if(ret < 0) {
			err_msg("request: info->gpio.usb_pwr.");
		}
		gpio_set_value(info->gpio.usb_pwr, 0);
	}

	return 0;
}

static void set_ctrl_gpio_defaul_status(struct pwr_info * info)
{
	if(info->gpio.dc_12v_out1 > 0)
		gpio_set_value(info->gpio.dc_12v_out1, 0);
	if(info->gpio.dc_12v_out2 > 0)
		gpio_set_value(info->gpio.dc_12v_out2, 0);
	if(info->gpio.beep_gpio > 0)
		gpio_set_value(info->gpio.beep_gpio, 1);
	mdelay(100);
	if(info->gpio.beep_gpio > 0)
		gpio_set_value(info->gpio.beep_gpio, 0);

	if(info->gpio.power_led > 0)
		gpio_set_value(info->gpio.power_led, 0);
	if(info->gpio.uart_pwr > 0)
		gpio_set_value(info->gpio.uart_pwr, 0);
	if(info->gpio.usb_pwr > 0)
		gpio_set_value(info->gpio.usb_pwr, 1);
}


#if USING_SYS_FS

static ssize_t pwr_output_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{

	struct pwr_info * info = dev_get_drvdata(dev);
	if(!info) {
		err_msg("in func: %s, get struct pwr_info failed", __func__);
		return size;
	}

	usr_msg("in func: %s, size = %d", __func__, size);
	if(size > 0) {
		usr_msg("in func: %s, buf[0] = %c", __func__, buf[0]);
		if('0' == buf[0]) {
			usr_msg("in func: %s, set 12v output to low", __func__);
			if(info->gpio.dc_12v_out1 > 0)
				gpio_set_value(info->gpio.dc_12v_out1, 0);
			if(info->gpio.dc_12v_out2 > 0)
				gpio_set_value(info->gpio.dc_12v_out2, 0);
			pwr_en_ctrl_status = 0;
		} else {
			usr_msg("in func: %s, set 12v output to high", __func__);
			if(info->gpio.dc_12v_out1 > 0)
				gpio_set_value(info->gpio.dc_12v_out1, 1);
			if(info->gpio.dc_12v_out2 > 0)
				gpio_set_value(info->gpio.dc_12v_out2, 1);
			pwr_en_ctrl_status = 1;
		}
	}
	return size;

}
static ssize_t pwr_output_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	usr_msg("moved in func: %s", __func__);

	return sprintf(buf, "%d\n", pwr_en_ctrl_status);
}

static ssize_t beep_ctrl_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{

	struct pwr_info * info = dev_get_drvdata(dev);
	if(!info) {
		err_msg("in func: %s, get struct pwr_info failed", __func__);
		return size;
	}

	usr_msg("in func: %s, size = %d", __func__, size);
	if(size > 0) {
		usr_msg("in func: %s, buf[0] = %c", __func__, buf[0]);
		if('0' == buf[0]) {
			usr_msg("in func: %s, set beep_crtl off", __func__);
			beep_en_ctrl_status = 0;
		} else {
			usr_msg("in func: %s, set beep_ctrl on", __func__);
			beep_en_ctrl_status = 1;
		}
	}
	return size;
}
static ssize_t beep_ctrl_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	usr_msg("moved in func: %s", __func__);

	return sprintf(buf, "%d\n", beep_en_ctrl_status);
}

static ssize_t uart_power_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{

	struct pwr_info * info = dev_get_drvdata(dev);
	if(!info) {
		err_msg("in func: %s, get struct pwr_info failed", __func__);
		return size;
	}

	usr_msg("in func: %s, size = %d", __func__, size);
	if(size > 0) {
		usr_msg("in func: %s, buf[0] = %c", __func__, buf[0]);
		if('0' == buf[0]) {
			usr_msg("in func: %s, set uart_power off", __func__);
			uart_en_ctrl_status = 0;
		} else {
			usr_msg("in func: %s, set uart_power on", __func__);
			uart_en_ctrl_status = 1;
		}
	}
	return size;
}

static ssize_t uart_power_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	usr_msg("moved in func: %s", __func__);

	return sprintf(buf, "%d\n", uart_en_ctrl_status);
}

static DEVICE_ATTR(beep_ctrl,	   0664, beep_ctrl_show, beep_ctrl_sotre);
static DEVICE_ATTR(pwr_output,     0664, pwr_output_show, pwr_output_sotre);
static DEVICE_ATTR(uart_power,     0664, uart_power_show, uart_power_sotre);

static struct attribute *usr_gpio_ctrl_attributes[] = {
    &dev_attr_pwr_output.attr,	//name dev_attr_name.attr
    &dev_attr_beep_ctrl.attr,
	&dev_attr_uart_power.attr,
    NULL
};
	
static struct attribute_group usr_gpio_ctrl_attribute_group = {
	.attrs	= usr_gpio_ctrl_attributes,
};

static int pwr_ctrl_sysfs_create(struct platform_device * pdev)
{
	int rc;
	
	rc = sysfs_create_group(&pdev->dev.kobj, &usr_gpio_ctrl_attribute_group);
	if (rc) {
		usr_msg("in function: %s, create fs led_runing_reboot failed", __func__);
		return -1;
	}
	
	return 0;
}

static void pwr_ctrl_sysfs_remove(struct platform_device * pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_pwr_output);
}
#endif

static int usr_gpio_ctrl_probe(struct platform_device *pdev)
{
	int ret;

	struct pwr_info * info = devm_kmalloc(&pdev->dev, sizeof(struct pwr_info), GFP_KERNEL);
	if(IS_ERR(info)) {
		usr_msg("error: devm_kmalloc");
		return -ENOMEM;
	}
	usr_msg("moved in function: %s", __func__);
	mutex_init(&lock);

	info->pdev_bk = pdev;
	
	ret = usr_get_dts_info(info);
	if(ret < 0) {
		usr_msg("error: get gpio dts info");
		goto error1;
	}
	set_ctrl_gpio_defaul_status(info);

	dev_set_drvdata(&pdev->dev, (void *) info);
	
#if USING_SYS_FS
	ret = pwr_ctrl_sysfs_create(pdev);
	if(ret < 0) {
		usr_msg("create sysfs failed.");
		goto error1;
	}
#endif

	create_beep_alarm_thread(info);
	return 0;

error1:

   return ret;

}

static int usr_gpio_ctrl_remove(struct platform_device *pdev)
{
	struct pwr_info *info = platform_get_drvdata(pdev);
	
	gpio_free(info->gpio.dc_12v_out1);
	gpio_free(info->gpio.dc_12v_out2);
	
#if USING_SYS_FS
	pwr_ctrl_sysfs_remove(pdev);
#endif

	return 0;
}


#ifdef CONFIG_PM	
static int reboot_key_suspend(struct device *dev)
{
	/*struct reboot_key_info *btn = dev_get_drvdata(dev);
	usr_msg("ready to suspend");
	disable_irq_nosync(btn->irq);

	cancel_work_sync(&btn->work);
	flush_workqueue(btn->wq);
	gpio_set_value(btn->touch_ic_rst, 0);
	usr_msg("customer key suspended.");*/
	printk(KERN_INFO "zjk standby_led suspend\n");
	gpio_direction_output(info->gpio.power_led, 1);
	return 0;
}

static int reboot_key_resume(struct device *dev)
{
	/*struct reboot_key_info *btn = dev_get_drvdata(dev);
	usr_msg("ready to resume");
	chip_cp2610_init(btn);

	enable_irq(btn->irq);
	usr_msg("customer key resumed operation.");*/
	printk(KERN_INFO "zjk standby_led resume\n");
	gpio_direction_output(info->gpio.power_led, 0);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(reboot_key_pm_ops, reboot_key_suspend, reboot_key_resume);


static struct platform_driver pwr_ctrl_driver = {
	.probe		= usr_gpio_ctrl_probe,
	.remove		= usr_gpio_ctrl_remove,
	.driver		= {
		.name	= USR_GPIO_CTRL_NAME,
		.owner	= THIS_MODULE,
		.pm	= &reboot_key_pm_ops,
		.of_match_table = of_match_ptr(usr_gpio_ctrl_match_table),
	},
};


module_platform_driver(pwr_ctrl_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("pwr_ctrl_driver control loop");
MODULE_ALIAS("pwr_ctrl_driver");
