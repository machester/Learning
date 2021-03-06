/***** key dts defined in file rk3288.dtsi ***********************
usr-key {
	.compati
};
*******************************************************************************/
#include "key_input.h"

#define show_HZ()           usr_msg("current system HZ = %u", HZ)

  

struct miscdevice misc_dev = {
	.name = KEY_NAME,
	.fops = misc_ops,
	.this_device
	.minor = 
	.mode

};

static const struct of_device_id usr_key_match_table[] = {
	{.compatible = "usr-key",},
    {/** keep this */},
};
// MODULE_DEVICE_TABLE(of, usr_key_match_table);



static ssize_t key_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	usr_msg("----> move in function key_show");
	
	return 0;
}

static ssize_t key_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	usr_msg("----> move in function key_store");
	
	return 0;
}

#if 1

DEVICE_ATTR(KEY_DEV_NAME, S_IRUGO | S_IWUSR, key_show, key_store);
#else
static struct device_attribute key_attr = {
	.attr = {
		.name = KEY_DEV_NAME,
		.mode=  S_IRUSR | S_IRGRP, //0664
	},
	.show = key_show,
	.store = key_store,
};
#endif

static struct attribute *key_attributes[] = {
	.dev_attr_runtime_suspend
};

static struct attribute_group key_attr_group =  {
	.attrs = key_attributes;
};


static int key_create_sysfs(struct device *dev)
{
	device_create_file(dev, &key_attr);
}


static int key_get_dts_info(struct platform_device *pdev)
{
	int ret;
	struct pinctrl * node;
	struct pinctrl_state * pin_state;

	usr_msg("ready to get dts information.");
	struct key_information * key_info = platform_get_drvdata(pdev);
	if(IS_ERR(key_info)) {
		return PTR_ERR(key_info);
	}
	mutex_init(&key_info->dts->pin_lock);
	mutex_lock(&key_info->dts->pin_lock);
	
	key_info->dts->node = devm_pinctrl_get(&pdev->dev);
	if(IS_ERR(key_info->dts->node)) {
		ret = ERR_PTR(-ENOENT);
		goto out;
	}
	key_info->dts->state = pinctrl_lookup_state(key_info->dts->node, "default");
	if(IS_ERR(key_info->dts->state)) {
		devm_pinctrl_put(key_info->dts->node);
		ret = PTR_ERR(key_info->dts->state);
		goto out;
	}
	ret = pinctrl_select_state(key_info->dts->node, key_info->dts->state);

	key_info->dts->dev_node = of_find_matching_node(NULL, usr_key_match_table);
	if(key_info->dts->dev_node) {
		key_info->dts->irq_no = irq_of_parse_and_map(key_info->dts->dev_node, 0);
		
	}
	
	mutex_unlock(&key_info->dts->pin_lock);
	
	return ret;
	
out:
	mutex_unlock(&key_info->dts->pin_lock);
	return ret;
}



static int key_allocate_node(struct key_information * key_info)
{
	int ret;

	key_info->device->devt = MKDEV(0, 0);
	usr_msg("allocate key device node information.");

	ret = alloc_chrdev_region(&key_info->device->devt, 0, 1, KEY_DEV_NAME);
	if(ret < 0)
		return ret;	
	key_info->device->major = MAJOR(key_info->device->devt);
	
}

static int key_input_dev_register(struct key_information * key_info)
{
	int ret;
	int irq_no;
	
	struct input_dev * i_dev;

	i_dev = input_allocate_device();
	if(IS_ERR(key_info->input_dev)) {
		ret = ERR_PTR(key_info->input_dev);
		return ret;
	}


	
	ret = input_register_device(key_info->input_dev);
	if (ret < 0) {
		goto err_out;
	}
	
	key_info->input_dev = i_dev;

	// gpio mapped to irq
	irq_no = gpio_to_irq(unsigned gpio);
	if(!irq_no) {
		err_msg("error : gpio mapped to irq.");
		ret = -1;
		goto out_map_irq;
	}
	// request_threaded_irq(unsigned int irq, irq_handler_t handler, irq_handler_t thread_fn, unsigned long irqflags, const char * devname, void * dev_id)
	request_threaded_irq(unsigned int irq, irq_handler_t handler, irq_handler_t thread_fn, 
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT, KEY_DEV_NAME, NULL)


	
	return ret;

out_map_irq:
	free_irq(irq_no, NULL);
err_out:
	input_free_device(key_info->input_dev);
	return ret;
}

static int key_probe(struct platform_device *pdev)
{
    int err;

	
	struct key_information * key_info;
	key_info = devm_kmalloc(&pdev->dev, sizeof(struct key_information), GFP_KERNEL);
	if(IS_ERR(key_info)) {
		err = ERR_PTR(key_info);
		return err;
	}
	memset(key_info, 0, sizeof(key_info));

	err = key_get_dts_info(pdev);

	if(err < 0) {
		err_msg("error : get dts information.");
	}
	
	key_info->input_dev = input_allocate_device();
	if(IS_ERR(key_info->input_dev)) {
		err = ERR_PTR(key_info->input_dev);
		return err;
	}
	
	/**************************************************
	 * input device set bit configuration
	**************************************************/

	err = key_create_sysfs(&pdev->dev);

	sysfs_notify(struct kobject * kobj, const char * dir, const char * attr);

	platform_set_drvdata(pdev, key_info);
	
	return err;

err_out:
	input_free_device(key_info->input_dev);
	return err;
}

static int key_remove(struct platform_device *pdev)
{
	struct key_information * key_info;
	key_info = platform_get_drvdata(pdev);

	input_free_device(key_info->input_dev);

	

	return 0;
}



static const struct platform_device_id usr_key_dev_id[] = {
    {"usr-key", 0},
    {/*keep this*/}
};
MODULE_DEVICE_TABLE(pwm, usr_key_dev_id);

struct platform_driver drv_key = {
	.driver = {
		.owner          = THIS_MODULE,
		.name           = KEY_DEV_NAME,
		.of_match_table = of_match_ptr(usr_key_match_table), 
	},
	.id_table = usr_key_dev_id,
	.probe    = key_probe,
	.remove   = key_remove,
};

module_platform_driver(drv_key);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("Linux kernel key demo.");
MODULE_ALIAS("KEY DEMO");
MODULE_VERSION(VERSION);
