/* oled whole information struction ---------------------------------------*/

#include "oled_driver.h"

#ifndef CONFIG_OF
    #define CONFIG_OF
#endif

#define OLED_DRIVER_VERSION             "0.1"

struct _oled_device_info    * oled_dev_info;
struct _oled_dts_info       * oled_dts_info;
struct _oled_i2c_info       * oled_i2c_info;

/* of_device_id oled_i2c_of_match ---------------------------------------------------*/
static const struct of_device_id oled_i2c_of_match[] = {
	{.compatible = "oled,ssd1306"},
	{/* keep this */}
};
MODULE_DEVICE_TABLE(of, oled_i2c_of_match);

/* hrt timer start -------------------------------------------------------------------------*/
#if HRTIMER_DEFINE

#define MS_TO_NS(x)         (x * 1000000)      // ms to ns

struct hrtimer  oled_timer;
ktime_t         tm_period;

static enum hrtimer_restart loop_hrtimer_callback(struct hrtimer * arg)
{
	static int loop_counter = 0;

    ktime_t now = arg->base->get_time();
    usr_msg("loop counter = %d, timer : jiffies=%ld\n",loop_counter, jiffies);
    hrtimer_forward(arg, now, tm_period);

    if(0 == loop_counter % 2)
		gpio_set_state(high);
    else
		gpio_set_state(low);

	loop_counter++;
    return HRTIMER_RESTART;
}

/**
 * [oled_timer_init description]
 * @param ticks microsencod
 */

void oled_timer_init(unsigned long     ticks)
{
    mutex_lock(&oled_dev_info->oled_dev_lock);
    // ktime_set(const s64 secs, const unsigned long nsecs); // param1: second, param2:nanosecond
    tm_period = ktime_set(0, MS_TO_NS(ticks));     // set 1second, 1000 nanosecond.
    hrtimer_init(&oled_timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    oled_timer.function = loop_hrtimer_callback;
    hrtimer_start(&oled_timer, tm_period, HRTIMER_MODE_REL);
    mutex_unlock(&oled_dev_info->oled_dev_lock);
}
#endif
/* hrt timer end -------------------------------------------------------------------------*/
/* waitqueue -----------------------------------------------------------------------------*/
static DECLARE_WAIT_QUEUE_HEAD(blink_waitqueue_head);
// static struct wait_queue_head_t     blink_waitqueue_head;     // not recommended definition
static struct work_struct   blink_work;
static int waitqueue_flag = 1;

static void blink_reacall_function(struct work_struct *work)
{

    wake_up_interruptible(&blink_waitqueue_head);
    usr_msg("queue woke-up, waitqueue_flag = %d", waitqueue_flag);
    // reschedule 10 times
    waitqueue_flag += 1;
    oled_power_on();
    oled_fill(0xff);
    schedule_work(&blink_work);
    if(2 == waitqueue_flag){
        waitqueue_flag = 0;
        usr_msg("disable wake_up_interruptible, waitqueue_flag = %d", waitqueue_flag);
        // after 10 times schedule_work, program will be suspend
        usr_msg("---------> oled init loop");
    }
}

void waitqueue_init(void)
{
    init_waitqueue_head(&blink_waitqueue_head);
    INIT_WORK(&blink_work, blink_reacall_function);
    schedule_work(&blink_work);
    wait_event_interruptible(blink_waitqueue_head, waitqueue_flag != 0);
    usr_msg("waitqueue, init codition =  %d", waitqueue_flag);
}

/* i2c fileoperations start -------------------------------------------------------*/
static int oled_i2c_open(struct inode *inode, struct file *filp)
{
    usr_msg("oled proc : oled_i2c_open");
    return 0;
}

static ssize_t oled_i2c_read(struct file *flip, char __user *buff,
                            size_t counter, loff_t *fops)
{
    usr_msg("oled proc : oled_i2c_read");
    return 0;
}
static ssize_t oled_i2c_write(struct file *flip, const char __user *buff,
                                    size_t counter, loff_t *fops)
{
    int value;
    int ret;
    int index;
    usr_msg("oled proc : oled_i2c_write");

    if (counter < 0 || counter > 4)
        return -EINVAL;
    ret = copy_from_user(&value, buff, counter);
    if (ret > 0) {
        err_msg("copy_from_user error");
        return -EFAULT;
    }
    usr_msg("start to switch copied data.");
    switch (value) {
    case 0:
        usr_msg("value is : 0");
        gpio_set_state(low);
    break;
    case 1:
        usr_msg("value is : 1");
        gpio_set_state(high);
    break;
    case 2:
        usr_msg("value is : 2");
        for(index = 0; index < 3; index++) {
            gpio_set_state(low);
            mdelay(500);
            gpio_set_state(high);
            mdelay(500);
        }
    break;
    default:
        usr_msg("Out of handle range");
    }

    return 0;
}
static int oled_i2c_release(struct inode *inode, struct file *filp)
{
    usr_msg("oled proc : oled_i2c_release");
    return 0;
}
static long oled_i2c_ioctl(struct file *flip, unsigned int cmd,
                            unsigned long param)
{
    usr_msg("oled proc : oled_i2c_ioctl");
    return 0;
}
/* i2c fileoperations end -------------------------------------------------------*/

/* i2c transmit function start -------------------------------------------------------*/
/**
 * [oled_i2c_send_byte description, 8bit mode]
 * @param  addr :write sub-address
 * @param  data :write data
 * @return      [description]
 */
#if 0
int oled_i2c_send_byte(struct i2c_client *client, unsigned char sub_addr, unsigned char data)
{
    int ret;
    struct i2c_msg	msg;

    unsigned char 	temp[2] = {sub_addr, data};

    temp[0] = sub_addr;
    temp[1] = data;

	msg.addr 	= client->addr;
    msg.len		= 2;		// write = sub-address + data
	msg.flags 	= WRITE_FLAG;
	msg.buf 	= temp;

    mutex_lock(&oled_i2c_info->i2c_lock);
    ret = i2c_transfer(client->adapter, &msg, 1);
    mutex_unlock(&oled_i2c_info->i2c_lock);
    usr_msg("---> sub_addr = 0x%x, data = 0x%x", sub_addr, data);
    memset(temp, 0, sizeof(temp));
    return ret;
}
#else
int oled_i2c_send_byte(struct i2c_client *client, unsigned char sub_addr, unsigned char data)
{
	int ret;
	unsigned short send_mark;
    
#if 1

    struct i2c_msg msg;
	struct i2c_adapter *adap=client->adapter;
	char msg_buff[]; = {sub_addr, data};

	send_mark = 0;
    msg.addr = client->addr;
    msg.flags = client->flags & send_mark;
    msg.len = 2;
    msg.buf = msg_buff;
	
    ret = i2c_transfer(adap, &msg, 1); 
    //kfree(msg_buff);
	
#else
    char buff[2] = {sub_addr, data};
    usr_msg("ready to sent client_addr = 0x%x, sub_addr = 0x%x, data = 0x%x", client->addr, sub_addr, data);
	
    ret = i2c_master_send(client, buff, 2);
#endif
    return ret;
}
#endif


int oled_i2c_send_matrix(struct i2c_client * client, unsigned char sub_addr, unsigned char *data, unsigned int length)
{
	int ret;
    unsigned char   tmp[length + 1];
    struct i2c_msg  msg[1];

	// 8-bit address mode
	tmp[0] = sub_addr;
	memcpy (tmp + 1, data, length);

    msg[0].addr = client->addr;
    msg[0].flags = 0;                    /* Write */
    msg[0].len = length + 1; /* Address is 1 bytes coded */
    msg[0].buf = tmp;
    mutex_lock(&oled_i2c_info->i2c_lock);
    ret = i2c_transfer(client->adapter , msg, ARRAY_SIZE(msg));
    mutex_unlock(&oled_i2c_info->i2c_lock);
	return ((ret > 0) ? ret : -ENOMSG);
}
/* i2c transmit function end ----------------------------------------------------------*/

static struct file_operations oled_i2c_fops = {
	.open 			= oled_i2c_open,
	.write			= oled_i2c_write,
	.read 			= oled_i2c_read,
	.release 		= oled_i2c_release,
	.unlocked_ioctl = oled_i2c_ioctl,
};

static int oled_i2c_probe(struct i2c_client * client, const struct i2c_device_id *i2c_id)
{
    int ret;

#if I2C_CHECK_SELF
    usr_msg("---> move in");
    if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err_msg("i2c check failed");
		return -ENODEV;
    }
#endif

	usr_msg("detected slaver adapter address = 0x%x", client->addr);
    usr_msg("detected slaver adapter (address << 1) = 0x%x", (client->addr << 1));

	ret = struct_mallock_init();
	if(ret < 0) {
		err_msg("error : malloc.");
		return -ENOMEM;
	}
    ret = oled_get_i2c_dts_info(&client->dev);
	if(ret < 0) {
		err_msg("error : get i2c device tree info");
		ret = -ENODEV;
		goto err_get_dts;
	}

	oled_i2c_info->oled_client = client;
    mutex_init(&oled_i2c_info->i2c_lock);
    // no need for global prameter
    // i2c_set_clientdata(client, oled_i2c_info);
	ret = register_oled_driver();
	if(ret < 0) {
		err_msg("error : oled driver register.");
		return ret;
	}

    usr_msg("---> waitqueue_init");
	waitqueue_init();
#if HRTIMER_DEFINE
    // set tick counter
    oled_timer_init(2000);
#endif

    return ret;

err_get_dts:
	struct_mallock_free();
	return ret;
}


static int oled_i2c_remove(struct i2c_client *client)
{
#if HRTIMER_DEFINE
    int retval;
    retval = hrtimer_cancel(&oled_timer);
    if (retval)
        err_msg("The timer is still in use...\n");
    usr_msg("timer_jiffy exit success");
#endif
    usr_msg("i2c driver remove");
    mutex_destroy(&oled_i2c_info->i2c_lock);
    gpio_free(oled_dts_info->oled_rst_pin);
    device_destroy(oled_dev_info->oled_class, oled_dev_info->oled_devno);
    class_destroy(oled_dev_info->oled_class);
    cdev_del(&oled_dev_info->oled_cdev);
    unregister_chrdev(oled_dev_info->oled_major, OLED_DEV_NAME);
    struct_mallock_free();


    return 0;
}




int oled_get_i2c_dts_info(struct device *oled_dev)
{

	int ret, index;

	// get dts information
#if 0
	if(!oled_node) {
		err_msg("error for oled_node pointer transfer");
		return -ENOMSG;
	}
#endif

    // oled_dts_info->node = of_find_matching_node(NULL, oled_i2c_of_match);
	oled_dts_info->node = oled_dev->of_node;
	if(!oled_dts_info->node) {
		err_msg("error : cannot find oled_i2c_of_match");
		return -ENODEV;
	}
	of_property_read_u32(oled_dts_info->node, "width", &oled_dts_info->oled_width);
	usr_msg("---> width = %d", oled_dts_info->oled_width);
	of_property_read_u32(oled_dts_info->node, "height", &oled_dts_info->oled_height);
	usr_msg("---> height = %d", oled_dts_info->oled_height);

	oled_dts_info->oled_rst_pin = of_get_named_gpio(oled_dts_info->node, "oled-rst", 0);
	if (!gpio_is_valid(oled_dts_info->oled_rst_pin)) {
        err_msg("error : oled-rst not valid");
        ret = -EIO;
		goto dts_out;
	}
    usr_msg("---> gpio = %d", oled_dts_info->oled_rst_pin);
	mutex_lock(&oled_dev_info->oled_dev_lock);
	ret = devm_gpio_request_one(oled_dev, oled_dts_info->oled_rst_pin, GPIOF_OUT_INIT_HIGH, "oled-rst");
	if(ret < 0) {
	    printk(KERN_ERR "---> gpio  %d request error\n", oled_dts_info->oled_rst_pin);
	    gpio_free(oled_dts_info->oled_rst_pin);
	    ret = -EBUSY;
		goto dts_out;
	}
	// set default
	gpio_direction_output(oled_dts_info->oled_rst_pin, 0);
    mutex_unlock(&oled_dev_info->oled_dev_lock);

    for (index = 0; index < 3; index ++) {
        gpio_set_state(high);
        mdelay(1000);
        gpio_set_state(low);
        mdelay(1000);
    }

	usr_msg("gpio request and set ok");
	return ret;

dts_out:
	return ret;
}



int struct_mallock_init(void)
{
	int ret = 0;

	usr_msg("struct malloc and initilaziton.");
	oled_dts_info = kmalloc(sizeof(struct _oled_dts_info), GFP_KERNEL);
	if(IS_ERR(oled_dts_info)) {
		err_msg("malloc struct <oled_dts_info> error");
		ret = -ENOMEM;
		return ret;
	}
	oled_dev_info = kmalloc(sizeof(struct _oled_device_info), GFP_KERNEL);
	if(IS_ERR(oled_dev_info)) {
		err_msg("malloc struct <oled_dev_info> error");
		ret = -ENOMEM;
		goto err_oled_dev_info;
	}
	oled_i2c_info = kmalloc(sizeof(struct _oled_i2c_info), GFP_KERNEL);
	if(IS_ERR(oled_i2c_info)) {
		err_msg("malloc struct <oled_i2c_info> error");
		ret = -ENOMEM;
		goto err_oled_i2c_info;
	}
    mutex_init(&oled_dev_info->oled_dev_lock);

	return ret;

err_oled_i2c_info:
	kfree(oled_dev_info);
err_oled_dev_info:
	kfree(oled_dts_info);

	return ret;

}

void struct_mallock_free(void)
{
	kfree(oled_dev_info);
	kfree(oled_dts_info);
	kfree(oled_i2c_info);
}

int register_oled_driver(void)
{
	int ret = 0;
	ret = alloc_chrdev_region(&oled_dev_info->oled_devno , 0, 1, OLED_DEV_NAME);
	if(ret < 0) {
		err_msg("error : alloc_chrdev_region");
		ret = -ENODEV;
		goto out;
	}
	oled_dev_info->oled_major = MAJOR(oled_dev_info->oled_devno);
	usr_msg("---> dev_info->oled_major = %d", oled_dev_info->oled_major);
#if 0
	oled_dev_info->oled_cdev = cdev_alloc();
	if(IS_ERR(oled_dev_info->oled_cdev)) {
		err_msg("error : cdev_alloc");
		goto err_cdev_alloc;
	}
#endif
	cdev_init(&oled_dev_info->oled_cdev, &oled_i2c_fops);
	ret = cdev_add(&oled_dev_info->oled_cdev , oled_dev_info->oled_devno, 1);
	if(ret < 0) {
		err_msg("error : cdev_add.");
		goto err_cdev_add;
	}

	oled_dev_info->oled_class = class_create(THIS_MODULE, OLED_DEV_NAME);
    if (IS_ERR(oled_dev_info->oled_class)) {
        err_msg("error : class create.");
        ret = PTR_ERR(oled_dev_info->oled_class);
        goto err_cls_create;
    }
    oled_dev_info->oled_device = device_create(oled_dev_info->oled_class, NULL,
                                                oled_dev_info->oled_devno,
                                                NULL, "%s", OLED_DEV_NAME);
    if(IS_ERR(oled_dev_info->oled_device)) {
        err_msg("error : device_create.");
        ret = PTR_ERR(oled_dev_info->oled_device);
        goto err_dev_create;
    }
    return ret;

err_dev_create:
    class_destroy(oled_dev_info->oled_class);
err_cls_create:
	cdev_del(&oled_dev_info->oled_cdev);
err_cdev_add:
	unregister_chrdev_region(oled_dev_info->oled_devno, 1);
out:
	return ret;
}

void gpio_set_state(status state)
{
    mutex_lock(&oled_dev_info->oled_dev_lock);
    if(low == state) {
        gpio_set_value(oled_dts_info->oled_rst_pin, 0);

    } else {
        gpio_set_value(oled_dts_info->oled_rst_pin, 1);
    }
    mutex_unlock(&oled_dev_info->oled_dev_lock);
}

static const struct i2c_device_id oled_device_id[] = {
    {"oled,ssd1306", 0},
    {/*keep this*/}
};
MODULE_DEVICE_TABLE(i2c, oled_device_id);

static struct i2c_driver oled_i2c_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = OLED_DEV_NAME,
        .of_match_table = of_match_ptr(oled_i2c_of_match),
    },
    .probe = oled_i2c_probe,
    .remove = oled_i2c_remove,
    .id_table = oled_device_id,
};

module_i2c_driver(oled_i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("quan, <shusheng1991@gmail.com>");
MODULE_ALIAS("i2c :" OLED_DEV_NAME);
MODULE_DESCRIPTION("OLED Driver, control chip SSD1306");
MODULE_VERSION(OLED_DRIVER_VERSION);
