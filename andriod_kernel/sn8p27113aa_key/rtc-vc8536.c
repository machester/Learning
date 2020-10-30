/*  Date: 2016/01/05 15:16:00
 *  Revision: 2.5
 */

/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * 
 * All Rights Reserved
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/bcd.h>
#include <linux/bitops.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/rtc.h>

#define VC8536_CHIP_ID_REG                      0x00

enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_CONTROL_INFO = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
};

#define TAG								"<VC8536> "
#define SET_LEVEL						KERN_INFO
#define usr_msg(fmt, args...)			usr_msg(SET_LEVEL TAG fmt"\n", ##args)


#define SENSOR_NAME         "vc8536"
#define VC8536_I2C_ADDR     (0xA3 >> 1)

/* Register definitions */
#define VC8536_REG_CTRL1	0x00
#define VC8536_REG_CTRL2	0x01
#define VC8536_REG_SEC		0x02
#define VC8536_REG_MIN		0x03
#define VC8536_REG_HOUR		0x04
#define VC8536_REG_MDAY		0x05
#define VC8536_REG_WDAY		0x06
#define VC8536_REG_MONTH	0x07
#define VC8536_REG_YEAR		0x08
#define VC8536_REG_ALWMIN	0x09
#define VC8536_REG_ALWHOUR	0x0a
#define VC8536_REG_ALWMDAY	0x0b
#define VC8536_REG_ALWWDAY	0x0c
#define VC8536_REG_CLKOUTCTL	0x0d
#define VC8536_REG_TIMERCTL	0x0e
#define VC8536_REG_TIMERCD	0x0f

#define VC8536_BIT_AE_DISABLE	(0 << 7)
#define VC8536_BIT_AE_ENABLE	(1 << 7)

#define VC8536_BIT_CTRL1_TEST		(1 << 3)
#define VC8536_BIT_CTRL1_TESTC	(1 << 7)
#define VC8536_BIT_CTRL1_STOP		(1 << 5)

#define VC8536_BIT_CTRL2_TIE	(1 << 0)
#define VC8536_BIT_CTRL2_AIE	(1 << 1)
#define VC8536_BIT_CTRL2_TF	(1 << 2)
#define VC8536_BIT_CTRL2_AF	(1 << 3)
#define VC8536_BIT_CTRL2_TI	(1 << 4)


struct vc8536_data {
	struct i2c_client *client;
	struct rtc_device *rtc;
	u8 ctrl1;
	u8 ctrl2;

};

static const struct of_device_id vc_8536_dev_id[] = {
	{ .compatible = "vc8536"},
	{},
};
MODULE_DEVICE_TABLE(of, xfm10213_match_table);

static __u32 twi_id = 0;
static int i2c_num = 0;

static int vc8536_read_reg(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0)
		return -1;
		*data = dummy & 0x000000ff;

	return 0;
}

static int vc8536_write_reg(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int vc8536_read_regs(struct i2c_client *client,
		unsigned char reg_addr, unsigned char len, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int vc8536_write_regs(struct i2c_client *client,
		unsigned char reg_addr, unsigned char len, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int vc8536_get_time(struct device *dev, struct rtc_time *dt)
{
	struct vc8536_data *vc8536 = dev_get_drvdata(dev);
	u8 date[7];
	int err;

	err = vc8536_read_regs(vc8536->client, VC8536_REG_SEC, 7, date);
	if (err)
		return err;

	usr_msg("%s: read 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
        __func__, date[0], date[1], date[2], date[3], date[4], date[5], date[6]);

	dt->tm_sec = bcd2bin(date[0] & 0x7f);
	dt->tm_min = bcd2bin(date[1] & 0x7f);
//	if (vc8536->ctrl1 & VC8536_BIT_CTRL1_1224)
		dt->tm_hour = bcd2bin(date[2] & 0x3f);
//	else
//		dt->tm_hour = bcd2bin(date[VC8536_REG_HOUR] & 0x1f) % 12
//			+ (date[VC8536_REG_HOUR] & 0x20 ? 12 : 0);

	dt->tm_mday = bcd2bin(date[3] & 0x3f);
	dt->tm_mon = bcd2bin(date[5] & 0x1f);
	dt->tm_year = bcd2bin(date[6]);

	if (date[5] & 0x80){
		dt->tm_year += 2000;
	}else{
		dt->tm_year += 1900;
	}
	
	dt->tm_mon -= 1;
	
	//fix to the same as rtc0
/*	dt->tm_year -= 1900;
	dt->tm_mon -= 1;
	if(dt->tm_hour>= 8){
		dt->tm_hour -= 8;
	}else{
		dt->tm_hour = 24 - dt->tm_hour;
		dt->tm_mday -= 1;
		
	}
*/
	
	dev_warn(dev, "%s: date %ds %dm %dh %dmd %dm %dy\n", __func__,
		dt->tm_sec, dt->tm_min, dt->tm_hour,
		dt->tm_mday, dt->tm_mon, dt->tm_year);

	return rtc_valid_tm(dt);
}

int caculateweekday(int y,int m, int d)
{
		int iweek;
		if(m==1||m==2) {
				m+=12;
				y--;
		}
		iweek=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7; //0-monday
		usr_msg("%s  value: %02x\n", __func__, iweek);
		
		iweek += 1;
		if(iweek > 6)
			iweek = 0;
		return iweek; //return  valule 00:sunday 01:monday
}

static int vc8536_set_time(struct device *dev, struct rtc_time *dt)
{
	struct vc8536_data *vc8536 = dev_get_drvdata(dev);
	u8 date[7];
	int week;

	date[0] = bin2bcd(dt->tm_sec);
	date[1] = bin2bcd(dt->tm_min);
//	if (vc8536->ctrl1 & VC8536_BIT_CTRL1_1224)
		date[2] = bin2bcd(dt->tm_hour);
//	else
//		date[VC8536_REG_HOUR] = (dt->tm_hour >= 12 ? 0x20 : 0)
//			| bin2bcd((dt->tm_hour + 11) % 12 + 1);
	date[3] = bin2bcd(dt->tm_mday);
	date[4] = bin2bcd(dt->tm_wday);
	date[5] = bin2bcd(dt->tm_mon )+1;
	date[6] = bin2bcd(dt->tm_year % 100);
	
	if(dt->tm_year >= 2000){
		date[5] |= 0x80;	
	}
	
	week = caculateweekday(dt->tm_year,dt->tm_mon+1,dt->tm_mday);
	if(dt->tm_wday != week){
			date[4] = bin2bcd(week);
	}

	usr_msg("%s: write 0x%02x s 0x%02x m 0x%02x h 0x%02x d 0x%02x w 0x%02x mon 0x%02x",
		__func__, date[0], date[1], date[2], date[3], date[4], date[5], date[6]);

	return vc8536_write_regs(vc8536->client, VC8536_REG_SEC, 7, date);
}

static int vc8536_init_client(struct i2c_client *client, int *need_reset)
{
	struct vc8536_data *vc8536 = i2c_get_clientdata(client);
	u8 ctrl[2];
	int need_clear = 0;
	int err;

	err = vc8536_read_regs(vc8536->client, VC8536_REG_CTRL1, 2, ctrl);
	if (err)
		goto out;

	/* Keep test bit zero ! */
	vc8536->ctrl1 = ctrl[0] & ~VC8536_BIT_CTRL1_TEST;
	vc8536->ctrl1 = ctrl[0] & ~VC8536_BIT_CTRL1_TESTC;
	
	vc8536->ctrl2 = ctrl[1];

if (ctrl[0] & VC8536_BIT_CTRL1_TEST) {
		usr_msg("test mode for manufacturer was detected, "
			 "you may have to readjust the clock\n");
		*need_reset = 1;
	}
	
if (ctrl[0] & VC8536_BIT_CTRL1_TESTC) {
		usr_msg("testC mode for manufacturer was detected, "
			 "you may have to readjust the clock\n");
		*need_reset = 1;
	}

	if (ctrl[0] & VC8536_BIT_CTRL1_STOP) {
		usr_msg("power-on reset was detected, "
			 "you may have to readjust the clock\n");
		*need_reset = 1;
	}

	if (*need_reset || need_clear) {
		ctrl[0]	&= ~(VC8536_BIT_CTRL1_TEST | VC8536_BIT_CTRL1_TESTC |
			   VC8536_BIT_CTRL1_STOP);
		usr_msg("test ctrl1 == %#x\n",ctrl[0]);
		
		ctrl[1]	|= (VC8536_BIT_CTRL2_TIE | VC8536_BIT_CTRL2_AIE |
			   VC8536_BIT_CTRL2_TI);
		usr_msg("test ctrl2 == %#x\n",ctrl[1]);
		ctrl[1] = 0x00;
		vc8536->ctrl2 = ctrl[1];
		err = vc8536_write_regs(vc8536->client, VC8536_REG_CTRL1, 2, ctrl);
	}
	
	usr_msg("%s write ctr2 value: %02x\n", __func__, vc8536->ctrl2);
out:
	return err;
}

/* Alarm support */
static int vc8536_read_alarm(struct device *dev, struct rtc_wkalrm *t)
{
	struct vc8536_data *vc8536 = dev_get_drvdata(dev);
	struct i2c_client *client = vc8536->client;
	u8 ctrl2, ald[4];
	int err;

//	if (client->irq <= 0)
//		return -EINVAL;

	err = vc8536_read_regs(client, VC8536_REG_ALWMIN, 4, ald);
	if (err)
		return err;

	err = vc8536_read_reg(client, VC8536_REG_CTRL2, &ctrl2);
	if (err)
		return err;

	usr_msg("%s: read alarm 0x%02x 0x%02x 0x%02x 0x%02x ctrl2 %02x\n",
		__func__, ald[0], ald[1], ald[2], ald[3], ctrl2);

	/* Hardware alarms precision is 1 minute! */
	t->time.tm_sec = 0;
	t->time.tm_min = bcd2bin(ald[0] & 0x7f);
//	if (vc8536->ctrl1 & VC8536_BIT_CTRL1_1224)
		t->time.tm_hour = bcd2bin(ald[1] & 0x3f);
//	else
//		t->time.tm_hour = bcd2bin(ald[1] & 0x1f) % 12
//			+ (ald[1] & 0x20 ? 12 : 0);

	t->time.tm_wday = bcd2bin(ald[3] & 0x07);
	t->time.tm_mday = bcd2bin(ald[2] & 0x3f);
	t->time.tm_mon = -1;
	t->time.tm_year = -1;

	usr_msg("%s: date: %ds %dm %dh %dmd %dm %dy\n",	__func__,
    t->time.tm_sec, t->time.tm_min, t->time.tm_hour,
    t->time.tm_mday, t->time.tm_mon, t->time.tm_year);
	t->enabled = !!(vc8536->ctrl1 & VC8536_BIT_CTRL2_AIE);
	t->pending = (ctrl2 & VC8536_BIT_CTRL2_AF) && t->enabled;

	return err;
}

static int vc8536_set_alarm(struct device *dev, struct rtc_wkalrm *t)
{
    int err;
    u8 ald[4];
	// u8 test[16];
	u8 ctrl2;
	// struct i2c_client *client = to_i2c_client(dev);
	struct vc8536_data *vc8536 = dev_get_drvdata(dev);

	ctrl2 = vc8536->ctrl2;
	ctrl2 &= ~VC8536_BIT_CTRL2_AF;

//	if (client->irq <= 0)
//		return -EINVAL;

	/* Hardware alarm precision is 1 minute! */
	ald[0] = bin2bcd(t->time.tm_min);
//	if (vc8536->ctrl1 & VC8536_BIT_CTRL1_1224)
		ald[1] = bin2bcd(t->time.tm_hour);
//	else
//		ald[1] = (t->time.tm_hour >= 12 ? 0x20 : 0)
//			| bin2bcd((t->time.tm_hour + 11) % 12 + 1);
	ald[2] = bin2bcd(t->time.tm_mday);
	ald[3] = bin2bcd(t->time.tm_wday);

	usr_msg("%s: write 0x%02x 0x%02x 0x%02x 0x%02x\n", __func__, ald[0], ald[1], ald[2], ald[3]);

/*	if (vc8536->ctrl2 & VC8536_BIT_CTRL2_AIE) {
		vc8536->ctrl2 &= ~VC8536_BIT_CTRL2_AIE;
		err = vc8536_write_reg(vc8536->client, VC8536_REG_CTRL2,
				       vc8536->ctrl2);
		if (err)
			return err;
	}*/
	ald[0] &= ~ VC8536_BIT_AE_ENABLE;
	ald[1] &= ~ VC8536_BIT_AE_ENABLE;
	ald[2] &= ~ VC8536_BIT_AE_ENABLE;
	ald[3] |= VC8536_BIT_AE_ENABLE;
	
	usr_msg("%s: write 0x%02x 0x%02x 0x%02x 0x%02x\n", __func__, ald[0], ald[1], ald[2], ald[3]); 
	err = vc8536_write_reg(vc8536->client, VC8536_REG_CTRL2, &ctrl2);
	usr_msg("%s write ctr2 value: %02x\n", __func__, ctrl2);
	if (err)
		return err;	
	
	err = vc8536_write_regs(vc8536->client, VC8536_REG_ALWMIN, 4, ald);
	if (err)
		return err;
		
		
/*
	if (t->enabled) {
		vc8536->ctrl2 |= VC8536_BIT_CTRL2_AIE;
		err = vc8536_write_reg(vc8536->client, VC8536_REG_CTRL2,
				       vc8536->ctrl2);
		if (err)
			return err;
	}
*/
	return 0;
}

static int vc8536_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
	u8 ctrl2;
	u8 test[16];
	int err, i;
	struct vc8536_data *vc8536 = dev_get_drvdata(dev);

	usr_msg("%s enter \n", __func__);
	ctrl2 = vc8536->ctrl2;
	if (enabled)
		ctrl2 |= VC8536_BIT_CTRL2_AIE;
	else
		ctrl2 &= ~VC8536_BIT_CTRL2_AIE;

	if (ctrl2 != vc8536->ctrl2) {
		vc8536->ctrl2 = ctrl2;
		err = vc8536_write_reg(vc8536->client, VC8536_REG_CTRL2,
				       &ctrl2);
		usr_msg("%s write ctr2 value: %02x", __func__, ctrl2);
		if (err)
			return err;
	}
	
		err = vc8536_read_regs(vc8536->client, VC8536_REG_CTRL1, 16, test);
		if (err)
			return err;
		for(i = 0; i < 16; i++){
				usr_msg("reg %02x value is :%02x", i,test[i]);
		}
	
	return 0;
}

static struct rtc_class_ops vc8536_rtc_ops = {
	.read_time = vc8536_get_time,
	.set_time = vc8536_set_time,
	.read_alarm = vc8536_read_alarm,
	.set_alarm = vc8536_set_alarm,
	.alarm_irq_enable = vc8536_alarm_irq_enable,
};


/*
static void vc8536_work_func(struct work_struct *work)
{
	struct vc8536_data *vc8536 = container_of((struct delayed_work *)work,
			struct vc8536_data, work);
	static struct vc8536acc acc;
	unsigned long delay = msecs_to_jiffies(atomic_read(&vc8536->delay));

	mutex_lock(&vc8536->value_mutex);
	vc8536->value = acc;
	mutex_unlock(&vc8536->value_mutex);
	schedule_delayed_work(&vc8536->work, delay);
}*/


static int vc8536_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err = 0,need_reset = 0;
	struct vc8536_data *vc8536;
	
	// usr_msg("vc8536 probe i2c address is %d \n",i2c_address[i2c_num]);
	// client->addr =i2c_address[i2c_num];

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		usr_msg(KERN_INFO "i2c_check_functionality error\n");
		goto exit;
	}
    usr_msg("vc8536 i2c matched, slave address: 0x%x", client->addr);
	vc8536 = kzalloc(sizeof(struct vc8536_data), GFP_KERNEL);
	if (!vc8536) {
		err = -ENOMEM;
		goto exit_free;
	}

	i2c_set_clientdata(client, vc8536);
	vc8536->client = client;

//	INIT_DELAYED_WORK(&data->work, vc8536_work_func);

	err = vc8536_init_client(client, &need_reset);
	if (err)
		goto exit_free;

	if (need_reset) {
		struct rtc_time tm;
		usr_msg("bad conditions detected, resetting date");
		rtc_time_to_tm(0, &tm);	/* 1970/1/1 */
		tm.tm_year = 2011; /*set to 2011/1/1 Saturday*/
		tm.tm_mon += 1;
		tm.tm_wday = 6;
		usr_msg("%s:==init time %d:%d:%d:%d:%d:%d:%d ",__func__,tm.tm_year,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,tm.tm_wday);
		vc8536_set_time(&client->dev, &tm);
	}
	
	vc8536->rtc = rtc_device_register(client->name, &client->dev,
					  &vc8536_rtc_ops, THIS_MODULE);
	if (IS_ERR(vc8536->rtc)) {
		err = PTR_ERR(vc8536->rtc);
		usr_msg("unable to register the class device");
		goto exit_free;
	}
	
	usr_msg("vc8536: probe end");

	return 0;

exit_free:
	usr_msg("error,free device");
	kfree(vc8536);

exit:
	return err;
}

static int vc8536_remove(struct i2c_client *client)
{
	
	struct vc8536_data *vc8536 = i2c_get_clientdata(client);
    usr_msg("vc8536: remove");
	i2c_set_clientdata(client, NULL);
	kfree(vc8536);
	return 0;
}

static const struct i2c_device_id vc8536_id[] = {
	{ SENSOR_NAME, 0 },
	{ }
};


static struct i2c_driver vc8536_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= SENSOR_NAME,
		.of_match_table = of_match_ptr(vc_8536_dev_id),
	},
	.id_table	= vc8536_id,
	.probe		= vc8536_probe,
	.remove		= vc8536_remove,

	// .detect		= vc8536_detect,
	// .address_list	= normal_i2c,
};

module_i2c_driver(vc8536_driver);

MODULE_AUTHOR("V");
MODULE_DESCRIPTION("RTC vc8536 driver");
MODULE_LICENSE("GPL");


