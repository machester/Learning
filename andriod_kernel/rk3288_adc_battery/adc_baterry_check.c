
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/of_device.h>

#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/gpio/consumer.h>


#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/iio/consumer.h>

#include <linux/workqueue.h>
#include <linux/kthread.h>

#include <linux/delay.h>

static int dbg_mark = 1;

#define BATERRY_DRIVER_NAME 	"adc-rk3288-baterry"
#define BATTERY_TAG				"<adc-battery> "

		
#define dbg_msg(fmt, args...)           do {                                                	\
                                             if(dbg_mark)                               		\
                                                 printk(KERN_INFO BATTERY_TAG fmt, ##args); 	\
                                        } while(0)


struct adc_rk3288_battery_data {
	int status;
	int health;
	int present;
	int capacity;

	struct power_supply 	*battery;
	struct power_supply 	*dc_pwr;

	struct iio_channel		*channel;
	int dc_detect_gpio;
	int irq_trigger_leve;
	int irq_num;

	u32 adc_report_interval;
	u32 vol_lower_limit;
	u32 vol_middle_limit;
	u32 vol_upper_limit;
	u32 vol_shutdown_limit;
	int current_adc_value;
	int compare_value;

	bool battery_full;
	bool battery_mid;
	bool battery_low;
	bool system_need_shutdown;

	struct task_struct 			*battery_vol_wrok;
	struct mutex				lock;

	bool battery_monitor_running;
	bool dc_input_detected;
			   
};

static enum power_supply_property adc_rk3288_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static bool is_dc_exist(struct adc_rk3288_battery_data *di)
{
	if (di->dc_detect_gpio) {		// gpio requested
		if(gpio_get_value(di->dc_detect_gpio)) {
			di->dc_input_detected = false;
			dbg_msg("di->dc_input_detected = %s\n", di->dc_input_detected ? "true" : "false");
			return false;
		}
		else {
			di->dc_input_detected = true;
			dbg_msg("di->dc_input_detected = %s\n", di->dc_input_detected ? "true" : "false");
			return true;
		}
	}
	return false;
}

#define ADC_COMPARE_TIMES			5

static int get_adc_value(struct adc_rk3288_battery_data *info)
{
	int ret, value;
	
	// dbg_msg("moved in func: %s\n", __func__);
	if(info->channel) {
		mutex_lock(&info->lock);
		ret = iio_read_channel_raw(info->channel, &value);
		mutex_unlock(&info->lock);
		if(ret < 0) {
			dbg_msg("in func: %s, read adc channel failed, ret = %d\n", __func__, ret);
			return ret;
		} else {
			info->current_adc_value = value;
			// dbg_msg("in func: %s, adc value = %d\n", __func__, value);
		}
	} else {
		dbg_msg("in func: %s, get adc channel failed\n", __func__);
		return -EIO;
	}
	return ret;
}

static void compare_system_status(struct adc_rk3288_battery_data *info)
{
	int loop_index;
	int divid_gap;
	
	// dbg_msg("moved in func: %s\n", __func__);
	
	for(loop_index = 0; loop_index < ADC_COMPARE_TIMES; loop_index++) {
		info->current_adc_value += get_adc_value(info);
	}
	info->compare_value = info->current_adc_value / ADC_COMPARE_TIMES;
	info->compare_value = (((u32)1800 * info->current_adc_value) / (u32)1023);
	// dbg_msg("compare value = %d\n", info->compare_value);
	mutex_lock(&info->lock);
	if(info->compare_value >= info->vol_upper_limit) {
		info->present = 100;
		info->capacity = 100;
		info->battery_full = true;
		info->battery_mid = false;
		info->battery_low = false;
		info->system_need_shutdown = false;	
		
	} else if ((info->compare_value < info->vol_upper_limit) && \
				(info->compare_value >= info->vol_middle_limit)) {
		divid_gap = ((info->vol_upper_limit - info->vol_middle_limit) / 50);
		
		info->present = (50 + ((info->compare_value - info->vol_middle_limit) / divid_gap));
		info->capacity = info->present;
		info->battery_full = false;
		info->battery_mid = true;
		info->battery_low = false;
		info->system_need_shutdown = false;	
		
	} else if ((info->compare_value <= info->vol_middle_limit) && \
				(info->compare_value > info->vol_lower_limit)) {
		divid_gap = ((info->vol_middle_limit - info->vol_lower_limit) / 45);
		info->present = (5 + ((info->compare_value - info->vol_lower_limit) / divid_gap));
		
		info->capacity = info->present;
		info->battery_full = false;
		info->battery_mid = false;
		info->battery_low = true;
		info->system_need_shutdown = false;	
		
	}  else if ((info->compare_value <= info->vol_lower_limit) && \
				(info->compare_value > info->vol_shutdown_limit)) {
		divid_gap = ((info->vol_lower_limit - info->vol_shutdown_limit) / 4);
		info->present = (1 + ((info->compare_value - info->vol_shutdown_limit) / divid_gap));
		
		info->capacity = info->present;
		info->battery_full = false;
		info->battery_mid = false;
		info->battery_low = true;
		info->system_need_shutdown = false;
		
	} else {
		info->present = 0;
		info->capacity = info->present;
		info->battery_full = false;
		info->battery_mid = false;
		info->battery_low = false;
		info->system_need_shutdown = true;	
	}
	mutex_unlock(&info->lock);

}

static int adc_battery_vol_recall(void * data)
{
	// int ret;
		
	struct adc_rk3288_battery_data *info = 	(struct adc_rk3288_battery_data *)data;
	if(!info) {
		dbg_msg("in func: %s, get adc_rk3288_battery_data failed\n", __func__);
		return -1;
	}
	dbg_msg("moved in func: %s\n", __func__);
	while(1) {
		if(kthread_should_stop()) {
			break;
		}
		if(!is_dc_exist(info)) {	// connceted battery
			// dbg_msg("battery connected\n");
			compare_system_status(info);
			// mdelay(info->adc_report_interval);
			schedule_timeout((info->adc_report_interval) * HZ);
		} else {
			// dbg_msg("dc power connected\n");
			info->capacity = 100;
			info->present = 100;
			// mdelay(info->adc_report_interval);
			schedule_timeout((info->adc_report_interval) * HZ);
		}
	}
	return 0;
}

static int adc_battery_get_property(struct power_supply *psy,
	 enum power_supply_property psp,
	 union power_supply_propval *val)
{
	struct adc_rk3288_battery_data *di = power_supply_get_drvdata(psy);
	int ret = 0;

	dbg_msg("moved in func: %s\n", __func__);

	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			// dbg_msg("in func: %s, POWER_SUPPLY_PROP_STATUS\n", __func__);
			if(is_dc_exist(di)) {		// dc power connector
				val->intval = POWER_SUPPLY_STATUS_CHARGING;
			} else {		// battery detect
				val->intval = POWER_SUPPLY_STATUS_DISCHARGING;			
			}
			dbg_msg("1. val->intval = %d\n", val->intval);
			break;
			case POWER_SUPPLY_PROP_HEALTH:
			// dbg_msg("in func: %s, POWER_SUPPLY_PROP_HEALTH\n", __func__);
				val->intval = di->health;
		break;
		
		case POWER_SUPPLY_PROP_PRESENT:
			// dbg_msg("in func: %s, POWER_SUPPLY_PROP_PRESENT\n", __func__);
			if(is_dc_exist(di)) {		// dc power connector
				val->intval = POWER_SUPPLY_STATUS_FULL;
			} else {		// battery detect
				val->intval = di->present;
			}
			dbg_msg("2. val->intval = %d\n", val->intval);
		break;
			
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			// dbg_msg("in func: %s, POWER_SUPPLY_PROP_TECHNOLOGY\n", __func__);
				val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
		case POWER_SUPPLY_PROP_CAPACITY:
			// dbg_msg("in func: %s, POWER_SUPPLY_PROP_CAPACITY\n", __func__);
				val->intval = 100;
		break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			// dbg_msg("in func: %s, POWER_SUPPLY_PROP_VOLTAGE_NOW\n", __func__);
			// val->intval = VIRTUAL_VOLTAGE * 1000;
		break;
		
		default:
			ret = -EINVAL;
		break;
	}

	return ret;
}

static int adc_battery_set_property(struct power_supply *psy,
	enum power_supply_property psp,
	const union power_supply_propval *val)
{
	struct adc_rk3288_battery_data *data = power_supply_get_drvdata(psy);
	int ret = 0;
	int value = val->intval;

	dbg_msg("moved in func: %s\n", __func__);

	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
		// dbg_msg("in func: %s, POWER_SUPPLY_PROP_STATUS\n", __func__);
			if (value <= POWER_SUPPLY_STATUS_FULL) {
				data->status =value;
			} else {
				printk("Unreasonable value:%d\n", value);
				ret = 1;
		}
		break;
		case POWER_SUPPLY_PROP_HEALTH:
		// dbg_msg("in func: %s, POWER_SUPPLY_PROP_HEALTH\n", __func__);
			if (value <= POWER_SUPPLY_HEALTH_COLD) {
				data->health = value;
			} else {
				printk("Unreasonable value:%d\n", value);
				ret = 1;
			}
		break;
		case POWER_SUPPLY_PROP_PRESENT:
		// dbg_msg("in func: %s, POWER_SUPPLY_PROP_PRESENT\n", __func__);
			data->present = value;
		break;
		case POWER_SUPPLY_PROP_CAPACITY:
		// dbg_msg("in func: %s, POWER_SUPPLY_PROP_CAPACITY\n", __func__);
			//Capacity from 0 to 100.
			if (value > 0 && value <= 100) {
				data->capacity = value;
			} else {
				printk("Unreasonable value:%d\n", value);
				ret = 1;
			}
		break;
		default:
			ret = -EINVAL;
		break;
	}

	//Send uevent.
	power_supply_changed(data->battery);

	return ret;
}

static int adc_battery_is_writeable(struct power_supply *psy,
	 enum power_supply_property psp)
{
	dbg_msg("moved in func: %s\n", __func__);
	switch (psp) {
		//Need to be written from medusa service.
		case POWER_SUPPLY_PROP_CAPACITY:
		case POWER_SUPPLY_PROP_STATUS:
		case POWER_SUPPLY_PROP_PRESENT:
		case POWER_SUPPLY_PROP_HEALTH:
		return 1;

		default:
		break;
	}

	return 0;
}


static int adc_battery_parse_dt(struct platform_device *pdev, struct adc_rk3288_battery_data *info)
{
	int ret;
	u32 val;
	enum of_gpio_flags flags;

	
	struct device_node *np = pdev->dev.of_node;
	if(!np) {
		printk(KERN_ERR BATTERY_TAG "cannot get dts node\n");
		return -ENODEV;
	}
	dbg_msg("moved in func: %s\n", __func__);

	// info->channel = iio_channel_get(&pdev->dev, "adc0");
	info->channel = iio_channel_get(&pdev->dev, NULL);
	if (IS_ERR(info->channel)) {
		printk(KERN_ERR BATTERY_TAG "no iio-channels defined\n");
		info->channel = NULL;
		return -ENODEV;
	} else {
		dbg_msg("got info->channel\n");
	}

	info->dc_detect_gpio = of_get_named_gpio_flags(np, "dc_check-gpio", 0, &flags);
	if(!gpio_is_valid(info->dc_detect_gpio)) {
		printk(KERN_ERR BATTERY_TAG "find node dc_check failed\n");
		return -EIO;
	} else {
		ret = devm_gpio_request(&pdev->dev, info->dc_detect_gpio, "battery-dc-check");
		if(ret < 0) {
			printk(KERN_ERR BATTERY_TAG "request dc_check failed\n");
			return -EIO;
		}
		if(flags) {
			info->irq_trigger_leve = 1;
			info->dc_input_detected = false;
		}
		else {
			info->irq_trigger_leve = 0;
			info->dc_input_detected = true;
		}
	
		dbg_msg("info->dc_detect_gpio = %d, info->irq_trigger_leve = %d\n", 
					info->dc_detect_gpio, info->irq_trigger_leve);
	}


	ret = of_property_read_u32(np, "report-interval-ms", &val);
	if(ret < 0) {
		printk(KERN_ERR BATTERY_TAG "report-interval-ms read failed\n");
		return -ENOENT;
	} else {
		info->adc_report_interval = val;
		dbg_msg("info->adc_report_interval = %d\n", info->adc_report_interval);
	}

	ret = of_property_read_u32(np, "lower-limit-vol-mv", &val);
	if(ret < 0) {
		printk(KERN_ERR BATTERY_TAG "lower-limit-vol-mv read failed\n");
		return -ENOENT;
	} else {
		info->vol_lower_limit = val;
		dbg_msg("info->vol_lower_limit = %d\n", info->vol_lower_limit);
	}
	
	ret = of_property_read_u32(np, "middle-limit-vol-mv", &val);
	if(ret < 0) {
		printk(KERN_ERR BATTERY_TAG "middle-limit-vol-mv read failed\n");
		return -ENOENT;
	} else {
		info->vol_middle_limit = val;
		dbg_msg("info->vol_middle_limit = %d\n", info->vol_middle_limit);
	}
	
	ret = of_property_read_u32(np, "upper-limit-vol-mv", &val);
	if(ret < 0) {
		printk(KERN_ERR BATTERY_TAG "upper-limit-vol-mv read failed\n");
		return -ENOENT;
	} else {
		info->vol_upper_limit = val;
		dbg_msg("info->vol_upper_limit = %d\n", info->vol_upper_limit);
	}

	ret = of_property_read_u32(np, "shutdown-vol-mv", &val);
	if(ret < 0) {
		printk(KERN_ERR BATTERY_TAG "shutdown-vol-mv read failed\n");
		return -ENOENT;
	} else {
		info->vol_shutdown_limit = val;
		dbg_msg("info->vol_upper_limit = %d\n", info->vol_shutdown_limit);
	}
	
	return 0;

}

struct power_supply_config psy_cfg;
// struct power_supply_desc eco_bat_desc;

static const struct power_supply_desc eco_bat_desc = {
	.name				      = "adc-battery",
	.type				      = POWER_SUPPLY_TYPE_BATTERY,
	.properties			      = adc_rk3288_battery_props,
	.num_properties		      = ARRAY_SIZE(adc_rk3288_battery_props),
	.get_property		      = adc_battery_get_property,
	.set_property		      = adc_battery_set_property,
	.property_is_writeable 	  = adc_battery_is_writeable,

	// void (*external_power_changed)(struct power_supply *psy);
	// void (*set_charged)(struct power_supply *psy);
};




static int adc_battery_probe(struct platform_device *pdev)
{
	int ret;
	struct adc_rk3288_battery_data *data;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		ret = -ENOMEM;
		goto err_data_alloc_failed;
	}
	dbg_msg("moved in func: %s\n", __func__);

	ret = adc_battery_parse_dt(pdev, data);
	if(ret < 0){
		dbg_msg("in func: %s, adc_battery_parse_dt failed\n", __func__);
	}

	mutex_init(&data->lock);
	
	if(is_dc_exist(data)) {		// dc power connector
		data->status = POWER_SUPPLY_STATUS_FULL;
		data ->capacity = 100;
		data->present   = 100;  //Present
	} else {				// battery detect
		data->status = POWER_SUPPLY_STATUS_NOT_CHARGING;	
		data ->capacity = 50;
		data->present   = 50;  //Present
	}
	data->health    = POWER_SUPPLY_HEALTH_GOOD;
	

	psy_cfg.drv_data = data;
	data->battery= devm_power_supply_register(&pdev->dev, &eco_bat_desc, &psy_cfg);
	if (IS_ERR(data->battery))
	goto err_battery_failed;

	platform_set_drvdata(pdev, data);

#if 0
	data->irq_num = gpio_to_irq(data->dc_detect_gpio);
	ret = devm_request_threaded_irq(&pdev->dev, data->irq_num, NULL, dc_detect_interrupt_recall,
									IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "dc-detect", (void *)data);
	if(ret < 0)
		printk(KERN_ERR BATTERY_TAG "request irq dc_detect_gpio failed\n");
#endif

	data->battery_vol_wrok = kthread_create(adc_battery_vol_recall, (void *)data, "adc_battery_vol_recall");
	if(IS_ERR(data->battery_vol_wrok)) {
		dbg_msg("adc_battery_vol_recall failed\n");
		return -1;
	} else 
		dbg_msg("adc_battery_vol_recall succeed\n");
	
	if (is_dc_exist(data)) {
		dbg_msg("in func: %s, dc connetor detected\n", __func__);
	} else {
		wake_up_process(data->battery_vol_wrok); 
		dbg_msg("in func: %s, dc is not connet\n", __func__);
	}
	

	dbg_msg("moved out func: %s\n", __func__);
	
	return 0;

err_battery_failed:
	kfree(data);
err_data_alloc_failed:
	return ret;
}

static int adc_battery_remove(struct platform_device *pdev)
{
	struct adc_rk3288_battery_data *data = platform_get_drvdata(pdev);

	power_supply_unregister(data->battery);

	kfree(data);
	return 0;
}

static const struct of_device_id adc_baterry_match_table[] = {
	{ .compatible = "rk3288-adc-battery" },
	{ /* Sentinel */ }
};

static struct platform_driver adc_rk3288_baterry_driver = {
	.probe	  = adc_battery_probe,
	.remove	 = adc_battery_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = BATERRY_DRIVER_NAME,
		.of_match_table = of_match_ptr(adc_baterry_match_table),
	}
};

static int __init adc_rk3288_baterry_init(void)
{
	dbg_msg("moved in func: %s\n", __func__);
	return platform_driver_register(&adc_rk3288_baterry_driver);
}

static void __exit adc_rk3288_batery_exit(void)
{
	dbg_msg("moved in func: %s\n", __func__);
	platform_driver_unregister(&adc_rk3288_baterry_driver);
}

// module_init(adc_rk3288_baterry_init);
late_initcall_sync(adc_rk3288_baterry_init);
module_exit(adc_rk3288_batery_exit);

// MODULE_AUTHOR("jiangq@machester@aliyun.com");
// MODULE_LICENSE("GPL");
// MODULE_DESCRIPTION("ADC Battery driver for rk3288");
/**-------------------------------- END LINE ---------------------------------------------*/