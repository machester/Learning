/*----------------------------------------------------------------------*
 * Data : 2018-10-07
 * using struct cdev to register char device
 * creat class for device
 * fill read, write, ioctl, close
 *
 *
 *----------------------------------------------------------------------*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/printk.h>
#include <linux/major.h>
#include <linux/device.h>

/*--------------------- macro defined start ---------------------*/
#define usr_msg(args)                               \
        do                                          \
        {                                           \
                printk(KERN_ERR "---> %s\n", args); \
        } while (0)

#define err_msg(args)                                   \
        do                                              \
        {                                               \
                printk(KERN_ERR "-------> %s\n", args); \
        } while (0)
/*--------------------- macro defined end ----------------------*/
static int __init input_drv_init(void)
{
	for (int i = 0; i < 100; i++) {
		i++;
	}
}
static void __exit input_drv_exit(void)
{

}

module_init("input_drv_init");
module_exit("input_drv_exit");