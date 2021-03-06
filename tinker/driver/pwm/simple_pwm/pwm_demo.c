/***** pwm dts defined in file rk3288.dtsi ***********************
	pwm2: pwm@ff680020 {
		compatible = "rockchip,rk3288-pwm";
		reg = <0x0 0xff680020 0x0 0x10>;
		#pwm-cells = <3>;
		pinctrl-names = "active";
		pinctrl-0 = <&pwm2_pin>;
		clocks = <&cru PCLK_PWM>;
		clock-names = "pwm";
		status = "disabled";
	};

	pwm3: pwm@ff680030 {
		compatible = "rockchip,rk3288-pwm";
		reg = <0x0 0xff680030 0x0 0x10>;
		#pwm-cells = <2>;
		pinctrl-names = "active";
		pinctrl-0 = <&pwm3_pin>;
		clocks = <&cru PCLK_PWM>;
		clock-names = "pwm";
		status = "disabled";
	};
*******************************************************************************/
#include "pwm_demo.h"

/* set compatible dts start -----------------------------------*/

static const struct of_device_id usr_pwm_match_table[] = {
	{.compatible = "pwm_module"},
    {/** keep this */},
};
/* set compatible dts end -----------------------------------*/

/*-------------- standard timer start ------------------------------------*/
#if USED_HRS_TIMER

#define MS_TO_NS(x) (x * 1000000)      // ms to ns

struct pwm_timer_info {
	struct 	 mutex		 tim_lock;
    struct   hrtimer     usr_hrtimer;
    struct   timeval     timval;
    ktime_t  tm_period;
};

static struct pwm_timer_info      * usr_tim;



static enum hrtimer_restart pwm_hrtimer_callback(struct hrtimer * arg)
{
    ktime_t now = arg->base->get_time();
    usr_msg("timer running at jiffies=%ld\n", jiffies);
    hrtimer_forward(arg, now, usr_tim->tm_period);
    return HRTIMER_RESTART;
}

int usr_pwm_timer_init(unsigned int usr_ticks)
{
    int retval;

    if(usr_ticks > 5000 || usr_ticks <= 0) {
        err_msg("error : init timer ticks error");
        return -EINVAL;
    }
    usr_tim = kmalloc(sizeof(struct pwm_timer_info), GFP_KERNEL);
    if(!usr_tim) {
        err_msg("error : usr_tim struct malloc");
        retval = -ENOMEM;
        return retval;
    }
    
    mutex_init(&usr_tim->tim_lock);
    mutex_lock(&usr_tim->tim_lock);
    // ktime_set(const s64 secs, const unsigned long nsecs); // param1: second, param2:nanosecond
    usr_tim->tm_period = ktime_set(0, MS_TO_NS(usr_ticks));     // set 1second, 1000 nanosecond.
    hrtimer_init(&usr_tim->usr_hrtimer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    usr_tim->usr_hrtimer.function = pwm_hrtimer_callback;
    hrtimer_start(&usr_tim->usr_hrtimer, usr_tim->tm_period, HRTIMER_MODE_REL);
    mutex_unlock(&usr_tim->tim_lock);

    return retval;
}
#endif // end #if USED_HRS_TIMER


#if TRANDITIONAL_WAY
static int pwm_open(struct inode *inode, struct file *filp)
{
    usr_msg("oled proc : oled_i2c_open");
    return 0;
}
static ssize_t pwm_wirte(struct file *flip, const char __user *buff,
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
static int pwm_release(struct inode *inode, struct file *filp)
{
    usr_msg("oled proc : oled_i2c_release");
    return 0;
}
static long pwm_ioctl (struct file *flip, unsigned int cmd,
                            unsigned long param)
{
    usr_msg("oled proc : oled_i2c_ioctl");
    return 0;
}

static struct file_operations usr_fops = {
	.open = pwm_open,
	.write = pwm_wirte,
	.release = pwm_release,
	.unlocked_ioctl = pwm_ioctl,
};

#else   // #if TRANDITIONAL_WAY

static ssize_t usr_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	usr_msg("usr show");
    return 0;
}

static ssize_t usr_close(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	usr_msg("usr close");
    return 0;
}

static struct device_attribute usr_pwm_attrs = {
	.show = usr_show,
	.store = usr_close,
	.attr = {
		.name = "usr_pwm_attr",
		.mode = 0664,
	},
};


static int usr_pwm_dev_create_file(struct device * dev)
{
    int err;
	usr_msg("pwm device create file start");
	err = device_create_file(dev, &usr_pwm_attrs);
    if(err < 0) {
        err_msg("error : create file");
        err =  -EINVAL;
    }
    return err;
}

static int usr_pwm_destory_sysfs(struct device * dev)
{
	usr_msg("pwm device destory file start");
	device_remove_file(dev, &usr_pwm_attrs);
    return 0;
}
#endif // end #if TRANDITIONAL_WAY

static int usr_pwm_register_device(struct platform_device *pdev)
{
    int ret;
	struct pwm_dev * pwm_dev;
	pwm_dev = platform_get_drvdata(pdev);	
	
    ret = alloc_chrdev_region(&pwm_dev->pwm_dev_num, 0, 1, USR_PWM_DRV_NAME);
    if(ret < 0) {
        err_msg("error : alloc_chrdev_region");
        ret = -ENODEV;
        return ret;
    }
	
    pwm_dev->pwm_major_num = MAJOR(pwm_dev->pwm_dev_num);
    usr_msg("device major number = %d", pwm_dev->pwm_major_num);
#if TRANDITIONAL_WAY
    cdev_init(&pwm_dev->pwm_cdev, &usr_fops);
    pwm_dev->pwm_cdev.owner = THIS_MODULE;
    
    ret = cdev_add(&pwm_dev->pwm_cdev, pwm_dev->pwm_dev_num, 1);
    if(ret < 0) {
        err_msg("error : cdev add");
        ret = -ENODEV;
        goto err_cdev_add;
    }
    pwm_dev->pwm_class = class_create(THIS_MODULE, USR_PWM_DRV_NAME);
    if(IS_ERR(pwm_dev->pwm_class)) {
        err_msg("error : class_create");
        ret = PTR_ERR(pwm_dev->pwm_class);
        goto err_class_create;
    }
    pwm_dev->pwm_device = device_create(pwm_dev->pwm_class, NULL, pwm_dev->pwm_dev_num, NULL, "%s", USR_PWM_DRV_NAME);
    if(IS_ERR(pwm_dev->pwm_device)) {
        err_msg("error : device_create");
        ret = PTR_ERR(pwm_dev->pwm_device);
        goto err_device_create;
    }
    usr_msg("pwm device create success");
    return ret;
	
err_device_create:
    class_destroy(pwm_dev->pwm_class);
err_class_create:
	cdev_del(&pwm_dev->pwm_cdev);
err_cdev_add:
    unregister_chrdev_region(pwm_dev->pwm_dev_num, 1);
    return ret;
	
	
#else   // #if TRANDITIONAL_WAY
	ret = usr_pwm_dev_create_file(&pdev->dev);
    return ret;
#endif // end #if TRANDITIONAL_WAY
}


static void usr_pwm_unregister_device(struct platform_device *pdev)
{
    struct pwm_dev * pwm_dev;

	pwm_dev = platform_get_drvdata(pdev);
	usr_msg("usr pwm unregister start");
	
#if TRANDITIONAL_WAY
	device_destroy(pwm_dev->pwm_class, pwm_dev->pwm_dev_num);
    class_destroy(pwm_dev->pwm_class);
	cdev_del(&pwm_dev->pwm_cdev);
    unregister_chrdev_region(pwm_dev->pwm_dev_num, 1);
	kfree(pwm_dev);
	
#else
	usr_pwm_destory_sysfs(&pdev->dev);

#endif // end #define TRANDITIONAL_WAY
}


static int usr_pwm_probe(struct platform_device *pdev)
{
    int ret;
   	struct pwm_dev *usr_pwm_info;

	usr_msg("pwm driver init start");
	usr_pwm_info = kmalloc(sizeof(struct pwm_dev), GFP_KERNEL);
	if(!usr_pwm_info) {
		err_msg(" err : kmalloc");
		return -ENOMEM;
	}
	memset(usr_pwm_info, 0, sizeof(struct pwm_dev));
    mutex_init(&usr_pwm_info->lock);
	
	dev_set_drvdata(&pdev->dev, usr_pwm_info);

	ret = usr_pwm_register_device(pdev);
    if(ret < 0) {
        err_msg("error : pwm driver register.");
        kfree(usr_pwm_info);
    }
    return ret;
	
}

static int usr_pwm_remove(struct platform_device *pdev)
{
	usr_pwm_unregister_device(pdev);
	return 0;
}

static int usr_pwm_suspend(struct platform_device * pdev, pm_message_t state)
{
    
    return 0;
}

static int usr_pwm_resume(struct platform_device * pdev)
{
    return 0;
}


static const struct platform_device_id usr_pwm_dev_id[] = {
    {"usr_pwm", 0},
    {/*keep this*/}
};
MODULE_DEVICE_TABLE(usr_pwm, usr_pwm_dev_id);

struct platform_driver usr_pwm_drv = {
	.driver = {
		.owner = THIS_MODULE,
		.name = USR_PWM_DRV_NAME,
		.of_match_table = of_match_ptr(usr_pwm_match_table), 
	},
	.id_table = usr_pwm_dev_id,
	.probe = usr_pwm_probe,
	.remove = usr_pwm_remove,
	.suspend = usr_pwm_suspend,
	.resume = usr_pwm_resume,
};

static int __init usr_pwm_init(void)
{
	usr_msg("usr pwm module init start");
	return platform_driver_register(&usr_pwm_drv);
}

static void __exit usr_pwm_exit(void)
{
	usr_msg("usr pwm module exit start");
	platform_driver_unregister(&usr_pwm_drv);
}

module_init(usr_pwm_init);
module_exit(usr_pwm_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("quan");
MODULE_DESCRIPTION("kernel jiffy timer example.");
MODULE_ALIAS("USR PWM");
