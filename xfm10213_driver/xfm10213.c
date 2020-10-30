
#include "./xfm10213.h"

static int xfm10213_update_flag;


static const struct of_device_id xfm10213_match_table[] = {
	{ .compatible = "xfm10213"},
	{},
};
MODULE_DEVICE_TABLE(of, xfm10213_match_table);

#if 0
static irqreturn_t xfm10213_irq_handler(int irq, void *data)
{
	struct xfm10213_info *info = data;
	// struct i2c_client *client = info->client;
	usr_msg("moved in function: %s", __func__);
	upgrade_finished_mark = 1;
	disable_irq(info->irq_num);
	// schedule_work(xfm10213_irq_init);
	return IRQ_HANDLED;
}

static int xfm10213_irq_request(struct xfm10213_info *info)
{
	int ret, trigger_level;

	usr_msg("moved in function: %s", __func__);
	// using IRQF_ONESHOT to keep thread_handler full exit before accept next interrupt.
	trigger_level = info->irq_trigger_level ? IRQF_TRIGGER_RISING : IRQF_TRIGGER_FALLING;
	ret = request_threaded_irq(info->irq_num, NULL, xfm10213_irq_handler, trigger_level | IRQF_ONESHOT, XFM10213_DEV_NAME, (void *)info);
	if (ret) {
		usr_msg("failed to request IRQ");
		return ret;
	}
	return 0;
}
#endif



static ssize_t xfm10213_update_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}

static ssize_t xfm10213_wrok_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_wrok_mode_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_mic_channel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_mic_channel_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_mic_gain_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_mic_gain_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_uart_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_uart_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_out_gain_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_out_gain_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_i2s_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}
static ssize_t xfm10213_i2s_mode_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	usr_msg("moved in function: %s", __func__);
	return 0;
}

static DEVICE_ATTR(xfm10213_update, S_IRUGO, NULL, xfm10213_update_set);
static DEVICE_ATTR(xfm10213_wrok_mode, S_IRUGO, xfm10213_wrok_mode_show, xfm10213_wrok_mode_set);
static DEVICE_ATTR(xfm10213_mic_channel, S_IRUGO, xfm10213_mic_channel_show, xfm10213_mic_channel_set);
static DEVICE_ATTR(xfm10213_mic_gain, S_IRUGO, xfm10213_mic_gain_show, xfm10213_mic_gain_set);
static DEVICE_ATTR(xfm10213_uart, S_IRUGO, xfm10213_uart_show, xfm10213_uart_set);
static DEVICE_ATTR(xfm10213_out_gain, S_IRUGO, xfm10213_out_gain_show, xfm10213_out_gain_set);
static DEVICE_ATTR(xfm10213_i2s_mode, S_IRUGO, xfm10213_i2s_mode_show, xfm10213_i2s_mode_set);

static struct attribute * xfm10213_attributes[] = {
	&dev_attr_xfm10213_update.attr,
	&dev_attr_xfm10213_wrok_mode.attr,
	&dev_attr_xfm10213_mic_channel.attr,
	&dev_attr_xfm10213_mic_gain.attr,
	&dev_attr_xfm10213_uart.attr,
	&dev_attr_xfm10213_out_gain.attr,
	&dev_attr_xfm10213_i2s_mode.attr,
	NULL,
};
static const struct attribute_group xfm10213_attribute_group = {
	.attrs = xfm10213_attributes,
};

unsigned int xfm10213_read_16bit(struct i2c_client *client, unsigned int sub_addr)
{
  	unsigned long rxdata;
    unsigned char reg_buff[2];
	unsigned char data_buff[2];
    struct i2c_msg msgs[2];
	
    reg_buff[0] = sub_addr >>8;
    reg_buff[1] = sub_addr & 0xff;
    
    msgs[0].addr  = client->addr;
    msgs[0].flags = 0;
    msgs[0].len   = 2;
    msgs[0].buf   = reg_buff;

    msgs[1].addr  = client->addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len   = 2;
    msgs[1].buf   = data_buff;

    if(unlikely(i2c_transfer(client->adapter, msgs, 2) != 2)) {
		usr_msg("error: rec 16bit message.");
        return -1;
    }

	rxdata= ((rxdata | data_buff[0]) << 8) | data_buff[1];
    return rxdata;
}
int xfm10213_send_16bit(struct i2c_client *client, unsigned int sub_addr, unsigned int data)
{
	int ret;
	unsigned char buff[4] = {0};
	struct i2c_msg msg;

	buff[0] = sub_addr >> 8;
	buff[1] = sub_addr & 0xff;
	buff[2] = data >> 8;
	buff[3] = data & 0xff;
#if 1
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 4;
	msg.buf = buff;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if(ret < 0) {
		usr_msg("error: send 16bit message. ret = %d", ret);
		return ret;
	}
	return 0;
#else
	ret = i2c_master_send(client, buff, 4)
	if(ret > 0)
		usr_msg("sent %d byte mesage", ret);
	else {
		usr_msg("error: send 16bit message. ret = %d", ret);
		return ret;
	}
	return 0;
#endif
}
/** start: declare xfm10213 upgrade work -----------------------------------------------------*/
static void xfm10213_update_protocal(struct work_struct *work);
static DECLARE_WAIT_QUEUE_HEAD(xfm10213_upgrade_work_head);
static struct work_struct upgrade_work;
static int upgrade_flag;
static void xfm10213_update_protocal(struct work_struct *work)
{
    usr_msg("xfm10213_update_protocal woke-up, xfm10213_update_flag = %d", xfm10213_update_flag);
	struct xfm10213_info * info = container_of(work, struct xfm10213_info, xfm10213_upgrade_work);
	if(IS_ERR(info)) {
		usr_msg("error: get struct xfm10213_info in function: %s", __func__);
		return -PTR_ERR(info);
	}
	xfm10213_update_flag= 0;

	/** TODO */
	wake_up_interruptible(info->xfm10213_upgrade_work);
}
void xfm10213_create_upgrade_work(struct xfm10213_info * info)
{
	upgrade_flag = 0;
	info->xfm10213_upgrade_flag = upgrade_flag;
	xfm10213_update_flag = info->xfm10213_upgrade_flag;
	info->xfm10213_upgrade_work = &upgrade_work;		// transfer work to struct to manage
    INIT_WORK(info->xfm10213_upgrade_work, xfm10213_update_protocal);
    schedule_work(info->xfm10213_upgrade_work);
	wait_event_interruptible(xfm10213_upgrade_work_head, xfm10213_update_flag != 0);
}
void xfm10213_upgrade_work_remove(struct xfm10213_info * info)
{
	usr_msg("moved in function: %s", __func__);
	cancel_work_sync(info->xfm10213_upgrade_work);
	flush_work(info->xfm10213_upgrade_work);
}
/** end: declare xfm10213 upgrade work -----------------------------------------------------*/

/** start: declare xfm10213 init work -----------------------------------------------------*/
static void xfm10213_init_protocal(struct work_struct *work); 
struct delayed_work init_work;
static DECLARE_DELAYED_WORK(xfm10213_init_work, xfm10213_init_protocal); 
static void xfm10213_init_protocal(struct work_struct *work)
{
	usr_msg("moved in function: %s", __func__);
	struct xfm10213_info * info = container_of(work, struct xfm10213_info, xfm10213_init_work);
	if(IS_ERR(info)) {
		usr_msg("error: get struct xfm10213_info in function: %s", __func__);
		return ;
	}
	if(0 == info->init_flag) {
	xfm10213_send_16bit(info->client, UART_PRINT_CTRL_REG, UART_PRINT_HMI_MSG);
	xfm10213_send_16bit(info->client, VOI_WORK_MODE_REG, VOI_WORK_TEST_MODE);

	gpio_set_value(info->reset_pin, 0);
	mdelay(50);
	gpio_set_value(info->reset_pin, 1);
	mdelay(100);
	info->init_flag = 0;
	init_flag = 1;
	
}
int xfm10213_create_init_work(struct xfm10213_info * info)
{
	usr_msg("moved in function: %s", __func__);
	init_flag = 0;
	info->init_flag = init_flag;
	xfm10213_update_flag = info->init_flag;
	info->xfm10213_init_work = &init_work;		// transfer work to struct to manage
	INIT_DELAYED_WORK(info->xfm10213_init_work, xfm10213_init_protocal); 
}
void xfm10213_init_work_remove(struct xfm10213_info * info)
{
	usr_msg("moved in function: %s", __func__);
	cancel_work_sync(info->xfm10213_init_work);
	cancel
	flush_work(info->xfm10213_upgrade_work);
}
/** end: declare xfm10213 init work -----------------------------------------------------*/

static int xfm10213_get_dts_info(struct i2c_client * client, struct xfm10213_info * info)
{
	int ret;
	const char * temp_str;
	
	usr_msg("moved in function: %s", __func__);
	
	if(!client->dev.of_node) {
		usr_msg("cannot find device node");
		return -ENODEV;
	}
	usr_msg("device node found");
	info->node = client->dev.of_node;
	if(!info->node) {
		usr_msg("error: info->node empty");
		return -ENODEV;
	}
	usr_msg("ready to get irq-gpio");
	info->irq_pin = of_get_named_gpio(info->node, "irq-gpio", 0);
	if (!gpio_is_valid(info->irq_pin)) {
        usr_msg("error : irq-gpio not valid");
        return -EIO;
	}
	usr_msg("ready to get reset-gpio");
	info->reset_pin = of_get_named_gpio(info->node, "reset-gpio", 0);
	if (!gpio_is_valid(info->reset_pin)) {
		gpio_free(info->reset_pin);
		if (!gpio_is_valid(info->reset_pin)) {
        	usr_msg("error : reset-gpio not valid");
        	return -EIO;
		}
	}
	usr_msg("ready to get irq-trigger-level");
	ret = of_property_read_string(info->node, "irq-trigger-level", &temp_str);
	if(ret < 0) {
		usr_msg("error: get xfm10213-irq-trigger-level");
		return -EIO;
	}
	if(0 == strncmp("low", temp_str, 3))
		info->irq_trigger_level = 0;
	else
		info->irq_trigger_level = 1;
	
	info->irq_num = gpio_to_irq(info->irq_pin);
	usr_msg("ready to request gpio");

	ret = devm_gpio_request_one(&client->dev, info->irq_pin, GPIOF_IN, "xfm10213-irq-gpio");
	if(ret < 0) {
		usr_msg("error: get xfm10213-irq-gpio");
		return -EIO;
	}
	ret = devm_gpio_request_one(&client->dev, info->reset_pin, GPIOF_OUT_INIT_HIGH, "xfm10213-reset");
	if(ret < 0) {
		usr_msg("error: get xfm10213-reset");
		goto dts_out;
	}
	usr_msg("requested irq_pin = %d, irq_num = %d, setted irq-trigger-level = %s, reset-pin = %d",
					info->irq_pin, info->irq_num, info->irq_trigger_level ? "HIGH" : "LOW",
					info->reset_pin);
	return 0;

dts_out:
	devm_gpio_free(&client->dev, info->irq_pin);
	return ret;

}

static int xfm10213_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	// struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct xfm10213_info * info = kmalloc(sizeof(struct xfm10213_info), GFP_KERNEL);
	if(!info) {
		usr_msg("error: malloc struct xfm10213_info");
		return -ENOMEM;
	}
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		usr_msg("i2c check failed.");
		goto out;
	}
	usr_msg("i2c matched current slave addr is : 0x%x", client->addr);
	info->client = client;
	info->addr = client->addr;
	usr_msg("ready to get dts information");
	ret = xfm10213_get_dts_info(client, info);
	if(ret < 0) {
		usr_msg("error: get dts info");
		goto out;
	}
	ret = xfm10213_chip_init(info);
	if(ret < 0) {
		usr_msg("error: xfm10213_chip_init");
		goto out;
	}

    mutex_init(&info->lock);
	
	i2c_set_clientdata(client, info);
//	i2c_set_adapdata(struct i2c_adapter * dev, void * data);
 	
	xfm10213_upgrade_work_init(info);
	xfm10213_create_init_work(info); 
	schedule_delayed_work(info->xfm10213_init_work, msecs_to_jiffies(100));
	ret = sysfs_create_group(&client->dev.kobj, &xfm10213_attribute_group);
	if(ret < 0) {
		usr_msg("error: sysfs_create_group");
		goto out;
	}
	return 0;

out:
	kfree(info);
	return ret;

}

static int xfm10213_remove(struct i2c_client *client)
{
	struct xfm10213_info *info = i2c_get_clientdata(client);
	free_irq(info->irq_num, NULL);
	sysfs_remove_group(&client->dev.kobj, &xfm10213_attribute_group);
	return 0;
}

static const struct i2c_device_id xfm10213_dev_id[] = {
    {"xfm10213", 0},
    {/*keep this*/}
};
MODULE_DEVICE_TABLE(i2c, xfm10213_dev_id);

static struct i2c_driver xfm10213_driver = {
	.driver = {
		.name = XFM10213_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(xfm10213_match_table),
	},
	.probe = xfm10213_probe,
	.remove = xfm10213_remove,
	.id_table = xfm10213_dev_id,
};

static int __init xfm10213_init(void)
{
	usr_msg("moved in function: %s", __func__);
	i2c_add_driver(&xfm10213_driver);
	return 0;
}

static void __exit xfm10213_exit(void)
{
	usr_msg("moved in function: %s", __func__);
	i2c_del_driver(&xfm10213_driver);
}

// module_i2c_driver(xfm10213_driver);

module_init(xfm10213_init);
module_exit(xfm10213_exit);

MODULE_AUTHOR("V");
MODULE_DESCRIPTION("XFM10213 Driver");
MODULE_LICENSE("GPL");

