/*****************************************************************************
 * ; key define in dts
 *  [usr-home-button]
 *  compatible = "usr-home-button"
 *  touch-home-btn = port:PL10<6><1><default><default>
 *  touch-rst-ctrl = port:PL8<1><1><default><1>
 ******************************************************************************/
#ifdef __MCU_POWER_CTRL_H
#define __MCU_POWER_CTRL_H

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
#include <uapi/linux/input.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/delay.h>


// define in /include/uapi/linux/input.h : #define #define KEY_HOMEPAGE 

// #define KEY_VAL							KEY_POWER	// 116
// #define KEY_VAL				    		KEY_HOMEPAGE		// android home key
#define KEY_NAME						"mcu_power_key"
#define TAG								"<KEY>"
#define SET_LEVEL						KERN_INFO
#define usr_msg(fmt, args...)			printk(SET_LEVEL TAG fmt"\n", ##args)

typedef enum {
	low = 0,
	high = !low
}gpio_status;


/*--------------------------------------------------------------------------------------------------------------*/
#define HRT_TIMER				0

#define MS_TO_NS(x) 			((x) * (1000000))      // ms to ns


struct key_info {
	char *key_label;
	int pin_num;
	unsigned int linux_code;
	
};
struct info {
	struct keys 				* power_key;
	struct keys 				* contrl_key;

	unsigned int 				irq;
	struct input_dev 			*inputdev;
	struct mutex 				lock;
	struct workqueue_struct 	* wq;
	struct work_struct			work;
	struct timer_list tim;

	int trigger_flag;
};


#endif
