/**********************************************************************
* dts 

output_vol_ctrl: output-vol-ctrl {
	compatible = "rockchip, output_vol_ctrl";
	status = "okay";

	dc_12v:dc-12v {
		dc_out1-gpios = <&gpio7 RK_PA4 GPIO_ACTIVE_HIGH>;
		dc_out1-delayms = <10>;
		dc_out2-gpios = <&gpio0 RK_PA7 GPIO_ACTIVE_HIGH>;
		dc_out2-delayms = <10>;
	};
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


#include <linux/kernel.h>
#include <linux/ctype.h>



static int debug_status = 1;


#define TAG								" <VOL_OUTPUT_CTRL> "
#define SET_LEVEL						KERN_INFO
#define err_msg(fmt, args...)			printk(KERN_ERR TAG "error " fmt"\n", ##args)
#define usr_msg(fmt, args...)			do {                                                \
                                            if(debug_status)                                \
                                                printk(SET_LEVEL TAG fmt"\n", ##args);      \
                                        } while(0)

#define PWR_CTRL_DEV_NAME						"r101-vol-ouput-ctrl"	

#define OUTPUT_12V_NODE_NAME			"dc_12v"



#define USING_SYS_FS					1

#if USING_SYS_FS
	static int pwr_en_ctrl_status;
	static int hdmi_en_ctrl_status;
#endif

typedef enum {
	low = 0,
	high = 1
}gpio_status;


static const struct of_device_id pwr_ctrl_match_table[] = {
	{ .compatible = "rockchip,output_vol_ctrl", },
	{/* KEEP THIS */},
};
MODULE_DEVICE_TABLE(of, pwr_ctrl_match_table);

/*------------------------------------------------------------------------------------*/

struct pwr_ctrl_gpio_info {
	int dc_12v_out1;
	int hdmi_pwr_ctrl;

};

struct pwr_info {
	struct pwr_ctrl_gpio_info	gpio;
	struct platform_device 		* pdev_bk;
};

struct mutex 	lock;

/**----------------------------------------------------------------------------------*/

static int usr_get_dts_info(struct pwr_info * info)
{
	int ret;
	struct device_node *node = info->pdev_bk->dev.of_node;
	if(!node) {
		err_msg("in func: %s, cannot find node", __func__);
		return -ENXIO;
	}
	
	info->gpio.dc_12v_out1 = of_get_named_gpio(node, "dc_out1-gpio", 0);
	if(!gpio_is_valid(info->gpio.dc_12v_out1)) {
		err_msg("get gpio.dc_12v_out1.");
	} else {
		ret = gpio_request_one(info->gpio.dc_12v_out1, GPIOF_OUT_INIT_LOW, "dc_out1-gpio");
		if(ret < 0) {
			err_msg("info->gpio.dc_12v_out1 request.");
		}
	}

	info->gpio.hdmi_pwr_ctrl = of_get_named_gpio(node, "hdmi-pwr-ctrl-gpio", 0);
	if(!gpio_is_valid(info->gpio.hdmi_pwr_ctrl)) {
		err_msg("get gpio.hdmi_pwr_ctrl");
	} else {
		ret = gpio_request_one(info->gpio.hdmi_pwr_ctrl, GPIOF_OUT_INIT_LOW, "hdmi_pwr_ctrl");
		if(ret < 0) {
			err_msg("gpio.hdmi_pwr_ctrl request.");
		}
	}

	return 0;
}

static void set_ctrl_gpio_defaul_status(struct pwr_info * info)
{
	if(info->gpio.dc_12v_out1 > 0)
		gpio_set_value(info->gpio.dc_12v_out1, 0);
	if(info->gpio.hdmi_pwr_ctrl > 0)
		gpio_set_value(info->gpio.hdmi_pwr_ctrl, 0);
}


#if USING_SYS_FS

static ssize_t dc_pwr_output_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
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
			pwr_en_ctrl_status = 0;
		} else {
			usr_msg("in func: %s, set 12v output to high", __func__);
			if(info->gpio.dc_12v_out1 > 0)
				gpio_set_value(info->gpio.dc_12v_out1, 1);
			pwr_en_ctrl_status = 1;
		}
	}
	return size;

}
static ssize_t dc_pwr_output_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	usr_msg("moved in func: %s", __func__);

	return sprintf(buf, "%d\n", hdmi_en_ctrl_status);
}

static ssize_t hdmi_pwr_output_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
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
			if(info->gpio.hdmi_pwr_ctrl > 0)
				gpio_set_value(info->gpio.hdmi_pwr_ctrl, 0);
			pwr_en_ctrl_status = 0;
		} else {
			usr_msg("in func: %s, set 12v output to high", __func__);
			if(info->gpio.hdmi_pwr_ctrl > 0)
				gpio_set_value(info->gpio.hdmi_pwr_ctrl, 1);
			hdmi_en_ctrl_status = 1;
		}
	}
	return size;

}
static ssize_t hdmi_pwr_output_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	usr_msg("moved in func: %s", __func__);

	return sprintf(buf, "%d\n", hdmi_en_ctrl_status);
}

static DEVICE_ATTR(dc_pwr_output,     0664, dc_pwr_output_show, dc_pwr_output_sotre);
static DEVICE_ATTR(hdmi_pwr_output,     0664, hdmi_pwr_output_show, hdmi_pwr_output_sotre);

static int pwr_ctrl_sysfs_create(struct platform_device * pdev)
{
	int rc;
	
	rc = device_create_file(&pdev->dev, &dev_attr_dc_pwr_output);
	if (rc) {
		usr_msg("in function: %s, create fs dc_pwr_output failed", __func__);
		return -1;
	}
	rc = device_create_file(&pdev->dev, &dev_attr_hdmi_pwr_output);
	if (rc) {
		usr_msg("in function: %s, create fs hdmi_pwr_output failed", __func__);
		return -1;
	}

	return 0;
}

static void pwr_ctrl_sysfs_remove(struct platform_device * pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_dc_pwr_output);
	device_remove_file(&pdev->dev, &dev_attr_hdmi_pwr_output);
}
#endif

static int pwr_ctrl_probe(struct platform_device *pdev)
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

	return 0;

error1:

   return ret;

}

static int pwr_ctrl_remove(struct platform_device *pdev)
{
	struct pwr_info *info = platform_get_drvdata(pdev);
	
	gpio_free(info->gpio.dc_12v_out1);
	gpio_free(info->gpio.hdmi_pwr_ctrl);
	
#if USING_SYS_FS
	pwr_ctrl_sysfs_remove(pdev);
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

static struct platform_driver pwr_ctrl_driver = {
	.probe		= pwr_ctrl_probe,
	.remove		= pwr_ctrl_remove,
	.driver		= {
		.name	= PWR_CTRL_DEV_NAME,
		.owner	= THIS_MODULE,
		// .pm	= &reboot_key_pm_ops,
		.of_match_table = of_match_ptr(pwr_ctrl_match_table),
	},
};


module_platform_driver(pwr_ctrl_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("pwr_ctrl_driver control loop");
MODULE_ALIAS("pwr_ctrl_driver");

