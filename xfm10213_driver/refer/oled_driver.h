#ifndef __OLED_DRIVER_H
#define __OLED_DRIVER_H

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/of.h>                       /* For DT*/
#include <linux/fs.h>                       // ioctl and file_operations support
#include <linux/gpio.h>                     /* For Legacy integer based GPIO */
#include <linux/device.h>

#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/of_gpio.h>                  /* For of_gpio* functions */
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinctrl-state.h>
#include <linux/platform_device.h>          /* For platform devices */
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
// #include <linux/proc.h>
#include <linux/time.h>	// for get system current time
#include <linux/proc_fs.h>
// #include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fb.h>
#include <linux/err.h>
#include <linux/timer.h>        //for timer_list, jiffy timer, standard timer
#include <linux/i2c.h>



#define OLED_DEBUG					 0
#define HRTIMER_DEFINE               1
#define I2C_16_BIT_MODE				 0
#define I2C_CHECK_SELF               1

#define READ_FLAG			1
#define WRITE_FLAG			0


#define TAG                         " <OLED> "
#define USR_MSG_LEVEL               KERN_ERR
#define USR_ERR_LEVEL               KERN_ERR
#define usr_msg(fmt, args...)       printk(USR_MSG_LEVEL TAG ""fmt"\n", ##args)
#define err_msg(fmt, args...)       printk(USR_ERR_LEVEL TAG " (function : %s), [line : %d] "fmt"\n",__func__, __LINE__, ##args)


#define    OLED_DEV_NAME         "oled_ssd1306"


typedef enum {
    low     = 0,
    high    = 1
}status;


void gpio_set_state(status state);
int struct_mallock_init(void);
void struct_mallock_free(void);
int oled_get_i2c_dts_info(struct device *oled_dev);
int register_oled_driver(void);

int oled_i2c_send_byte(struct i2c_client *client, unsigned char sub_addr, unsigned char data);
int oled_i2c_send_matrix(struct i2c_client *client, unsigned char sub_addr, unsigned char *data, unsigned int length);
void waitqueue_init(void);

/* extern function declartion ------------------------------------------------*/
extern void oled_power_on(void);
extern void oled_init(void);
extern void oled_test(void);
extern void oled_fill(unsigned char data);

/* oled device information struction ---------------------------------------*/
struct _oled_device_info {
    unsigned int    oled_major;
    dev_t           oled_devno;
    struct cdev     oled_cdev;
    struct class    * oled_class;
    struct device   * oled_device;
    struct mutex    oled_dev_lock;
};
extern struct _oled_device_info * oled_dev_info;
// EXPORT_SYMBOL(oled_dev_info);
/* oled dts information struction ---------------------------------------*/
struct _oled_dts_info {
	struct device_node 	       	*node;
	unsigned int 				oled_rst_pin;
	unsigned int 		        oled_width;
	unsigned int 		        oled_height;
	unsigned int 		        oled_i2c_clk;

	struct pinctrl 		        *dev_pinctrl;
	struct pinctrl_state 		*rst_high;
	struct pinctrl_state 		*rst_low;
};
extern struct _oled_dts_info * oled_dts_info;
// EXPORT_SYMBOL(oled_dts_info);
/* oled i2c information struction ---------------------------------------*/
struct _oled_i2c_info {
	struct i2c_client 	* oled_client;
	char 				* data;
    struct mutex        i2c_lock;
    struct i2c_msg		oled_i2c_msg;
};

extern struct _oled_i2c_info * oled_i2c_info;
// EXPORT_SYMBOL(oled_i2c_info);
/* oled whole information struction ---------------------------------------*/


#if HRTIMER_DEFINE
    void oled_timer_init(unsigned long     ticks);
#endif

#endif
