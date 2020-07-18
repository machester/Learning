#ifndef __SL3S4011_H
#define __SL3S4011_H

/*****************************************
sl3s4011@51 {
	 compatible = "mediatek,rfid_sl3s4011";
	 status = "okay";
	 reg = <0x51>;
	 report_time = <1000>;	 // report invertion microseconds
 };

******************************************/

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
#include <linux/sysfs.h>
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
#include <linux/time.h>	// for get system current time
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fb.h>
#include <linux/err.h>
#include <linux/timer.h>        //for timer_list, jiffy timer, standard timer
#include <linux/i2c.h>
#include <linux/kthread.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
// #include <linux/init-input.h>

#include <linux/bcd.h>
#include <linux/list.h>
#include <linux/rtc.h>
#include <linux/delay.h>
#include <linux/fs.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>


#define   S4011_WRITE   		((unsigned char)0xA2)
#define   S4011_READ    		((unsigned char)0xA3)
#define   S0411_ADDRESS_7BIT	((unsigned char)0xA3 >> 1)

/*------------------------------*
* macros for s4011 base address *
*-------------------------------*/

#define S4011_EPC_START_ADDR       		0x2004
#define S4011_TID_START_ADDR  			0x4000
#define S4011_USR_START_ADDR      		0x6000


#define S4011_EPC_END_ADDR				0x2016
#define S4011_TID_END_ADDR				0x400A
#define S4011_USR_END_ADDR      		0x619E

#define S4011_EPC_OffSET_ADRESS	    	(2)
#define S4011_TID_OffSET_ADRESS	    	(2)
#define S4011_USR_OffSET_ADRESS	    	(2)


#define S4011_EPC_WORDS_LEN	(((S4011_EPC_END_ADDR - S4011_EPC_START_ADDR) / S4011_EPC_OffSET_ADRESS) + 1)
#define S4011_TID_WORDS_LEN	(((S4011_TID_END_ADDR - S4011_TID_START_ADDR) / S4011_TID_OffSET_ADRESS) + 1)
#define S4011_USR_WORDS_LEN	(((S4011_USR_END_ADDR - S4011_USR_START_ADDR) / S4011_USR_OffSET_ADRESS) + 1)

#endif

