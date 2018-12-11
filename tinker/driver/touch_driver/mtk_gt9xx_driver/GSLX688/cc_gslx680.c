#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/of_irq.h>
#include <linux/dma-mapping.h>

#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
#include <linux/wakelock.h>
#include <linux/timer.h>
#include <mach/upmu_sw.h>

#include "tpd.h"
#include "upmu_common.h"
#include "mt_boot_common.h"

//#include "GSLxxx_xxx_xx_xx.h"

// added by elink_phil start <<<
#include <elink_tp.h>

#include <gsl_common.h>
#include <configs/SILEAD_COMPATIBILITY.h>

extern void init_tp_power(struct device *dev, char *name);
extern void tp_power_on(int on);

extern void elink_tp_get_config(u8 *r_buf, const struct fw_data **ptr_fw, unsigned int *source_len, unsigned int **data_id);
extern void elink_tp_misc_operation(struct i2c_client *client, int (*get_tp_resolution)(struct i2c_client *client, u16 reg));
extern void elink_tp_remap(int *x, int *y);

const struct fw_data *g_ptr_fw = GSL_COMPAT_FW;
unsigned int g_source_len = ARRAY_SIZE(GSL_COMPAT_FW);
unsigned int *g_config_data_id;
// added end >>>


#ifndef	TP_DEV_NAME
	#define TP_DEV_NAME 			"gsl1688"
#else
	#undef TP_DEV_NAME
	#define TP_DEV_NAME 			"gsl1688"
#endif

#ifndef	TP_DRV_NAME
	#define TP_DEV_DRV_NAME 			"gsl1688_driver"
#else
	#undef TP_DRV_NAME
	#define TP_DRV_NAME 			"gsl1688_driver"
#endif

typedef struct
{
	struct mutex i2c_lock;

}gsl_1688_i2c_info;


typedef struct {
    
	strcut device_node * gsl_node;
    struct mutex    * gsl_mutex;
    

}gsl_1688_info;

gsl_1688_i2c_info	* gsl_i2c_info;
gsl_1688_info		* gsl_1688_dev_info;

static struct workqueue_struct 	* gsl_1688_init_queue;

static int gsl1688_get_dts_info(void);

/************************************************************************************
alps\device\mediateksample\tb8735ba1_bsp\elink\PCBA\L863\tb8735ba1_bsp.dts
&i2c1 {
	silead_touch@40 {
		compatible = "mediatek,silead_touch";
		reg = <0x40>;

		interrupt-parent = <&eintc>;
		interrupts = <10 IRQ_TYPE_EDGE_FALLING>;
		debounce = <10 0>;
	};
};
************************************************************************************/

static struct i2c_device_id gsl_i2c_dev_match[] = {
	{ .compatible = "mediatek, silead_touch", },

};
MODULE_DEVICE_TABLE(of, gsl_i2c_dev_match);

static const struct of_device_id tpd_of_match[] = {
	{.compatible = "mediatek,silead_touch",},
	{},
};

static int gsl1688_get_dts_info(void);

static int gsl_i2c_probe(struct i2c_client * client, const struct i2c_device_id * dev_id);
static int gsl_i2c_release(struct i2c_client * client);
static int gsl_i2c_suspend(struct i2c_client * client, pm_message_t mesg);
static int gsl_i2c_resume(struct i2c_client * client);
static int gsl_i2c_detect(struct i2c_client * client, struct i2c_board_info * board_info)


static int gsl_i2c_release(struct i2c_client * client)
{
	return 0;
}

static int gsl_i2c_suspend(struct i2c_client * client, pm_message_t mesg)
{
	return 0;
}

static int gsl_i2c_resume(struct i2c_client * client)
{
	return 0;
}
static int gsl_i2c_detect(struct i2c_client * client, struct i2c_board_info * board_info)
{
	return 0;
}

static int gsl_i2c_probe(struct i2c_client * client, const struct i2c_device_id * dev_id)
{
	const struct i2c_device_id * client_match;

	client_match = of_match_device(gsl_i2c_dev_match, &client->dev);
	if(match) {

	} else {
		
	}


}







static struct of_device_id gsl_i2c_dev_match[] = {
	{compatible = "mediatek, silead_touch", },
}
/***************************************************************************
	int (*probe)(struct i2c_client *, const struct i2c_device_id *);
	int (*remove)(struct i2c_client *);
	void (*shutdown)(struct i2c_client *);
	int (*suspend)(struct i2c_client *, pm_message_t mesg);
	int (*resume)(struct i2c_client *);
	int (*detect)(struct i2c_client *, struct i2c_board_info *);
	const unsigned short *address_list;
******************************************************************************/
static stuct i2c_driver  gsl1688_i2c_drv = {
	.driver = {
		.name           = TP_DEV_NAME,
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(gsl_i2c_dev_match),
	},
	.probe    = gsl_i2c_probe,
	.release  = gsl_i2c_release,
	.detect   = gsl_i2c_detect,
	.suspend  = gsl_i2c_suspend,
	.resume   = gsl_i2c_resume,
	.id_table = -1,
};

static struct tpd_driver_t gsl1688_plt_drv = {
	.tpd_device_name = TP_DRV_NAME,
	.tpd_local_init  = gsl1688_local_init,
	.suspend         = gsl1688_suspend,
	.resume          = gsl1688_resume,
};

typedef struct {
	unsigned char 		*gsl_i2c_data;
	struct i2c_client 	*gsl_i2c_client;
	struct mutex 		gsl_i2c__mutex;
	int 				current_pointer;
	struct cdev 		gsl_i2c_cdev;

}gsl_1688_dev_info;

gsl_1688_dev_info	* gsl_dev_info;


static int gsl1688_get_dts_info(void)
{
    // find from root node
    gsl_node = of_find_matching_node(NULL, touch_of_match)
}

static int __init tpd_driver_init(vod)
{
	int ret = -1;
	gsl1688_get_dts_info();
	// 将 tpd_device_driver 加入到 结构体数组 tpd_driver_list[] 中
	ret = tpd_driver_add(&gsl1688_plt_drv);
	if ( ret < 0)
		printk(KERNEL_INFO "add gslX680 platform driver failed\n");
}

static void __exit gsl_driver_exit(void)
{
	tpd_driver_remove(&gsl1688_plt_drv)
}


 
module_i2c_driver(gsl1688_plt_drv);


module_init(tpd_driver_init);
module_exit(tpd_driver_exit);
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("touch panel drver");














































#if 0
//#define PEN_Adjust_Freq
//#define TPD_PROXIMITY
//#define HIGH_SPEED_I2C
//#define FILTER_POINT
#define GSL_NOID_VERSION
#define GSLX680_NAME "gslX680"
#define GSLX680_ADDR 0x40
#define MAX_FINGERS 10
#define MAX_CONTACTS 10
#define DMA_TRANS_LEN 0x20
#define SMBUS_TRANS_LEN 0x01
#define GSL_PAGE_REG 0xf0

#define TP_POWER "vtouch" //#define TP_POWER "VGP1" //MT6328_POWER_LDO_VGP1

int touch_irq_num;
u8 int_type = 0;

#ifdef TPD_PROC_DEBUG
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#if 0
static struct proc_dir_entry *gsl_config_proc = NULL;
#endif
#define GSL_CONFIG_PROC_FILE "gsl_config"
#define CONFIG_LEN 31
static char gsl_read[CONFIG_LEN];
static u8 gsl_data_proc[8] = {0};
static u8 gsl_proc_flag    = 0;
#endif

//static unsigned int GSL_TP_ID_USED = 0;

static int tpd_flag                  = 0;
static int tpd_halt                  = 0;
static char eint_flag                = 0;
extern struct tpd_device *tpd;
static struct i2c_client *i2c_client = NULL;
static struct task_struct *thread    = NULL;
#ifdef GSL_MONITOR
static struct delayed_work gsl_monitor_work;
static struct workqueue_struct *gsl_monitor_workqueue = NULL;
static u8 int_1st[4]                                  = {0};
static u8 int_2nd[4]                                  = {0};
static char bc_counter                                = 0;
static char b0_counter                                = 0;
static char i2c_lock_flag                             = 0;
#endif

static uint32_t id_sign[MAX_CONTACTS + 1]          = {0};
static uint8_t id_state_flag[MAX_CONTACTS + 1]     = {0};
static uint8_t id_state_old_flag[MAX_CONTACTS + 1] = {0};
static uint16_t x_old[MAX_CONTACTS + 1]            = {0};
static uint16_t y_old[MAX_CONTACTS + 1]            = {0};
static uint16_t x_new                              = 0;
static uint16_t y_new                              = 0;

static DECLARE_WAIT_QUEUE_HEAD(waiter);
extern void mt_eint_unmask(unsigned int line);
extern void mt_eint_mask(unsigned int line);

#ifdef GSL_DEBUG
#define print_info(fmt, args...) \
	do                           \
	{                            \
		printk(fmt, ##args);     \
	} while (0)
#else
#define print_info(fmt, args...)
#endif

#define gsl_info(fmt, args...) 								\
	do{														\
		printk(KERN_ERR "%s"fmt, "---> ", ##args);			\
	}while(0)


#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static struct mutex gsl_i2c_lock;
#define I2C_TRANS_SPEED 400 //100 khz or 400 khz
/*****************************************************************************
Prototype    	: gsl_read_interface
Description  	: gsl chip tranfer function
Input        	: struct i2c_client *client
u8 reg
u8 *buf
u32 num
Output		: None
Return Value 	: static

 *****************************************************************************/
static int gsl_read_interface(struct i2c_client *client,
                              u8 reg, u32 num, u8 *buf)
{
	int err = 0;
	int i;
	u8 temp = reg;

	gsl_info("<info>[get in %s]", __func__);
	mutex_lock(&gsl_i2c_lock);
	if (temp < 0x80)
	{
		if (temp == 0x7c)
		{
			temp = 0x78;
			i2c_master_send(client, &temp, 1);
			err = i2c_master_recv(client, &buf[0], 4);
			temp = 0x7c;
		}
		else
		{
			i2c_master_send(client, &temp, 1);
			err = i2c_master_recv(client, &buf[0], 4);
		}
	}
	for (i = 0; i < num;)
	{
		temp = reg + i;
		i2c_master_send(client, &temp, 1);
		if ((i + 8) < num)
			err = i2c_master_recv(client, (buf + i), 8);
		else
			err = i2c_master_recv(client, (buf + i), (num - i));
		i += 8;
	}
	mutex_unlock(&gsl_i2c_lock);

	gsl_info("<info>[go out %s]", __func__);
	return err;
}

/*****************************************************************************
Prototype    : gsl_write_interface
Description  : gsl chip tranfer function
Input        : struct i2c_client *client
const u8 reg
u8 *buf
u32 num
Output       : None
Return Value : static

 *****************************************************************************/
static int gsl_write_interface(struct i2c_client *client,
                               const u8 reg, u32 num, u8 *buf)
{
	struct i2c_msg xfer_msg[1] = {{0}};
	int err;
	u8 tmp_buf[num + 1];
	//	u8 retry = 0;

	tmp_buf[0] = reg;

	gsl_info("<info>[get in %s]", __func__);

	memcpy(tmp_buf + 1, buf, num);

	xfer_msg[0].addr = client->addr;
	xfer_msg[0].len = num + 1;
	xfer_msg[0].flags = client->flags & I2C_M_TEN;
	xfer_msg[0].buf = tmp_buf;

	mutex_lock(&gsl_i2c_lock);
	err = i2c_transfer(client->adapter, xfer_msg, 1);
	mutex_unlock(&gsl_i2c_lock);

	gsl_info("<info>[go out %s]", __func__);

	return 0;
}

static void startup_chip(struct i2c_client *client)
{
	char write_buf = 0x00;

	gsl_info("<info>[get in %s]", __func__);

	gsl_write_interface(client, 0xe0, 1, &write_buf);
	gsl_DataInit(g_config_data_id);

	msleep(10);
	gsl_info("<info>[go out %s]", __func__);
}

static void reset_chip(struct i2c_client *client)
{
	u8 write_buf[4] = {0};
	gsl_info("<info>[get in %s]", __func__);
	tpd_gpio_output(GTP_RST_PORT, 0);
	msleep(10);

	tpd_gpio_output(GTP_RST_PORT, 1);
	msleep(10);

	write_buf[0] = 0x04;
	gsl_write_interface(client, 0xe4, 1, &write_buf[0]);
	msleep(10);

	write_buf[0] = 0x00;
	write_buf[1] = 0x00;
	write_buf[2] = 0x00;
	write_buf[3] = 0x00;
	gsl_write_interface(client, 0xbc, 4, write_buf);
	msleep(10);
	gsl_info("<info>[go out %s]", __func__);
}

static void clr_reg(struct i2c_client *client)
{
	char write_buf[4] = {0};
	gsl_info("<info>[get in %s]", __func__);

	tpd_gpio_output(GTP_RST_PORT, 0);
	msleep(10);

	tpd_gpio_output(GTP_RST_PORT, 1);
	msleep(10);

	write_buf[0] = 0x03;
	gsl_write_interface(client, 0x80, 1, &write_buf[0]);
	msleep(5);

	write_buf[0] = 0x04;
	gsl_write_interface(client, 0xe4, 1, &write_buf[0]);
	msleep(5);

	write_buf[0] = 0x00;
	gsl_write_interface(client, 0xe0, 1, &write_buf[0]);
	msleep(20);
	gsl_info("<info>[go out %s]", __func__);
}

static void gsl_load_fw(struct i2c_client *client);

static void cfg_adjust(struct i2c_client *client)
{
	u8 read_buf[4] = {0};
	int ret = 0;
	gsl_info("<info>[get in %s]", __func__);
	
	msleep(500);
	ret = gsl_read_interface(client, 0xb8, sizeof(read_buf), read_buf);
	gsl_info("<info>[cfg_adjust 0xb8 [%x : %x : %x : %x] [ret = %d]]", read_buf[2], read_buf[1], read_buf[0], ret);
	elink_tp_get_config(read_buf, &g_ptr_fw, &g_source_len, &g_config_data_id);

	clr_reg(client);
	reset_chip(client);

	gsl_load_fw(client);
	startup_chip(client);
	reset_chip(client);
	startup_chip(client);
	gsl_info("<info>[go out %s]", __func__);
}

static void gsl_load_fw(struct i2c_client *client)
{
	unsigned char buf[SMBUS_TRANS_LEN * 4] = {0};
	unsigned char reg = 0, send_flag = 1, cur = 0;

	unsigned int source_line = 0;
	unsigned int source_len;
	const struct fw_data *ptr_fw;

	ptr_fw = g_ptr_fw;
	source_len = g_source_len;

	gsl_info("<info>[get in %s]", __func__);
	for (source_line = 0; source_line < source_len; source_line++)
	{
		if (1 == SMBUS_TRANS_LEN)
		{
			reg = ptr_fw[source_line].offset;
			memcpy(&buf[0], &ptr_fw[source_line].val, 4);
			gsl_write_interface(client, reg, 4, buf);
		}
		else
		{
			/* init page trans, set the page val */
			if (GSL_PAGE_REG == ptr_fw[source_line].offset)
			{
				buf[0] = (char)(ptr_fw[source_line].val & 0x000000ff);
				gsl_write_interface(client, GSL_PAGE_REG, 1, &buf[0]);
				send_flag = 1;
			}
			else
			{
				if (1 == send_flag % (SMBUS_TRANS_LEN < 0x08 ? SMBUS_TRANS_LEN : 0x08))
					reg = ptr_fw[source_line].offset;

				memcpy(&buf[cur], &ptr_fw[source_line].val, 4);
				cur += 4;

				if (0 == send_flag % (SMBUS_TRANS_LEN < 0x08 ? SMBUS_TRANS_LEN : 0x08))
				{
					gsl_write_interface(client, reg, SMBUS_TRANS_LEN * 4, buf);
					cur = 0;
				}
				send_flag++;
			}
		}
	}
	gsl_info("<info>[go out %s]", __func__);
}

static int test_i2c(struct i2c_client *client)
{
	char read_buf = 0;
	char write_buf = 0x12;
	int ret, rc = 1;
	gsl_info("<info>[get in %s]", __func__);

	ret = gsl_read_interface(client, 0xf0, 1, &read_buf);
	if (ret < 0)
		rc--;
	else
		printk("KERN_WARNING <i2c read> [read reg : 0xf0 = 0x%x]\n", read_buf);

	msleep(2);
	ret = gsl_write_interface(client, 0xf0, 1, &write_buf);
	if (ret >= 0)
		printk("KERN_WARNING <i2c write> [write reg : 0xf0 0x12 = %x]\n");
		// printk("I write reg 0xf0 0x12\n");

	msleep(2);
	ret = gsl_read_interface(client, 0xf0, 1, &read_buf);
	if (ret < 0)
		rc--;
	else
		printk("KERN_WARNING <i2c read> [read reg : 0xf0 = 0x%x]\n", read_buf);
		// printk("I read reg 0xf0 is 0x%x\n", read_buf);
	gsl_info("<info>[go out %s]", __func__);
	return rc;
}
static void init_chip_without_rst(struct i2c_client *client)
{
	gsl_info("<info>[get in %s]", __func__);

	clr_reg(client);
	reset_chip(client);

	gsl_load_fw(client);

	startup_chip(client);

	gsl_info("<info>[go out %s]", __func__);
}

static int init_chip(struct i2c_client *client)
{
	gsl_info("<info>[get in %s]", __func__);

	tpd_gpio_output(GTP_RST_PORT, 0);
	msleep(20);
	tpd_gpio_output(GTP_RST_PORT, 1);
	msleep(20);

	init_chip_without_rst(client);
	gsl_info("<info>[go out %s]", __func__);
	return 0;
}

static void check_mem_data(struct i2c_client *client)
{
	char read_buf[4] = {0};

	msleep(30);
	gsl_read_interface(client, 0xb0, sizeof(read_buf), read_buf);

	if (read_buf[3] != 0x5a || read_buf[2] != 0x5a || read_buf[1] != 0x5a || read_buf[0] != 0x5a)
	{
		printk("#########check mem read 0xb0 = %x %x %x %x #########\n", read_buf[3], read_buf[2], read_buf[1], read_buf[0]);
		init_chip(client);
	}
}

#ifdef TPD_PROC_DEBUG
static int char_to_int(char ch)
{
	if (ch >= '0' && ch <= '9')
		return (ch - '0');
	else
		return (ch - 'a' + 10);
}

static int gsl_config_read_proc(struct seq_file *m, void *v)
{
	//char *ptr = page;
	char temp_data[5] = {0};
	unsigned int tmp = 0;
	unsigned int *ptr_fw;

	if ('v' == gsl_read[0] && 's' == gsl_read[1])
	{
#ifdef GSL_NOID_VERSION
		tmp = gsl_version_id();
#else
		tmp = 0x20121215;
#endif
		seq_printf(m, "version:%x\n", tmp);
	}
	else if ('r' == gsl_read[0] && 'e' == gsl_read[1])
	{
		if ('i' == gsl_read[3])
		{
#ifdef GSL_NOID_VERSION

			ptr_fw = g_config_data_id;

			tmp = (gsl_data_proc[5] << 8) | gsl_data_proc[4];
#endif
		}
		else
		{
			gsl_write_interface(i2c_client, 0xf0, 4, &gsl_data_proc[4]);
			if (gsl_data_proc[0] < 0x80)
				gsl_read_interface(i2c_client, gsl_data_proc[0], 4, temp_data);
			gsl_read_interface(i2c_client, gsl_data_proc[0], 4, temp_data);

			seq_printf(m, "offset : {0x%02x,0x", gsl_data_proc[0]);
			seq_printf(m, "%02x", temp_data[3]);
			seq_printf(m, "%02x", temp_data[2]);
			seq_printf(m, "%02x", temp_data[1]);
			seq_printf(m, "%02x};\n", temp_data[0]);
		}
	}
	return 0;
}
static ssize_t gsl_config_write_proc(struct file *file, const char *buffer, size_t count, loff_t *data)
{
	u8 buf[8] = {0};
	char temp_buf[CONFIG_LEN];
	char *path_buf;
	int tmp = 0;
	int tmp1 = 0;
	print_info("[tp-gsl][%s] \n", __func__);
	if (count > 512)
	{
		print_info("size not match [%d:%ld]\n", CONFIG_LEN, (long int)count);
		return -EFAULT;
	}
	path_buf = kzalloc(count, GFP_KERNEL);
	if (!path_buf)
	{
		printk("alloc path_buf memory error \n");
	}
	if (copy_from_user(path_buf, buffer, count))
	{
		print_info("copy from user fail\n");
		goto exit_write_proc_out;
	}
	memcpy(temp_buf, path_buf, (count < CONFIG_LEN ? count : CONFIG_LEN));
	print_info("[tp-gsl][%s][%s]\n", __func__, temp_buf);

	buf[3] = char_to_int(temp_buf[14]) << 4 | char_to_int(temp_buf[15]);
	buf[2] = char_to_int(temp_buf[16]) << 4 | char_to_int(temp_buf[17]);
	buf[1] = char_to_int(temp_buf[18]) << 4 | char_to_int(temp_buf[19]);
	buf[0] = char_to_int(temp_buf[20]) << 4 | char_to_int(temp_buf[21]);

	buf[7] = char_to_int(temp_buf[5]) << 4 | char_to_int(temp_buf[6]);
	buf[6] = char_to_int(temp_buf[7]) << 4 | char_to_int(temp_buf[8]);
	buf[5] = char_to_int(temp_buf[9]) << 4 | char_to_int(temp_buf[10]);
	buf[4] = char_to_int(temp_buf[11]) << 4 | char_to_int(temp_buf[12]);
	if ('v' == temp_buf[0] && 's' == temp_buf[1]) //version //vs
	{
		memcpy(gsl_read, temp_buf, 4);
		printk("gsl version\n");
	}
	else if ('s' == temp_buf[0] && 't' == temp_buf[1]) //start //st
	{
		gsl_proc_flag = 1;
		reset_chip(i2c_client);
	}
	else if ('e' == temp_buf[0] && 'n' == temp_buf[1]) //end //en
	{
		msleep(20);
		reset_chip(i2c_client);
		startup_chip(i2c_client);
		gsl_proc_flag = 0;
	}
	else if ('r' == temp_buf[0] && 'e' == temp_buf[1]) //read buf //
	{
		memcpy(gsl_read, temp_buf, 4);
		memcpy(gsl_data_proc, buf, 8);
	}
	else if ('w' == temp_buf[0] && 'r' == temp_buf[1]) //write buf
	{
		gsl_write_interface(i2c_client, buf[4], 4, buf);
	}
#ifdef GSL_NOID_VERSION
	else if ('i' == temp_buf[0] && 'd' == temp_buf[1]) //write id config //
	{
		tmp1 = (buf[7] << 24) | (buf[6] << 16) | (buf[5] << 8) | buf[4];
		tmp = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];

		if (tmp1 >= 0 && tmp1 < 256)
		{

			g_config_data_id[tmp1] = tmp;
		}
	}
#endif
exit_write_proc_out:
	kfree(path_buf);
	return count;
}

static int gsl_server_list_open(struct inode *inode, struct file *file)
{
	return single_open(file, gsl_config_read_proc, NULL);
}

static const struct file_operations gsl_seq_fops = {
	.open = gsl_server_list_open,
	.read = seq_read,
	.release = single_release,
	.write = gsl_config_write_proc,
	.owner = THIS_MODULE,
};
#endif

static void record_point(u16 x, u16 y, u8 id)
{
	u16 x_err = 0;
	u16 y_err = 0;

	id_sign[id] = id_sign[id] + 1;

	if (id_sign[id] == 1)
	{
		x_old[id] = x;
		y_old[id] = y;
	}

	x = (x_old[id] + x) / 2;
	y = (y_old[id] + y) / 2;

	if (x > x_old[id])
	{
		x_err = x - x_old[id];
	}
	else
	{
		x_err = x_old[id] - x;
	}

	if (y > y_old[id])
	{
		y_err = y - y_old[id];
	}
	else
	{
		y_err = y_old[id] - y;
	}

	if ((x_err > 3 && y_err > 1) || (x_err > 1 && y_err > 3))
	{
		x_new = x;
		x_old[id] = x;
		y_new = y;
		y_old[id] = y;
	}
	else
	{
		if (x_err > 3)
		{
			x_new = x;
			x_old[id] = x;
		}
		else
			x_new = x_old[id];
		if (y_err > 3)
		{
			y_new = y;
			y_old[id] = y;
		}
		else
			y_new = y_old[id];
	}

	if (id_sign[id] == 1)
	{
		x_new = x_old[id];
		y_new = y_old[id];
	}
}
// #endif

void tpd_down(int id, int x, int y, int p)
{

	//print_info("------tpd_down id: %d, x:%d, y:%d------ \n", id, x, y);

	elink_tp_remap(&x, &y);

	input_report_key(tpd->dev, BTN_TOUCH, 1);
	//input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id);
	input_mt_sync(tpd->dev);

	if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
	{
		if (tpd_dts_data.use_tpd_button)
			tpd_button(x, y, 1);
	}
}

void tpd_up(int id, int x, int y)
{
	print_info("------tpd_up------ \n");

	input_report_key(tpd->dev, BTN_TOUCH, 0);
	input_mt_sync(tpd->dev);

	if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
	{
		if (tpd_dts_data.use_tpd_button)
			tpd_button(x, y, 0);
	}
}

static void report_data_handle(void)
{
	char touch_data[MAX_FINGERS * 4 + 4] = {0};
	char buf[4] = {0};
	unsigned char id = 0;
	char point_num = 0;
	unsigned int x, y, temp_a, temp_b, i, tmp1;

#ifdef GSL_NOID_VERSION
	struct gsl_touch_info cinfo;
#endif

#ifdef GSL_MONITOR
	if (i2c_lock_flag != 0)
		return;
	else
		i2c_lock_flag = 1;
#endif

#ifdef TPD_PROC_DEBUG
	if (gsl_proc_flag == 1)
		return;
#endif

	gsl_read_interface(i2c_client, 0x80, 4, &touch_data[0]);
	point_num = touch_data[0];
	if (point_num > 0)
		gsl_read_interface(i2c_client, 0x84, 8, &touch_data[4]);
	if (point_num > 2)
		gsl_read_interface(i2c_client, 0x8c, 8, &touch_data[12]);
	if (point_num > 4)
		gsl_read_interface(i2c_client, 0x94, 8, &touch_data[20]);
	if (point_num > 6)
		gsl_read_interface(i2c_client, 0x9c, 8, &touch_data[28]);
	if (point_num > 8)
		gsl_read_interface(i2c_client, 0xa4, 8, &touch_data[36]);

#ifdef GSL_NOID_VERSION
	cinfo.finger_num = point_num;
	//print_info("tp-gsl  finger_num = %d\n",cinfo.finger_num);
	for (i = 0; i < (point_num < MAX_CONTACTS ? point_num : MAX_CONTACTS); i++)
	{
		temp_a = touch_data[(i + 1) * 4 + 3] & 0x0f;
		temp_b = touch_data[(i + 1) * 4 + 2];
		cinfo.x[i] = temp_a << 8 | temp_b;
		temp_a = touch_data[(i + 1) * 4 + 1];
		temp_b = touch_data[(i + 1) * 4 + 0];
		cinfo.y[i] = temp_a << 8 | temp_b;
		//print_info("tp-gsl  x = %d y = %d \n",cinfo.x[i],cinfo.y[i]);
	}
	cinfo.finger_num = (touch_data[3] << 24) | (touch_data[2] << 16) |
	                   (touch_data[1] << 8) | touch_data[0];
	gsl_alg_id_main(&cinfo);
	tmp1 = gsl_mask_tiaoping();
	print_info("[tp-gsl] tmp1=%x\n", tmp1);
	if (tmp1 > 0 && tmp1 < 0xffffffff)
	{
		buf[0] = 0xa;
		buf[1] = 0;
		buf[2] = 0;
		buf[3] = 0;
		gsl_write_interface(i2c_client, 0xf0, 4, buf);
		buf[0] = (u8)(tmp1 & 0xff);
		buf[1] = (u8)((tmp1 >> 8) & 0xff);
		buf[2] = (u8)((tmp1 >> 16) & 0xff);
		buf[3] = (u8)((tmp1 >> 24) & 0xff);
		print_info("tmp1=%08x,buf[0]=%02x,buf[1]=%02x,buf[2]=%02x,buf[3]=%02x\n",
		           tmp1, buf[0], buf[1], buf[2], buf[3]);
		gsl_write_interface(i2c_client, 0x8, 4, buf);
	}
	point_num = cinfo.finger_num;
#endif

	for (i = 1; i <= MAX_CONTACTS; i++)
	{
		if (point_num == 0)
			id_sign[i] = 0;
		id_state_flag[i] = 0;
	}
	for (i = 0; i < (point_num < MAX_FINGERS ? point_num : MAX_FINGERS); i++)
	{
#ifdef GSL_NOID_VERSION
		id = cinfo.id[i];
		x = cinfo.x[i];
		y = cinfo.y[i];
#else
		id = touch_data[(i + 1) * 4 + 3] >> 4;
		temp_a = touch_data[(i + 1) * 4 + 3] & 0x0f;
		temp_b = touch_data[(i + 1) * 4 + 2];
		x = temp_a << 8 | temp_b;
		temp_a = touch_data[(i + 1) * 4 + 1];
		temp_b = touch_data[(i + 1) * 4 + 0];
		y = temp_a << 8 | temp_b;
#endif

		if (1 <= id && id <= MAX_CONTACTS)
		{
			tpd_down(id, x_new, y_new, 10);
			id_state_flag[id] = 1;
		}
	}
	for (i = 1; i <= MAX_CONTACTS; i++)
	{
		if ((0 == point_num) || ((0 != id_state_old_flag[i]) && (0 == id_state_flag[i])))
		{
			id_sign[i] = 0;
		}
		id_state_old_flag[i] = id_state_flag[i];
	}
	if (0 == point_num)
	{
		tpd_up(id, x_new, y_new);
	}
	input_sync(tpd->dev);
#ifdef GSL_MONITOR
	i2c_lock_flag = 0;
#endif
}

#ifdef GSL_MONITOR
static void gsl_monitor_worker(struct work_struct *data)
{
	//	u8 write_buf[4] = {0};
	u8 read_buf[4] = {0};
	u8 init_chip_flag = 0;

	print_info("----------------gsl_monitor_worker-----------------\n");

	if (i2c_lock_flag != 0)
		goto queue_monitor_work;
	else
		i2c_lock_flag = 1;

	gsl_read_interface(i2c_client, 0xb0, 4, read_buf);
	if (read_buf[3] != 0x5a || read_buf[2] != 0x5a || read_buf[1] != 0x5a || read_buf[0] != 0x5a)
		b0_counter++;
	else
		b0_counter = 0;

	if (b0_counter > 1)
	{
		printk("======read 0xb0: %x %x %x %x ======\n", read_buf[3], read_buf[2], read_buf[1], read_buf[0]);
		init_chip_flag = 1;
		b0_counter = 0;
	}

	gsl_read_interface(i2c_client, 0xb4, 4, read_buf);

	int_2nd[3] = int_1st[3];
	int_2nd[2] = int_1st[2];
	int_2nd[1] = int_1st[1];
	int_2nd[0] = int_1st[0];
	int_1st[3] = read_buf[3];
	int_1st[2] = read_buf[2];
	int_1st[1] = read_buf[1];
	int_1st[0] = read_buf[0];

	if (int_1st[3] == int_2nd[3] && int_1st[2] == int_2nd[2] && int_1st[1] == int_2nd[1] && int_1st[0] == int_2nd[0])
	{
		printk("======int_1st: %x %x %x %x , int_2nd: %x %x %x %x ======\n", int_1st[3], int_1st[2], int_1st[1], int_1st[0], int_2nd[3], int_2nd[2], int_2nd[1], int_2nd[0]);
		init_chip_flag = 1;
	}

	gsl_read_interface(i2c_client, 0xbc, 4, read_buf);
	if (read_buf[3] != 0 || read_buf[2] != 0 || read_buf[1] != 0 || read_buf[0] != 0)
		bc_counter++;
	else
		bc_counter = 0;

	if (bc_counter > 1)
	{
		printk("======read 0xbc: %x %x %x %x ======\n", read_buf[3], read_buf[2], read_buf[1], read_buf[0]);
		init_chip_flag = 1;
		bc_counter = 0;
	}

	if (init_chip_flag)
		init_chip(i2c_client);

	i2c_lock_flag = 0;

queue_monitor_work:
	queue_delayed_work(gsl_monitor_workqueue, &gsl_monitor_work, 100);
}
#endif

static irqreturn_t tpd_eint_interrupt_handler(int irq, void *dev_id)
{
	TPD_DEBUG_PRINT_INT;

	eint_flag = 1;
	tpd_flag = 1;
	wake_up_interruptible(&waiter);
	return IRQ_HANDLED;
}

static int get_tp_resolution(struct i2c_client *client, u16 reg)
{
	char write_buffer[4] = {0};
	char read_buffer[4] = {0};
	int ret;

	write_buffer[0] = 0x06;
	write_buffer[1] = 0x00;
	write_buffer[2] = 0x00;
	write_buffer[3] = 0x00;

	ret = gsl_write_interface(client, 0xf0, 4, write_buffer);
	if (ret < 0)
	{
		printk("write register to get tp width or height failed\n");
	}

	msleep(10);
	gsl_read_interface(client, reg, 4, read_buffer);
	printk("read_buffer = %d %d %d %d\n", read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]);
	ret = (*(int *)read_buffer);

	return ret;
}

static void gsl_post_init(void)
{
	struct device_node *node;
	int ret = 0;

	ret = init_chip(i2c_client);
	if (ret < 0)
		printk("gsl init_chip failed!\n");

	check_mem_data(i2c_client);
	cfg_adjust(i2c_client);
	elink_tp_misc_operation(i2c_client, get_tp_resolution);

#ifdef GSL_MONITOR
	printk("gsl_post_init () : queue gsl_monitor_workqueue\n");

	INIT_DELAYED_WORK(&gsl_monitor_work, gsl_monitor_worker);
	gsl_monitor_workqueue = create_singlethread_workqueue("gsl_monitor_workqueue");
	queue_delayed_work(gsl_monitor_workqueue, &gsl_monitor_work, 1000);
#endif

	node = of_find_matching_node(NULL, tpd_of_match);
	if (node)
	{
		touch_irq_num = irq_of_parse_and_map(node, 0);
		ret = request_irq(touch_irq_num,
		                  (irq_handler_t)tpd_eint_interrupt_handler,
		                  !int_type ? IRQF_TRIGGER_RISING : IRQF_TRIGGER_FALLING,
		                  "TOUCH_PANEL-eint", NULL);
		if (ret > 0)
		{
			ret = -1;
			TPD_DMESG("tpd request_irq IRQ LINE NOT AVAILABLE!.");
		}
	}

#ifdef TPD_PROC_DEBUG
#if 0
	gsl_config_proc = create_proc_entry(GSL_CONFIG_PROC_FILE, 0666, NULL);
	printk("[tp-gsl] [%s] gsl_config_proc = %x \n", __func__, gsl_config_proc);
	if (gsl_config_proc == NULL)
	{
		print_info("create_proc_entry %s failed\n", GSL_CONFIG_PROC_FILE);
	}
	else
	{
		gsl_config_proc->read_proc = gsl_config_read_proc;
		gsl_config_proc->write_proc = gsl_config_write_proc;
	}
#else
	proc_create(GSL_CONFIG_PROC_FILE, 0666, NULL, &gsl_seq_fops);
#endif
	gsl_proc_flag = 0;
#endif

#if 0
	init_timer(&led_timer);
	led_timer.expires	= jiffies + 1 / (1000 / HZ);
	led_timer.function	= timer_func;
#endif
}
/**
 *  touch panel event handler  function
 *  1. init chip
 *
 *
 */
static int touch_event_handler(void *unused)
{
	struct sched_param param = {.sched_priority = 4};
	// schedule set
	sched_setscheduler(current, SCHED_RR, &param);
	// 芯片初始化，申请中断
	gsl_post_init();
	// 循环等待事件处理
	do
	{
		set_current_state(TASK_INTERRUPTIBLE);
		// modified by elink_phil@20170616 start <<<
		if (tpd_flag == 0)
		{
			DEFINE_WAIT(wait);
			prepare_to_wait(&waiter, &wait, TASK_INTERRUPTIBLE);
			// 让出当前活动进程。此次不执行
			schedule();
			finish_wait(&waiter, &wait);
		}
		// wait_event_interruptible(waiter, tpd_flag != 0);
		// modified end >>>
		tpd_flag = 0;
		TPD_DEBUG_SET_TIME;
		set_current_state(TASK_RUNNING);
		//print_info("===touch_event_handler, task running===\n");

		eint_flag = 0;
		report_data_handle();

	} while (!kthread_should_stop()); // 保证线程在做完事后退出

	return 0;
}

static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	strcpy(info->type, TPD_DEVICE);
	return 0;
}

static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;

	printk("%s\n", __func__);

	i2c_client = client;

	tpd_gpio_output(GTP_RST_PORT, 0);
	msleep(100);

	tpd_gpio_output(GTP_RST_PORT, 1);

	tpd_gpio_output(GTP_INT_PORT, 0);
	tpd_gpio_as_int(GTP_INT_PORT);
	msleep(50);

	mutex_init(&gsl_i2c_lock);

	if (test_i2c(client) < 0)
	{
		printk("%s: i2c test error, so quit!!!\n", __func__);
		return -ENODEV;
	}

	// 创建 TPD_DEVICE 线程，并启动, 执行 touch_event_handler
	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	if (IS_ERR(thread))
	{
		err = PTR_ERR(thread);
		TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", err);
	}

	printk("%s successfully\n", __func__);

	tpd_load_status = 1;

	return 0;
}

static int tpd_i2c_remove(struct i2c_client *client)
{
	printk("==tpd_i2c_remove==\n");

	return 0;
}

static const struct i2c_device_id tpd_i2c_id[] = {{TPD_DEVICE, 0}, {}};

struct i2c_driver tpd_i2c_driver = {
	.driver = {
		.name = TPD_DEVICE,
		.owner = THIS_MODULE,
		.of_match_table = tpd_of_match,
	},
	.probe = tpd_i2c_probe,
	.remove = tpd_i2c_remove,
	.id_table = tpd_i2c_id,
	.detect = tpd_i2c_detect,
};

int tpd_local_init(void)
{
	printk("%s\n", __func__);

	init_tp_power(tpd->tpd_dev, TP_POWER);
	tp_power_on(1);

	if (i2c_add_driver(&tpd_i2c_driver) != 0)
	{
		printk("%s() unable to add i2c driver.\n", __func__);
		return -1;
	}

	if (tpd_load_status == 0)
	{
		printk("add error touch panel driver.\n");
		i2c_del_driver(&tpd_i2c_driver);
		return -1;
	}

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
	TPD_DO_WARP = 1;
	memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT * 4);
	memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT * 4);
#endif

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
	memcpy(tpd_calmat, tpd_calmat_local, 8 * 4);
	memcpy(tpd_def_calmat, tpd_def_calmat_local, 8 * 4);
#endif
	tpd_type_cap = 1;

	printk("%s successfully\n", __func__);
	return 0;
}

/* Function to manage low power suspend */
static void tpd_suspend(struct device *h)
{
	printk("==tpd_suspend==\n");

#ifdef TPD_PROC_DEBUG
	if (gsl_proc_flag == 1)
	{
		return;
	}
#endif

	tpd_halt = 1;
	//mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	disable_irq(touch_irq_num);
#ifdef GSL_MONITOR
	printk("gsl_ts_suspend () : cancel gsl_monitor_work\n");
	cancel_delayed_work_sync(&gsl_monitor_work);
#endif

	tpd_gpio_output(GTP_RST_PORT, 0);
}

/* Function to manage power-on resume */
static void tpd_resume(struct device *h)
{
	printk("==tpd_resume==\n");
#ifdef TPD_PROC_DEBUG
	if (gsl_proc_flag == 1)
	{
		return;
	}
#endif

	tpd_gpio_output(GTP_RST_PORT, 1);
	msleep(30);
	reset_chip(i2c_client);
	startup_chip(i2c_client);
	check_mem_data(i2c_client);

#ifdef GSL_MONITOR
	printk("gsl_ts_resume () : queue gsl_monitor_work\n");
	queue_delayed_work(gsl_monitor_workqueue, &gsl_monitor_work, 300);
#endif

	enable_irq(touch_irq_num);
	tpd_halt = 0;
}

static struct tpd_driver_t tpd_device_driver = {
	.tpd_device_name = GSLX680_NAME,
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
};

/* called when loaded into kernel */
static int __init tpd_driver_init(void)
{
	printk("Sileadinc gslX680 touch panel driver init\n");
	gsl_info("<info>[get in %s]", __func__);


	// added by elink_phil@20161021 for when in power off charging mode, don't load tp driver start >>>
	if (KERNEL_POWER_OFF_CHARGING_BOOT == get_boot_mode() || LOW_POWER_OFF_CHARGING_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
	{
		printk("CHARGING_BOOT!!! don't need tp driver\n");
		return -EPERM;
	}
	// added end <<<

	tpd_get_dts_info();
	// 将 tpd_device_driver 加入到 结构体数组 tpd_driver_list[] 中
	if (tpd_driver_add(&tpd_device_driver) < 0)
		printk("add gslX680 driver failed\n");

	gsl_info("<info>[go out %s]", __func__);
	return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void)
{
	printk("Sileadinc gslX680 touch panel driver exit\n");

	tpd_driver_remove(&tpd_device_driver);
}

#endif


