/**********************************************************************************
    xfm10213@3e {
		compatible = "xfm10213";
		status = "okay";
		reg = <0x3e>;
        xfm10213-irq-gpio = <&gpio5 RK_PB2 GPIO_ACTIVE_HIGH>;
        xfm10213-irq-trigger-level = "high";
	};
***********************************************************************************/
#ifndef XFM10213_H
#define XFM10213_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

#define XFM10213_DEV_NAME				"xfm10213"

#define TAG								"<XFM10213> "
#define SET_LEVEL						KERN_INFO
#define usr_msg(fmt, args...)			printk(SET_LEVEL TAG fmt"\n", ##args)

#define 	XFM10213_ADDR           	(0x3E) // 7bit mode
// #define 	XFM10213_ADDR				(0x7C)	// 8bit wirte address
// #define 	XFM10213_ADDR				(0x7D)	// 8bit read address

/** start: I2S configuration register --------------------------------------------------*/
#define 	I2S_CONFIG_REG 	 			(0x011B)
#define 	I2S_MASTER_MODE				(0x0000)
#define 	I2S_SLAVE_MODE				(0x0011)
/** end: I2S configuration register -----------------------------------------------------*/
/** start: UART speed configuration register --------------------------------------------*/
#define 	UART_SPEED_CONFIG_REG 	 	(0x011D)
#define 	UART_SPEED_9600				(0x0060)
#define 	UART_SPEED_115200			(0x00C0)
#define 	UART_SPEED_256000			(0x0480)
/** end: UART speed configuration register -----------------------------------------------*/
/** start: UART priority configuration register ------------------------------------------*/
#define 	UART_PRINT_CTRL_REG 	 	(0x011C)
#define 	UART_PRINT_CORE_MSG 	 	(0x0000)
#define 	UART_PRINT_ERR_MSG			(0x0001)
#define 	UART_PRINT_HMI_MSG			(0x0002)
#define 	UART_PRINT_ALL_MSG			(0x0003)
/** end: UART priority configuration register ---------------------------------------------*/
/** start: MIC distance configuration register --------------------------------------------*/
#define 	MIC_DISTANCE_REG 	 		(0x0116)
#define 	MIC_DISTANCE_MIN			(0x0014)
#define 	MIC_DISTANCE_MAX			(0x0078)
/** end: MIC distance configuration register -----------------------------------------------*/
/** start: MIC output gain configuration register ------------------------------------------*/
#define 	MIC_OUT_GAIN_REG 	 		(0x011A)
/** end: MIC output gain configuration register --------------------------------------------*/
/** start: MIC wake configuration register -------------------------------------------------*/
#define 	MIC_WAKEUP_REG 	 			(0x0113)
/** end: MIC wake configuration register ---------------------------------------------------*/
/** start: voice channel words register ----------------------------------------------------*/
#define 	VOI_CHANNEL_REG 	 		(0x0111)
/** end: voice channel words register ------------------------------------------------------*/

/** start: xfm10213 work mode configuration register ------------------------------------------*/
#define 	VOI_WORK_MODE_REG 	 		(0x0110)
#define 	VOI_WORK_MODE_BASE			(0x0000)
#define 	VOI_STOP					(VOI_WORK_MODE_BASE | (0 << 0))
#define 	VOI_WORK_WAKEUP				(VOI_WORK_MODE_BASE | (2 << 0))
#define 	VOI_WORK_CAE				(VOI_WORK_MODE_BASE | (3 << 0))
#define 	VOI_WORK_MUTEX				(VOI_WORK_MODE_BASE | (4 << 0))
#define 	VOI_WORK_DIRECTION			(VOI_WORK_MODE_BASE | (5 << 0))
#define 	VOI_WORK_TEST_MODE			(VOI_WORK_MODE_BASE | (6 << 0))
/** end: xfm10213 work mode configuration register --------------------------------------------*/

/** start: xfm10213 firware build timer check register ------------------------------------------*/
#define 	VOI_FIRMWARE_DATE_CHECK_REG 	 		(0x010E)		// read only
#define 	VOI_FIRMWARE_TIME_CHECK_REG 	 		(0x010F)		// read only
/** end: xfm10213 firware build timer check register --------------------------------------------*/
#define 	VOI_CAE_VERSION_CHECK					(0x010D)
#define 	VOI_IVW_VERSION_CHECK					(0x010C)
#define 	VOI_SYS_VERSION_CHECK					(0x010A)

typedef enum {
	low = 0,
	high = !low
} gpio_status;
	
typedef enum {
	in = 0,
	out = !in
}gpio_dir;
	
typedef enum {
	master = 0,
	slave = !master
} xfm10213_work_mode;

struct xfm10213_info {
	int irq_pin;
	int reset_pin;
	int irq_num;
	int irq_trigger_level;
	int xfm10213_upgrade_flag;
	int init_flag;
	
	xfm10213_work_mode mode;
	
	unsigned char 	addr;
	struct mutex 	lock;
	
	struct i2c_client * client;
	struct device_node * node;
	struct proc_dir_entry * xfm10213_proc_entry;
	struct work_struct * xfm10213_upgrade_work;
	struct delayed_work * xfm10213_init_work;
};


unsigned int xfm10213_read_16bit(struct i2c_client *client, unsigned int sub_addr);
int xfm10213_send_16bit(struct i2c_client *client, unsigned int sub_addr, unsigned int data);
// int xfm10213_send_str(struct i2c_client *client, unsigned int sub_addr, unsigned int data, unsigned int size);
xfm10213_work_mode xfm10213_get_current_work_mode(struct xfm10213_info);
int xfm10213_set_to_mode(struct xfm10213_info * info, xfm10213_work_mode mode);
void xfm10213_upgrade_work_init(struct xfm10213_info * info);
void xfm10213_upgrade_work_remove(struct xfm10213_info * info);
// int xfm10213_proc_create(struct xfm10213_info * info);
// int xfm10213_proc_remove(struct xfm10213_info * info);
int xfm10213_chip_init(struct xfm10213_info * info);
int xfm10213_set_value(int gpio, gpio_dir dir, gpio_status value);

#endif
