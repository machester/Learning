/* oled whole information struction ---------------------------------------*/

#include "oled_driver.h"


#define OLED_DRIVER_VERSION         "0.1"



static struct of_device_id oled_dev_of_match[] = {
	{.compatible = "oled-pins",},
	{/* keep this */}
};
MODULE_DEVICE_TABLE(of, oled_dev_of_match);

static struct of_device_id oled_i2c_of_match[] = {
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
/* waitqueue -------------------------------------------------------------------------*/
static DECLARE_WAIT_QUEUE_HEAD(blink_waitqueue_head);
// static struct wait_queue_head_t     blink_waitqueue_head;     // not recommended definition
static struct work_struct           blink_work;
static int condition = 1;

static void blink_reacall_function(struct work_struct *work)
{
    static int repeat_schedule = 1;
    
    usr_msg("---> move in, current condition = %d", condition);
    condition += 1;
    wake_up_interruptible(&blink_waitqueue_head);
    usr_msg("queue has been woke-up, current condition = %d", condition);
    // reschedule 10 times
    if(1 == repeat_schedule && condition < 100) {
        if(0 == condition % 2)
               gpio_set_state(high);
        else
            gpio_set_state(low);
        mdelay(1000);
        schedule_work(&blink_work);
    }
   else {
       // after 10 times schedule_work, program will be suspend
       usr_msg("skip wake_up_interruptible, condition = %d", condition);
   }
}


void waitqueue_init(void)
{
    init_waitqueue_head(&blink_waitqueue_head);
    INIT_WORK(&blink_work, blink_reacall_function);
    schedule_work(&blink_work);
    wait_event_interruptible(blink_waitqueue_head, condition != 0);
    usr_msg("waitqueue_created, init codition =  %d", condition);
}


/* i2c fileoperations -------------------------------------------------------*/
static int oled_i2c_open(struct inode *inode, struct file *filp)
{
    usr_msg("oled proc : oled_i2c_open");
    return 0;
}

static ssize_t oled_i2c_read (struct file *flip, char __user *buff,
                            size_t counter, loff_t *fops)
{
    usr_msg("oled proc : oled_i2c_read");
    return 0;
}
static ssize_t oled_i2c_write(struct file *flip, const char __user *buff,
                                    size_t counter, loff_t *fops)
{
    usr_msg("oled proc : oled_i2c_write");
    int value;
    int ret;
    int index;
    usr_msg("oled device : write");

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
static long oled_i2c_ioctl (struct file *flip, unsigned int cmd,
                            unsigned long param)
{
    usr_msg("oled proc : oled_i2c_ioctl");
    return 0;
}

static struct file_operations oled_i2c_fops = {
	.open 			= oled_i2c_open,
	.write			= oled_i2c_write,
	.read 			= oled_i2c_read,
	.release 		= oled_i2c_release,
	.unlocked_ioctl = oled_i2c_ioctl,
};

register_oled_i2c_driver(struct i2c_client * client)
{
	int ret = 0;
	struct _oled_i2c_info  * i2c_info;
	i2c_info = i2c_get_clientdata(client);
	ret = alloc_chrdev_region(&i2c_info->dev->devno , 0, 1, OLED_DEV_NAME);
	if(ret < 0) {
		err_msg("error : alloc_chrdev_region");
		ret = -ENODEV;
		goto out;
	}
	i2c_info->dev->major = MAJOR(i2c_info->dev->devno);
	usr_msg("---> i2c_info->dev->major = %d", i2c_info->dev->major);
	cdev_init(&i2c_info->dev->cdev, &oled_i2c_fops);
	ret = cdev_add(&i2c_info->dev->cdev , i2c_info->dev->devno, 1);
	if(ret < 0) {
		err_msg("error : cdev_add.");
		goto err_cdev_add;
	}

	i2c_info->dev->cls = class_create(THIS_MODULE, OLED_DEV_NAME);
    if (IS_ERR(i2c_info->dev->cls)) {
        err_msg("error : class create.");
        ret = PTR_ERR(i2c_info->dev->cls);
        goto err_cls_create;
    }
    i2c_info->dev->dev = device_create(i2c_info->dev->cls, NULL,
                                                i2c_info->dev->devno,
                                                NULL, "%s", OLED_DEV_NAME);
    if(IS_ERR(i2c_info->dev->dev)) {
        err_msg("error : device_create.");
        ret = PTR_ERR(i2c_info->dev->dev);
        goto err_dev_create;
    }
    return ret;

err_dev_create:
    class_destroy(i2c_info->dev->cls);
err_cls_create:
	cdev_del(&i2c_info->dev->cdev);
err_cdev_add:
	unregister_chrdev_region(i2c_info->dev->devno, 1);
out:
	return ret;
}

int oled_get_i2c_dts_info(struct i2c_client * client)
{
	struct _oled_i2c_info * i2c_info;
	i2c_info = i2c_get_clientdata(client);
	i2c_info->dts->i2c_node = client->dev.of_node;
	if(IS_ERR(i2c_info->dts->i2c_node)) {
		err_msg("error : i2c_info->dts->i2c_node");
		return -ENODATA;
	}
	// get dts information
	of_property_read_u32(i2c_info->dts->i2c_node, "width", &i2c_info->dts->oled_width);
	usr_msg("---> width = %d", i2c_info->dts->oled_width);
	of_property_read_u32(i2c_info->dts->i2c_node, "height", &i2c_info->dts->oled_height);
	usr_msg("---> height = %d", i2c_info->dts->oled_height);

	return 0;

}
s
static int oled_i2c_probe(struct i2c_client * client, const struct i2c_device_id *i2c_id)
{
    int ret = 0;
    struct _oled_i2c_info *oled_i2c_info;
    oled_i2c_info = devm_kmalloc(&client->dev, sizeof(struct _oled_i2c_info), GFP_KERNEL);
	if(IS_ERR(oled_i2c_info)) {
		return ERR_PTR(oled_i2c_info);
		err_msg("error : devm_kmalloc oled_i2c_info");
	}
    usr_msg("---> move in");
#if 0
    if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err_msg("i2c check failed");
		return -ENODEV;
    }
#endif

    usr_msg("chip found, driver verison, i2c_addr = 0x%x", client->addr);

	mutex_init(&oled_i2c_info->oled_i2c_lock);
	i2c_set_clientdata(client, oled_i2c_info);
	ret = oled_get_i2c_dts_info(client);
	if(ret < 0) {
		err_msg("error : oled_get_i2c_dts_info");
		goto err_out1;
	}
	// stage client
	oled_i2c_info->client = client;
	
	register_oled_i2c_driver(client);
    

    return 0;

err_out1:
	devm_kfree(&client->dev, oled_i2c_info);
	return ret;
}

static int oled_i2c_remove(struct i2c_client *client)
{
    struct _oled_i2c_info *temp = i2c_get_clientdata(client);
    kfree(temp->data);
    mutex_destroy(&temp->oled_i2c_lock);
    kfree(temp);
    return 0;
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



static ssize_t dev_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	usr_msg("---> device attribute show");
	return 0;
}
static ssize_t dev_store(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t count)
{
	usr_msg("---> device attribute store");
	return 0;
}


static const struct device_attribute dev_attr = {
	.attr = {
		.name = OLED_DEV_NAME,
		.mode = 0664,
	},
	.show  = dev_show,
	.store = dev_store,
};


static int oled_get_dev_dts_info(struct platform_device * pdev)
{
	int ret;
	struct _oled_device_info * oled_dev_info;

	oled_dev_info = platform_get_drvdata(pdev);
	
    mutex_lock(&oled_dev_info->oled_dev_lock);
    // get dts gpio configurations
    oled_dev_info->dev_pinctrl = devm_pinctrl_get(&pdev->dev);
    if(IS_ERR(oled_dev_info->dev_pinctrl)) {
        err_msg("error : oled_dts_info->dev_pinctrl");
        ret = PTR_ERR(oled_dev_info->dev_pinctrl);
        goto err_no_dev;
    }
    oled_dev_info->rst_low = pinctrl_lookup_state(oled_dev_info->dev_pinctrl, "default");
    if(IS_ERR(oled_dev_info->rst_low)) {
        err_msg("error : oled_dts_info->rst_high");
        ret = PTR_ERR(oled_dev_info->rst_low);
        goto err_no_dev;
    }
    oled_dev_info->rst_high = pinctrl_lookup_state(oled_dev_info->dev_pinctrl, "rst_high");
    if(IS_ERR(oled_dev_info->dev_pinctrl)) {
        err_msg("error : oled_dev_info->rst_high");
        ret = PTR_ERR(oled_dev_info->rst_high);
        goto err_no_dev;
    }
    // set default state
    pinctrl_select_state(oled_dev_info->dev_pinctrl, oled_dev_info->rst_high);
    usr_msg("set rest-pin default logic level is high");
    mutex_unlock(&oled_dev_info->oled_dev_lock);
    return ret;
    
err_no_dev:
	return ret;
}

int gpio_set_state(struct platform_device * pdev, status state)
{
    int ret;
	struct _oled_device_info * oled_dev_info;
	oled_dev_info = platform_get_drvdata(pdev);
	
    mutex_lock(&oled_dev_info->oled_dev_lock);
    if(low == state) {
        ret = pinctrl_select_state(oled_dev_info->dev_pinctrl, oled_dev_info->rst_low);
        if(ret < 0) {
            err_msg("error : set oled_dts_info->rst_low");
            ret = -EIO;
        }
        
    } else {
        ret = pinctrl_select_state(oled_dev_info->dev_pinctrl,oled_dev_info->rst_high);
        if(ret < 0) {
            err_msg("error : set oled_dts_info->rst_high");
            ret = -EIO;
        }
    }
    mutex_unlock(&oled_dev_info->oled_dev_lock);
    return ret;
}

static int oled_probe(struct platform_device *pdev)
{
	struct _oled_device_info * oled_info;
	// devm_kmalloc malloc memory to target device, no need to manual free
	oled_info = devm_kmalloc(&pdev->dev, sizeof(struct _oled_device_info), GFP_KERNEL);
	if(IS_ERR(oled_info)) {
		err_msg("no memory");
		return -ENOMEM;
	}

	mutex_init(&oled_info->oled_dev_lock);
	
	platform_set_drvdata(pdev, oled_info);
	oled_get_dev_dts_info(&pdev->dev);
	device_create_file(&pdev->dev , &dev_attr);

	// add i2c driver 
	i2c_add_driver(&oled_i2c_driver);

	
}

static int oled_remove(struct platform_device * pdev)
{
	struct _oled_device_info * dev_info;
	dev_info = (struct _oled_device_info *)platform_get_drvdata(pdev);
	
    i2c_del_driver(&oled_i2c_driver);
	// free pin
	pinctrl_free(dev_info->dev_pinctrl, false);
	device_remove_file(&pdev->dev, &dev_attr);
	return 0;
}



static const struct platform_device_id platform_dev_id[] = {
    {"oled,rst-pin", 0}, 
    {/*keep this*/}
};
MODULE_DEVICE_TABLE(oled, platform_dev_id);

static struct platform_driver oled_driver = {
	.probe = oled_probe,
	.remove = oled_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = OLED_DEV_NAME,
		.of_match_table = of_match_ptr(oled_dev_of_match),
	},
    .id_table = platform_dev_id,
};

module_platform_driver(oled_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("quan, <shusheng1991@gmail.com>");
MODULE_ALIAS("i2c :" OLED_DEV_NAME);
MODULE_DESCRIPTION("OLED Driver, control chip SSD1306");
MODULE_VERSION(OLED_DRIVER_VERSION);
