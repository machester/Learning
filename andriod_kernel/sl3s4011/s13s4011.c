#include "sl3s4011.h"

// #define  "sl3s4011"

static int debug_status = 1;

#define TAG "<S4011> "
#define SET_LEVEL KERN_ERR
#define err_msg(fmt, args...) printk(SET_LEVEL TAG "error " fmt "\n", ##args)
#define usr_msg(fmt, args...)                       \
	do                                              \
	{                                               \
		if (debug_status)                           \
			printk(SET_LEVEL TAG fmt "\n", ##args); \
	} while (0)

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define COMPATIBLE_NAME			"mediatek,rfid_sl3s4011"
// #define COMPATIBLE_NAME			"rfid_sl3s4011"

#define CHIP_NAME				"sl3s4011"

#define MISC_DEV_DEF			0

// #define GET_HIGH_8BIT(var)			((var & 0xff) << 8))
// #define GET_LOW_8BIT(var)			(var >> 8)

struct sl3s4011_ctrl_struct
{
	unsigned int auto_report_data;
	unsigned int data_polling_interval;
};

struct sl3s4011_info
{
	struct i2c_client 				*client;
	struct mutex 					lock;

	unsigned char 					slave_addr;
	struct task_struct 				*sl3s4011_poll_data;
	struct timer_list 				report_timer;

	struct sl3s4011_ctrl_struct 	ctrl_param;
	struct kobject 					*k_obj;
	struct device					dev;		// for container of
};

static const struct i2c_device_id sl3s4011_id[] = {
	{CHIP_NAME, 0},
};

static const struct of_device_id sl3s4011_match_table[] = {
	{.compatible = COMPATIBLE_NAME},
};


/** function declaration start ----------------------------------------------------------------*/
static void sl3s4011_timer_start(struct sl3s4011_info *info);
/** function declaration end ----------------------------------------------------------------*/

uint16_t sl3s4011_i2c_readword(struct i2c_client *client, uint16_t read_addr)
{
	struct i2c_msg msg[2];
	uint8_t addr_buf[2];
	uint8_t data_tmp[2];
	uint16_t read_value;
	int ret;

	struct sl3s4011_info *info = i2c_get_clientdata(client);
	if (IS_ERR(info))
	{
		usr_msg("failed: in func: %s, struct sl3s4011_info", __func__);
		return PTR_ERR_OR_ZERO(info);
	}
	// mutex_lock(&info->lock);
	read_value = 0; // clean return data

	addr_buf[0] = (uint8_t)((read_addr >> 8) & 0xff);
	addr_buf[1] = (uint8_t)(read_addr & 0xff);
	// 16bit addr read
	msg[0].addr = client->addr;
	msg[0].flags = !I2C_M_RD;
	msg[0].len = 2;
	msg[0].buf = addr_buf;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = data_tmp;
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret < 0) {
		usr_msg("client->adapter->name: %s, read_addr = 0X%04X, ret = %d", client->adapter->name, read_addr, ret);
		mutex_unlock(&info->lock);
		return 0xffff;

	} else {
		usr_msg("before: read address = 0X%04X, data_tmp[0] = 0X%02X, data_tmp[1] = 0X%02X", read_addr, data_tmp[0], data_tmp[1]);
		read_value = (uint16_t)data_tmp[0];
		read_value = read_value << 8;
		read_value = read_value | (0x00ff & (uint16_t)data_tmp[1]);
		usr_msg("read address = 0X%04X, data = 0X%04X", read_addr, read_value);
	}
	// mutex_unlock(&info->lock);
	return read_value;
}

int sl3s4011_i2c_writeword(struct i2c_client *client, uint16_t write_addr, uint16_t data)
{
	struct i2c_msg msg;
	uint8_t data_buf[4];
	int ret;

	struct sl3s4011_info *info = i2c_get_clientdata(client);
	if (IS_ERR(info))
	{
		usr_msg("failed: in func: %s, struct sl3s4011_info", __func__);
		return PTR_ERR_OR_ZERO(info);
	}

	// mutex_lock(&info->lock);
	// format 16bit addr
	data_buf[0] = (uint8_t)((write_addr >> 8) & 0xff);
	data_buf[1] = (uint8_t)(write_addr & 0xff);
	data_buf[2] = (uint8_t)((data >> 8) & 0xff);
	data_buf[3] = (uint8_t)(data & 0xff);

	msg.addr = client->addr;
	msg.flags = !I2C_M_RD;
	msg.len = 4;
	msg.buf = data_buf;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		usr_msg("client->adapter->name: %s, addr = 0X%02X, ret = %d", client->adapter->name, client->addr, ret);
	else
		usr_msg("write addr: 0X%04X, data: 0X%04X", write_addr, data);
	// mutex_unlock(&info->lock);
	return ret;
}

static int sl3s4011_poll_data_thread_entry(void *data)
{
	uint16_t loop_addr_index;
	uint16_t value;

	struct sl3s4011_info *info = (struct sl3s4011_info *)data;
	if (IS_ERR(info)) {
		usr_msg("in function: %s, get struct sl3s4011_info failed", __func__);
		return PTR_ERR_OR_ZERO(info);
	}
	while (1) {
		if (kthread_should_stop()) {
			break;
		}
		else if (info->ctrl_param.auto_report_data) {
			mutex_lock(&info->lock);
			usr_msg("\n--------------- EPC SECTORY --------------------");
			for (loop_addr_index = S4011_EPC_START_ADDR; loop_addr_index <= S4011_EPC_END_ADDR; loop_addr_index += S4011_EPC_OffSET_ADRESS)
			{
				value = sl3s4011_i2c_readword(info->client, loop_addr_index);
				printk("addr[0X%04X] = 0X%04X, ", loop_addr_index, value);
			}
			usr_msg("\n------------------------------------------------");
			usr_msg("\n--------------- TID SECTORY --------------------");
			for (loop_addr_index = S4011_TID_START_ADDR; loop_addr_index <= S4011_TID_END_ADDR; loop_addr_index += S4011_TID_OffSET_ADRESS)
			{
				value = sl3s4011_i2c_readword(info->client, loop_addr_index);
				printk("addr[0X%04X] = 0X%04X, ", loop_addr_index, value);
			}
			usr_msg("\n------------------------------------------------");
			usr_msg("\n--------------- USR SECTORY --------------------");
			for (loop_addr_index = S4011_USR_START_ADDR; loop_addr_index <= S4011_USR_END_ADDR; loop_addr_index += S4011_USR_OffSET_ADRESS)
			{
				value = sl3s4011_i2c_readword(info->client, loop_addr_index);
				printk("addr[0X%04X] = 0X%04X, ", loop_addr_index, value);
			}
			usr_msg("\n------------------------------------------------");
			mutex_unlock(&info->lock);
			schedule_timeout(jiffies +  msecs_to_jiffies(info->ctrl_param.data_polling_interval));
		}
		else {
			schedule_timeout(jiffies +  msecs_to_jiffies(info->ctrl_param.data_polling_interval));
		}
	}
	return 0;
}

static int sl3s4011_thread_create(struct sl3s4011_info *info)
{
	usr_msg("moved in func: %s", __func__);
	// return 0;

	info->sl3s4011_poll_data = kthread_create(sl3s4011_poll_data_thread_entry, (void *)info, "sl3s4011_poll_data");
	if (IS_ERR(info->sl3s4011_poll_data)) {
		err_msg("failed: create sl3s4011_poll_data");
		return -1;
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ssize_t epc_part_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int index;
	struct sl3s4011_info *info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		return size;
	}
	
	if (size > 0) {
		usr_msg("in func: %s, buf = %s", __func__, buf);
		usr_msg("<SL3S4011>: inde");
		for(index = 0; index < size; index++)
			printk("buf[%d]= 0X%02X,  ", index, buf[index]);
	}
	usr_msg("\n---------------------------------------------------");
	return 0;
}


static ssize_t epc_part_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret, value_index;
	uint16_t addr_index, buff_index;
	struct sl3s4011_info *info = NULL;
	uint16_t value[S4011_EPC_WORDS_LEN] = {0};
	ret = 0;
	
	info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		ret = -1;
		return sprintf(buf, "%d\n", ret);
	}
	
	usr_msg("moved in func: %s", __func__);
	usr_msg("\n--------------- EPC SECTORY --------------------");
	mutex_lock(&info->lock);
	value_index = -1;
	for(addr_index = S4011_EPC_START_ADDR; addr_index <= S4011_EPC_END_ADDR; addr_index += S4011_EPC_OffSET_ADRESS)
	{
		value_index += 1;
		value[value_index] = sl3s4011_i2c_readword(info->client, addr_index);
		if(0xffff == value[value_index]) {
			mutex_unlock(&info->lock);
			return sprintf(buf, "-1\n");
		}
		usr_msg("value_index: %d, addr[0X%04X] = 0X%04X, ", value_index, addr_index, value[value_index]);
	}
	for(buff_index = 0; buff_index < value_index; buff_index++) {
		ret += sprintf(buf, "%04X", value[buff_index]);
		buf = buf + 4;
		usr_msg("ret = %d, value[%d] = 0X%04X", ret, buff_index, value[buff_index]);
	}
	mutex_unlock(&info->lock);
	ret += sprintf(buf, "\n");
	return ret;

}

static ssize_t tid_part_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int index;
	struct sl3s4011_info *info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		return size;
	}
	
	if (size > 0) {
		usr_msg("in func: %s, buf = %s", __func__, buf);
		usr_msg("<SL3S4011>: inde");
		for(index = 0; index < size; index++)
			printk("buf[%d]= 0X%02X,  ", index, buf[index]);
	}
	usr_msg("---------------------------------------------------");
	return 0;
}

static ssize_t tid_part_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret, value_index;
	uint16_t addr_index, buff_index;
	struct sl3s4011_info *info = NULL;
	uint16_t value[S4011_TID_WORDS_LEN] = {0};
	ret = 0;
	
	info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		ret = -1;
		return sprintf(buf, "%d\n", ret);
	}
	
	usr_msg("moved in func: %s", __func__);
	usr_msg("\n--------------- TID SECTORY --------------------");
	mutex_lock(&info->lock);
	value_index = -1;
	for(addr_index = S4011_TID_START_ADDR; addr_index <= S4011_TID_END_ADDR; addr_index += S4011_TID_OffSET_ADRESS) {
		value_index += 1;
		value[value_index] = sl3s4011_i2c_readword(info->client, addr_index);
		if(0xffff == value[value_index]) {
			mutex_unlock(&info->lock);
			return sprintf(buf, "-1\n");
		}
		printk(KERN_ERR "addr[0X%04X] = 0X%04X, ", addr_index, value[value_index]);
	}
	for(buff_index = 0; buff_index < value_index; buff_index++) {
		ret += sprintf(buf, "%04X", value[buff_index]);
		buf = buf + 4;
		usr_msg("ret = %d, value[%d] = 0X%04X", ret, buff_index, value[buff_index]);
	}
	mutex_unlock(&info->lock);
	ret += sprintf(buf, "\n");
	return ret;
}

static ssize_t usr_part_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int index;
	struct sl3s4011_info *info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		return size;
	}
	
	if (size > 0) {
		usr_msg("in func: %s, buf = %s", __func__, buf);
		usr_msg("<SL3S4011>: inde");
		for(index = 0; index < size; index++)
			printk("buf[%d]= 0X%02X,  ", index, buf[index]);
	}
	usr_msg("---------------------------------------------------");
	return 0;
}

static ssize_t usr_part_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret, value_index;
	uint16_t addr_index, buff_index;
	struct sl3s4011_info *info = NULL;
	uint16_t value[S4011_USR_WORDS_LEN] = {0};
	ret = 0;
	
	info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		ret = -1;
		return sprintf(buf, "%d\n", ret);
	}
	
	usr_msg("moved in func: %s", __func__);
	usr_msg("\n--------------- USER SECTORY --------------------");
	mutex_lock(&info->lock);
	value_index = -1;
	for(addr_index = S4011_USR_START_ADDR; addr_index <= S4011_USR_END_ADDR; addr_index += S4011_USR_OffSET_ADRESS) {
		value_index += 1;
		value[value_index] = sl3s4011_i2c_readword(info->client, addr_index);
		if(0xffff == value[value_index]) {
			mutex_unlock(&info->lock);
			return sprintf(buf, "-1\n");
		}
		usr_msg("value_index = %d, addr[0X%04X] = 0X%04X, ", value_index, addr_index, value[value_index]);
	}
	
	for(buff_index = 0; buff_index < value_index; buff_index++) {
		ret += sprintf(buf, "%04X", value[buff_index]);
		buf = buf + 4;
		usr_msg("ret = %d, value[%d] = 0X%04X", ret, buff_index, value[buff_index]);
		if((4 * buff_index) > 120) {		// char * buff, max number 127
			mutex_unlock(&info->lock);
			usr_msg("get the max size");
			ret += sprintf(buf, "\n");
			return ret;
		}
	}
	mutex_unlock(&info->lock);
	ret += sprintf(buf, "\n");
	return ret;
}

static ssize_t auto_report_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{

	struct sl3s4011_info *info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		return size;
	}
	if (size > 0) {
		usr_msg("in func: %s, buf[0] = %c", __func__, buf[0]);
		if ('0' == buf[0])
		{
			usr_msg("in func: %s, set auto report off", __func__);
			info->ctrl_param.auto_report_data = 0;
		}
		else
		{
			usr_msg("in func: %s, set auto report on", __func__);
			info->ctrl_param.auto_report_data = 1;
			wake_up_process(info->sl3s4011_poll_data);
		}
	}
	return size;
}

static ssize_t auto_report_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;
	struct sl3s4011_info *info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		ret = -1;
		return sprintf(buf, "%d\n", ret);
	}

	usr_msg("moved in func: %s", __func__);
	return sprintf(buf, "%d\n", info->ctrl_param.auto_report_data);
}

static ssize_t report_interval_sotre(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;
	uint32_t tim_value;

	struct sl3s4011_info *info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct pwr_info failed", __func__);
		return -1;
	}
	usr_msg("moved in func: %s", __func__);
	if (size > 0) {
		tim_value = 0;
		ret = sprintf((char *)&tim_value, "%d", buf[0]);
		usr_msg("in func: %s, buf[0] = %d, tim_value = %d", __func__, buf[0], tim_value);
		if (ret) {
			usr_msg("in func: %s, set report invert off", __func__);
			info->ctrl_param.data_polling_interval = tim_value;
			mod_timer(&info->report_timer, msecs_to_jiffies(info->ctrl_param.data_polling_interval));
		}
	}
	return size;
}

static ssize_t report_interval_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;
	//struct sl3s4011_info *info = dev_get_drvdata(dev); // container_of(dev, struct sl3s4011_info, member)
	struct sl3s4011_info *info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		ret = -1;
		return sprintf(buf, "%d\n", ret);
	}
	return sprintf(buf, "%d\n", info->ctrl_param.data_polling_interval);
}

static ssize_t debug_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{

	struct sl3s4011_info *info = dev_get_drvdata(dev);
	if (!info) {
		err_msg("in func: %s, get struct sl3s4011_info failed", __func__);
		return size;
	}
	if (size > 0) {
		usr_msg("in func: %s, buf[0] = %c", __func__, buf[0]);
		if ('0' == buf[0])
		{
			usr_msg("in func: %s, set debug off", __func__);

			debug_status = 0;
		}
		else {
			usr_msg("in func: %s, set debug on", __func__);
			debug_status = 1;
		}
	}
	return size;
}

static ssize_t debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	usr_msg("moved in func: %s", __func__);
	usr_msg("len --> S4011_EPC_WORDS_LEN = %d", S4011_EPC_WORDS_LEN);
	usr_msg("len --> S4011_TID_WORDS_LEN = %d", S4011_TID_WORDS_LEN);
	usr_msg("len --> S4011_USR_WORDS_LEN = %d", S4011_USR_WORDS_LEN);
	return sprintf(buf, "%d\n", debug_status);
}

static DEVICE_ATTR(epc_part, 0664, epc_part_show, epc_part_sotre);
static DEVICE_ATTR(tid_part, 0664, tid_part_show, tid_part_sotre);
static DEVICE_ATTR(usr_part, 0664, usr_part_show, usr_part_store);
static DEVICE_ATTR(auto_report, 0664, auto_report_show, auto_report_sotre);
static DEVICE_ATTR(report_interval, 0664, report_interval_show, report_interval_sotre);
static DEVICE_ATTR(debug, 0664, debug_show, debug_store);

static struct attribute *sl3s4011_ctrl_attributes[] = {

	&dev_attr_epc_part.attr, //name dev_attr_name.attr
	&dev_attr_tid_part.attr,
	&dev_attr_usr_part.attr,
	&dev_attr_auto_report.attr,
	&dev_attr_report_interval.attr,
	&dev_attr_debug.attr,

	NULL};

static struct attribute_group sl3s4011_ctrl_attribute_group = {
	.attrs = sl3s4011_ctrl_attributes,
};

static int sl3s4011_ctrl_sysfs_create(struct sl3s4011_info *info)
{
	int ret;
#if 0		// dev_get_drvdata cannot get driver data in this way
	info->k_obj = NULL;
	if (NULL == (info->k_obj = kobject_create_and_add(CHIP_NAME, NULL))) {	// create dir /sys/CHIP_NAME
		err_msg("sysfs_demo sys node create error");
		return -1;
	}

	ret = sysfs_create_group(info->k_obj, &sl3s4011_ctrl_attribute_group);
	if (ret) {
		err_msg("in function: %s, create sysfs failed", __func__);
		return -1;
	}
#else
	ret = sysfs_create_group(&info->client->dev.kobj, &sl3s4011_ctrl_attribute_group);
	if (ret) {
		err_msg("in function: %s, create sysfs failed", __func__);
		return -1;
	}

#endif

	return 0;
}

static void report_timer_recall(unsigned long args)
{
	struct sl3s4011_info *info = (struct sl3s4011_info *)args;
	if (IS_ERR(info)) {
		err_msg("in func: %s, get struct sl3s4011_info", __func__);
		return;
	}
	usr_msg("moved in func:%s", __func__);
	if (info->ctrl_param.auto_report_data)																	// if enabled auto report
		mod_timer(&info->report_timer, jiffies + msecs_to_jiffies(info->ctrl_param.data_polling_interval)); // trigger next timer trigger
}

static int sl3s4011_timer_create(struct sl3s4011_info *info)
{
	if (!info->ctrl_param.data_polling_interval) // timer not set
		return 0;

	init_timer(&info->report_timer);
	info->report_timer.function = report_timer_recall;
	info->report_timer.data = (unsigned long)info;

	info->report_timer.expires = jiffies + msecs_to_jiffies(info->ctrl_param.data_polling_interval);

	return 0;
}

static void sl3s4011_timer_start(struct sl3s4011_info *info)
{
	add_timer(&info->report_timer);
}

#if 0
static int sl3s4011_get_dts_info(struct sl3s4011_info *info)
{
	int ret;
	u32 value;
	struct device_node *node = NULL;
	node = of_find_compatible_node(NULL, NULL, COMPATIBLE_NAME);
	if (!node) {
		usr_msg("cannot find node name mediatek,rfid_sl3s4011");
		ret = -ENODEV;
		goto err1;
	}

	ret = of_property_read_u32(node, "report_time", &value);
	if (ret) {
		usr_msg("error: get report_time");
		info->ctrl_param.data_polling_interval = 0;
		return -EIO;
	}
	else
		info->ctrl_param.data_polling_interval = value;
	value = 0;
	ret = of_property_read_u32(node, "enable_auto_report", &value);
	if (ret) {
		usr_msg("error: get enable_auto_report");
		info->ctrl_param.auto_report_data = 0;
		return -EIO;
	}
	else {
		if (value)
			info->ctrl_param.auto_report_data = 1;
		else
			info->ctrl_param.auto_report_data = 0;
	}
	return 0;
err1:
	info->ctrl_param.auto_report_data = 0;
	info->ctrl_param.data_polling_interval = 0;
	return ret;
}

#else

static void sl3s4011_ctrl_param_init(struct sl3s4011_info *info)
{
	info->ctrl_param.auto_report_data = 0;
	info->ctrl_param.data_polling_interval = 2000;
}
#endif

static int sl3s4011_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	struct sl3s4011_info *info;
	usr_msg("moved in func: %s", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		err_msg("in func: %s, i2c check failed", __func__);
		return -ENODEV;
	}

	info = devm_kmalloc(&client->dev, sizeof(struct sl3s4011_info), GFP_KERNEL);
	if (IS_ERR(info))
	{
		usr_msg("error: devm_kmalloc info");
		return -ENOMEM;
	}

	usr_msg("detected slaver adapter address = 0X%02X", client->addr);
	info->client = client;
	/// sl3s4011_get_dts_info(info);
	sl3s4011_ctrl_param_init(info);
	mutex_init(&info->lock);

	i2c_set_clientdata(info->client, (void *)info);
	dev_set_drvdata(&client->dev, (void *)info);

	sl3s4011_timer_create(info);
	sl3s4011_ctrl_sysfs_create(info);
	sl3s4011_thread_create(info);

	if(info->ctrl_param.auto_report_data)
		sl3s4011_timer_start(info);
	
	return 0;
}

static int sl3s4011_remove(struct i2c_client *client)
{
	usr_msg("moved in func: %s", __func__);
	return 0;
}

static struct i2c_driver sl3s4011_driver = {
	.probe = sl3s4011_probe,
	.remove = sl3s4011_remove,
	.id_table = sl3s4011_id,
	.driver = {
	.name = CHIP_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sl3s4011_match_table,
	},
};

module_i2c_driver(sl3s4011_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("V");
MODULE_DESCRIPTION("sl3s4011 driver");
