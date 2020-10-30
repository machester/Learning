/*************************************************************************
led_control: led-control {
        compatible = "led-gpio";
        status = "okay";
        gpios = <&gpio5 RK_PB4 GPIO_ACTIVE_HIGH>;
        gpio-keys = <&gpio5 RK_PB3 GPIO_ACTIVE_LOW>;
        led_status_inverse = <500>;			// 默认led 闪烁间隔
        key_report_inverse = <100>;			// 按键触发上报间隔
    };
**************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/slab.h> 
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/of.h>


struct Led_node{
	int gpios;						
	int gpio_keys;
	int led_status;						
	int long key_report;
	
	dev_t devno;
	struct class *cls;
	struct device *dev;
	int data;
	struct cdev *cdev;
	unsigned int irqno;
	struct timer_list timer_list;//定时器
};


static void ms_timer_handler(void)//定义定时器处理函数

{
    printk("<kernel>call %s\n",__FUNCTION__);
	static int val=0;
	if(val=0)
	{
		gpio_request(*led_node->gpios, "led_gpio");
		gpio_direction_output(*led_node->gpio, 1);//伪代码，置位指定输出
		gpio_free(*led_node->gpio);
		val=1;
	}
	else
	{
		gpio_request(*led_node->gpios, "led_gpio");
		gpio_direction_output(*led_node->gpio, 0);//伪代码，置位指定输出
		gpio_free(*led_node->gpio);
		val=0;
	}
}

static struct attribute *audio_attrs[] = {
    &driver_attr_led_test.attr,
    NULL,
};

static struct attribute_group audio_group = {
    .name ="led-gpio",
    .attrs = audio_attrs,
}; 

static const struct attribute_group *groups[] = {
    &audio_group,
    NULL,
};

//中断处理函数  ----当触发中断时调用
static irqreturn_t  button_irq_handler(int irqno, void *dev)
{
	int value;
	printk("<kernel>call %s\n",__FUNCTION__);
	value=gpio_get_value(*led_node->gpio-keys);
	//判断是否按下或松开	
	if(value){
		if((led_node->key_report-led_node->key_report)>=100)//与上次上报时间间隔100ms以上，执行以下操作
			led_node->led_status=led_node->led_status+500;//一次按键结束更新led_status值
	}else{
		printk("<kernel> key2 key down");
	}
	
	return IRQ_HANDLED;
}

static int led_open(struct inode *inode, struct file *file)
{
	
	printk("<kernel>call %s\n",__FUNCTION__);	
	return 0;
}


static const struct file_operations button_fops = {
	.owner		= THIS_MODULE,	//拥有这个模块的指针 ，避免使用时卸载
	.open		= led_open,	
	.release	= NULL,
};

/***************************************
struct Led_node{
	int gpios;						
	int gpio-keys;
	int led_status;						
	int long key_report;
	
	dev_t devno;
	struct class *cls;
	struct device *dev;
	int data;
	struct cdev *cdev;
	unsigned int irqno;
	struct timer_list timer_list;//定时器
};
****************************************/
static int get_dts_info(struct platform_device * pdev)
{
	struct device_node *node = pdev->dev.of_node;
	if (!node) {
		usr_msg("cannot find node name rockchip, reboot-test");
		return -ENODEV;
	}
	info->reboot_trigger_io = of_get_named_gpio(node, "reboot-key", 0);
	if(!gpio_is_valid(info->reboot_trigger_io)) {
		usr_msg("error: get reboot-key gpio info. ERROR code %d", info->reboot_trigger_io);
		return -EIO;
	}
	ret = of_property_read_u32(node, "detect-interval", &info->detect_interval);
    if(ret < 0) {
        usr_msg("error: read detect-interval.  ERROR code %d", info->detect_interval);
        return -EINVAL;
    }
	ret = devm_gpio_request_one(dev, info->reboot_trigger_io, GPIOF_IN, "reboot-test-key-gpios");
    if(ret < 0) {
		usr_msg("error: reboot-key-gpios request.");
		return -EIO;
	}
	debug_msg("requested reboot_trigger_io number: %d, info->detect_interval = %d\n", info->reboot_trigger_io, info->detect_interval);
	
	if(!node) {
		printk(KERN_INFO"---> node cannot be found\n");
		return -EIO;
	}
	led_node->gpios = of_get_named_gpio(node, "gpios", 0);
	led_node->gpio_keys = of_get_named_gpio(node, "gpio-keys", 0);
}

static int led_test_probe(struct platform_device *pdev)
{    
	struct Led_node *led_node;
	led_node = devm_kmalloc(&pdev->dev ,sizeof(struct Led_node), GFP_KERNEL);

	
	init_timer(&(led_node->timer_list));//初始化定时器
	ms_timer.expires=jiffies+msecs_to_jiffies(led_node->led_status);//定义中断时间：10ms进入中断
	ms_timer.function=&ms_timer_handler;//定义定时器中断处理函数
    add_timer(&ms_timer);//注册定时器，使定时器生效
	
	//申请中断
	led_node->irqno = IRQ_EINT(*led_node->gpio-keys);//依据具体外部引脚，此处不清楚具体引脚获取按键宏值假定中断，值为伪代码
	res= request_irq(led_node->irqno, button_irq_handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "eint-key", NULL);

    return 0;
}

static struct of_device_id usr_match_table[] = {
	{ .compatible = "led-gpio",},
    { },
};

static struct platform_driver led_test = {
    .driver = {
        .name  = "led-gpio",
        .owner  = THIS_MODULE,
        .of_match_table = usr_match_table,
    },

    .probe = led_test_probe,
    .remove = NULL,
};

static int __init audio_info_init(void)
{
    return platform_driver_register(&led_test);
}

static void __exit audio_info_exit(void)
{
    platform_driver_unregister(&led_test);
}

module_init(audio_info_init);
module_exit(audio_info_exit);
MODULE_LICENSE("GPL");