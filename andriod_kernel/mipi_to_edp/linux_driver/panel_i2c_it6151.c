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
#include <linux/of_irq.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <uapi/linux/input.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/i2c.h>

#include "it6151_global.h"


#define IT6151_NAME "IT6151"

#define IT6151_EDP_SLAVE_ADDR_WRITE						(0x5C << 0)
#define	IT6151_MIPIRX_SLAVE_ADDR_WRITE 					(0x6C << 0)
#define DP_I2C_ADDR 	(0x5C << 0)
#define MIPI_I2C_ADDR 	(0x6C << 0)
#define REGFLAG_DELAY 	(0xAB)


#define I2C_WRITE_FLAG			0
#define I2C_READ_FLAG			1


static struct i2c_client *it6151_mipirx = NULL;
static struct i2c_client *it6151_edp = NULL;
static int it6151_mipi_drv_register_flag;

struct mutex it6151_i2c_access;

static const struct i2c_device_id it6151_id_table[] = {
	{.name = "it6151_edp", 		},
	{.name = "it6151_mipirx", 	},
};
MODULE_DEVICE_TABLE(of, it6151_id_table);


typedef enum {
	low = 0,
	high = 1
}gpio_status_enum;




#if 0
static DECLARE_WAIT_QUEUE_HEAD(it6151_init_queue_head);
struct it6151_init_queue {
	struct work_struct	 		it6151_init_wq_struct;
	struct it6151_info			*it6151_info_in_workstruct;
};

static void it6151_init_wq_recall(struct work_struct *work)
{
	struct it6151_init_queue * queue_info = container_of(work, struct it6151_init_queue, it6151_init_wq_struct);
	if(!queue_info) {
		usr_msg("get struct it6151_init_wq_recall in function %s failed", __func__);
	}
	if(2 != it6151_i2c_init_flag)
		schedule();
    usr_msg("get in it6151_init_wq_recall, it6151_i2c_init_flag = %d", it6151_i2c_init_flag);
	it6151_chip_init();
	it6151_i2c_init_flag = 0;
}

static void it6151_create_chip_init_wq(struct it6151_info * info)
{
	struct it6151_init_queue * queue_info = kmalloc(sizeof(struct it6151_init_queue), GFP_KERNEL);
	queue_info->it6151_info_in_workstruct = info;
	init_waitqueue_head(&it6151_init_queue_head);
	INIT_WORK(&queue_info->it6151_init_wq_struct, it6151_init_wq_recall);
	// it6151_init_wq_struct.data = (atomic_long_t)info;
	schedule_work(&queue_info->it6151_init_wq_struct);
	wait_event_interruptible(it6151_init_queue_head, it6151_i2c_init_flag == 2);
	usr_msg("waitqueue_created, init codition =  %d", it6151_i2c_init_flag);
}

#endif
void it6151_power_on(struct it6151_info * info)
{
	// gpio_set_value(info->lcm_main_pwr, high);
	gpio_set_value(info->gpio_stdby_pwr, high);
	gpio_set_value(info->pwr_3v3, high);
	gpio_set_value(info->pwr_1v8, high);
	gpio_set_value(info->pwr_1v2, high);
	mdelay(50);
}

// static void it6151_power_off(struct it6151_info * info)
// {
// 	gpio_set_value(info->gpio_stdby_pwr, low);
// 	mdelay(20);
// }

void IT6151_dump_register(void)
{
	u8 tmp;
	int i;
	usr_msg("moved in function: %s", __func__);
	usr_msg("------------------------ 0x6c -------------------------------");
#if 0
	it6151_i2c_read_byte(DP_I2C_ADDR, 0x0d, &tmp);
	usr_msg("[IT6151_dump_register] IT6151 Reg0x0D=0x%x", tmp);
	it6151_i2c_read_byte(DP_I2C_ADDR, 0x0e, &tmp);
	usr_msg("[IT6151_dump_register] IT6151 Reg0x0E=0x%x", tmp);
	it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x0d, &tmp);
	usr_msg("[IT6151_dump_register] IT6151 Reg0x0D=0x%x", tmp);
#endif
	for(i = 0x00; i <= 0xff; i++) {	
		it6151_i2c_read_byte(MIPI_I2C_ADDR, i, &tmp);
		usr_msg("[IT6151_0x6c_register] IT6151  Reg 0x%02X = 0x%02X", i, tmp); 		
	}
	usr_msg("-------------------------------------------------------------");
	usr_msg("------------------------ 0x5c -------------------------------");
	for(i = 0x00; i <= 0xff; i++) {
		it6151_i2c_read_byte(DP_I2C_ADDR, i, &tmp);
		usr_msg("[IT6151_0x5c_register] IT6151  Reg 0x%02X = 0x%02X", i, tmp); 		
	}
	usr_msg("-------------------------------------------------------------");
}

void IT6151_DPTX_init(void)
{   
	usr_msg("moved in function: %s", __func__);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x29);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x00);
	
	it6151_i2c_write_byte(DP_I2C_ADDR,0x09,INT_MASK);// Enable HPD_IRQ,HPD_CHG,VIDSTABLE
	it6151_i2c_write_byte(DP_I2C_ADDR,0x0A,0x00);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x0B,0x00);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xC5,0xC1);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xB5,0x00);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xB7,0x80);		// 0x00
	it6151_i2c_write_byte(DP_I2C_ADDR,0xC4,0xF0);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x06,0xFF);// Clear all interrupt
	it6151_i2c_write_byte(DP_I2C_ADDR,0x07,0xFF);// Clear all interrupt
	it6151_i2c_write_byte(DP_I2C_ADDR,0x08,0xFF);// Clear all interrupt
	
	it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x00);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x0c,0x08);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x21,0x05);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x3a,0x04);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x5f,0x06);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xc9,0xf5);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xca,0x4c);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xcb,0x37);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xce,0x80);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xd3,0x03);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xd4,0x60);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xe8,0x11);
	it6151_i2c_write_byte(DP_I2C_ADDR,0xec,VIC);
	mdelay(5);			

	it6151_i2c_write_byte(DP_I2C_ADDR,0x23,0x42);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x24,0x07);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x25,0x01);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x26,0x00);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x27,0x10);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x2B,0x05);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x23,0x40);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x22,(DP_AUX_PN_SWAP<<3)|(DP_PN_SWAP<<2)|0x03);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x16,(DPTX_SSC_SETTING<<4)|(DP_LANE_SWAP<<3)|(DPTX_LANE_COUNT<<1)|TRAINING_BITRATE);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x0f,0x01);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x76,0xa7);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x77,0xaf);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x7e,0x8f);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x7f,0x07);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x80,0xef);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x81,0x5f);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x82,0xef);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x83,0x07);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x88,0x38);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x89,0x1f);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x8a,0x48);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x0f,0x00);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x5c,0xf3);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x17,0x04);
	it6151_i2c_write_byte(DP_I2C_ADDR,0x17,0x01);
	mdelay(500);	

	IT6151_dump_register();
}

int it6151_chip_reg_init_config(void)
{
	unsigned char VenID[2], DevID[2], RevID;
	unsigned char data_tmp;
	//unsigned char cmdBuffer;
	usr_msg("moved in function: %s", __func__);
	it6151_i2c_read_byte(DP_I2C_ADDR, 0x00, &VenID[0]); 
	it6151_i2c_read_byte(DP_I2C_ADDR, 0x01, &VenID[1]);
	it6151_i2c_read_byte(DP_I2C_ADDR, 0x02, &DevID[0]);
	it6151_i2c_read_byte(DP_I2C_ADDR, 0x03, &DevID[1]);
	it6151_i2c_read_byte(DP_I2C_ADDR, 0x04, &RevID);	
				
	usr_msg("Current 1 DPDevID=%02X%02X", DevID[1], DevID[0]);
	usr_msg("Current DPVenID=%02X%02X", VenID[1], VenID[0]);
	usr_msg("Current DPRevID=%02X", RevID); 	
	if( VenID[0]==0x54 && VenID[1]==0x49 && DevID[0]==0x51 && DevID[1]==0x61 ){

		usr_msg("[Kernel/LCM] Test 1 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x", DP_I2C_ADDR, MIPI_I2C_ADDR);
		it6151_i2c_write_byte(DP_I2C_ADDR,0x05,0x04);// DP SW Reset
		it6151_i2c_read_byte(DP_I2C_ADDR, 0xfd, &data_tmp);
		usr_msg("it6151 slave 5c value of 0xfd = 0x%x", data_tmp);
		it6151_i2c_write_byte(DP_I2C_ADDR,0xfd,(MIPI_I2C_ADDR<<1)|1);
		it6151_i2c_read_byte(DP_I2C_ADDR, 0xfd, &data_tmp);
		usr_msg("it6151 slave 5c value of 0xfd = 0x%x", data_tmp);
		mdelay(20);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x00);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x0c,(MP_LANE_SWAP<<7)|(MP_PN_SWAP<<6)|(MIPI_LANE_COUNT<<4)|EN_UFO);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x11,MP_MCLK_INV);

		if(RevID == 0xA1){			
			it6151_i2c_write_byte(MIPI_I2C_ADDR,0x19, MP_LANE_DESKEW); 
		}else{
			it6151_i2c_write_byte(MIPI_I2C_ADDR,0x19,(MP_CONTINUOUS_CLK<<1) | MP_LANE_DESKEW); 
		}
				
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x27, MIPI_PACKED_FMT);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x28,((PANEL_WIDTH/4-1)>>2)&0xC0);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x29,(PANEL_WIDTH/4-1)&0xFF);
		
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x2e,0x34);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x2f,0x01);
		
		
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x4e,(MP_V_RESYNC<<3)|(MP_H_RESYNC<<2)|(MP_VPOL<<1)|(MP_HPOL));
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x80,(EN_UFO<<5)|MP_PCLK_DIV);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x84,0x8f);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x09,MIPI_INT_MASK);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x92,TIMER_CNT);	
		/** start: for rockchip platform -------------------------*/
	#if 0
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x33,0x80 | MIPI_HSYNC_W >> 8);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x32,MIPI_HSYNC_W & 0xFF);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x3D,0x80 | MIPI_VSYNC_W >> 8);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x3C,MIPI_VSYNC_W & 0xFF);
		/** end: for rockchip platform -------------------------*/
		// 1920=0x0780, 1080=0x0438
	#endif
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x33,0x80 | 0x07);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x32,0x80);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x3D,0x80 | 0x04);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x3C,0x38);

		IT6151_DPTX_init();

		return 0;
	}
	usr_msg("[Kernel/LCM] Test 2 DP_I2C_ADDR=0x%x, MIPI_I2C_ADDR=0x%x", DP_I2C_ADDR, MIPI_I2C_ADDR);

	it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x00, &VenID[0]);
	it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x01, &VenID[1]);
	it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x02, &DevID[0]);
	it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x03, &DevID[1]);
	it6151_i2c_read_byte(MIPI_I2C_ADDR, 0x04, &RevID);

	usr_msg("Current 2 Current MPDevID=%02X%02X", DevID[1], DevID[0]);
	usr_msg("Current Current MPVenID=%02X%02X", VenID[1], VenID[0]);
	usr_msg("Current Current MPRevID=%02X", RevID);

	if(VenID[0]==0x54 && VenID[1]==0x49 && DevID[0]==0x21 && DevID[1]==0x61 ){
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x33);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x40);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x05,0x00);
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x0c,(MP_LANE_SWAP<<7)|(MP_PN_SWAP<<6)|(MIPI_LANE_COUNT<<4));
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x11, MP_MCLK_INV); 
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x19,(MP_CONTINUOUS_CLK<<1) | MP_LANE_DESKEW);	
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x4B,(FRAME_RESYNC<<4));
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x4E,(MP_V_RESYNC<<3)|(MP_H_RESYNC<<2)|(MP_VPOL<<1)|(MP_HPOL)); 	 
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x72,0x01); 
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x73,0x03); 
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0x80,MP_PCLK_DIV); 
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC0,(HIGH_PCLK<< 4) | 0x0F);	
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC1,0x01);  
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC2,0x47);  
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC3,0x67);  
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0xC4,0x04);  
		it6151_i2c_write_byte(MIPI_I2C_ADDR,0xCB,(LVDS_PN_SWAP<<5)|(LVDS_LANE_SWAP<<4)|(LVDS_6BIT<<2)|(LVDS_DC_BALANCE<<1)| VESA_MAP);	
		return 1;
	} 
	return -1;
}



int usr_get_dts_info(struct it6151_info * info, struct i2c_client * client)
{
	int ret;
		
	struct device_node *node = NULL;
	usr_msg("device node name: %s", client->dev.of_node->name);
	if(!client->dev.of_node) {
		usr_msg("device node is empty");
		return -ENODEV;
	}
	node = client->dev.of_node;
    info->gpio_stdby_pwr = of_get_named_gpio(node, "standby-pwr-gpio", 0);
    if(!gpio_is_valid(info->gpio_stdby_pwr)) {
        usr_msg("error: get standby-pwr-gpio gpio info");
        return -EIO;
    }

	info->gpio_int = of_get_named_gpio(node, "interrupt-gpio", 0);
	if(!gpio_is_valid(info->gpio_int)) {
		usr_msg("error: get interrupt-gpio gpio info.");
		return -EIO;
	}
	info->pwr_3v3 = of_get_named_gpio(node, "it6151-3v3", 0);
	if(!gpio_is_valid(info->pwr_3v3)) {
		usr_msg("error: get pwr_3v3 gpio info.");
		return -EIO;
	}
	info->pwr_1v8 = of_get_named_gpio(node, "it6151-1v8", 0);
	if(!gpio_is_valid(info->pwr_1v8)) {
		usr_msg("error: get pwr_1v8 gpio info.");
		return -EIO;
	}
	info->pwr_1v2 = of_get_named_gpio(node, "it6151-1v2", 0);
	if(!gpio_is_valid(info->pwr_1v2)) {
		usr_msg("error: get pwr_1v2 gpio info.");
		return -EIO;
	}
#if 0
	info->lcm_main_pwr = of_get_named_gpio(node, "lcm-main-pwr", 0);
	if(!gpio_is_valid(info->lcm_main_pwr)) {
		usr_msg("error: get lcm_main_pwr gpio info.");
		return -EIO;
	}
#endif
	ret = devm_gpio_request_one(&client->dev , info->gpio_int, GPIOF_IN, "it6151-int");
	if(ret < 0) {
		usr_msg("error: it6151-int request.");
		return -EIO;
	}
	ret = devm_gpio_request_one(&client->dev, info->gpio_stdby_pwr, GPIOF_OUT_INIT_HIGH, "it6151-stby-pwr");
    if(ret < 0) {
		usr_msg("error: it6151-standby request.");
		ret = -EIO;
		goto out1;
	}
	ret = devm_gpio_request_one(&client->dev, info->pwr_3v3, GPIOF_OUT_INIT_HIGH, "it6151-3v3");
    if(ret < 0) {
		usr_msg("error: it6151-standby request.");
		ret = -EIO;
		goto out1;
	}
	ret = devm_gpio_request_one(&client->dev, info->pwr_1v8, GPIOF_OUT_INIT_HIGH, "it6151-1v8");
    if(ret < 0) {
		usr_msg("error: it6151-standby request.");
		ret = -EIO;
		goto out1;
	}
	ret = devm_gpio_request_one(&client->dev, info->pwr_1v2, GPIOF_OUT_INIT_HIGH, "it6151-1v2");
    if(ret < 0) {
		usr_msg("error: it6151-standby request.");
		ret = -EIO;
		goto out1;
	}
#if 0
	ret = devm_gpio_request_one(&client->dev, info->lcm_main_pwr, GPIOF_OUT_INIT_HIGH, "lcm-main-pwr");
    if(ret < 0) {
		usr_msg("error: lcm-main-pwr request.");
		ret = -EIO;
		goto out1;
	}
#endif
	info->irq_num = gpio_to_irq(info->gpio_int);
	usr_msg("requested gpio_int: %d, gpio_standby: %d, irq_num = %d", 
						info->gpio_int, info->gpio_stdby_pwr, info->irq_num);
	if(ret < 0)
		goto out1;
	
	return 0;
	
out1:
	return ret;

}


int it6151_i2c_read_byte(uint8_t adpt_addr, uint8_t reg_addr, uint8_t *data)
{
	struct i2c_msg msg[2];
	char data_tmp = 0;
	int ret = 0;


	mutex_lock(&it6151_i2c_access);

	if(adpt_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE) {
		msg[0].addr = adpt_addr;
		msg[0].flags = it6151_mipirx->flags & I2C_WRITE_FLAG;
		msg[0].len = 1;
		msg[0].buf = &reg_addr;
		msg[1].addr = adpt_addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = 1;
		msg[1].buf = &data_tmp;
		ret = i2c_transfer(it6151_mipirx->adapter, msg, 2);
		// usr_msg("IT6151_MIPIRX: ret = %d", ret);
				
	} else if(adpt_addr == IT6151_EDP_SLAVE_ADDR_WRITE) {
		msg[0].addr = adpt_addr;
		msg[0].flags = it6151_mipirx->flags & I2C_WRITE_FLAG;
		msg[0].len = 1;
		msg[0].buf = &reg_addr;
		msg[1].addr = adpt_addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = 1;
		msg[1].buf = &data_tmp;
		ret = i2c_transfer(it6151_edp->adapter, msg, 2);
		// usr_msg("IT6151_EDP:  ret = %d", ret);

	}
	if(ret < 0) {
		if(adpt_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE) {
			usr_msg("IT6151_read_mipirx:  ret = %d", ret);
		} else if(adpt_addr == IT6151_EDP_SLAVE_ADDR_WRITE) {
			usr_msg("IT6151_read_edp:  ret = %d", ret);
		}
		mutex_unlock(&it6151_i2c_access);
		return ret;
	}
	*data = data_tmp;
	mutex_unlock(&it6151_i2c_access);

	return ret;
}

int it6151_i2c_write_byte(uint8_t adpt_addr, uint8_t reg_addr, uint8_t data)
{
#if 1
	int ret = 0;
	char msg_buff[] = {reg_addr, data};
	
	mutex_lock(&it6151_i2c_access);

	if(adpt_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE) {
		it6151_mipirx->addr = IT6151_MIPIRX_SLAVE_ADDR_WRITE;
		ret = i2c_master_send(it6151_mipirx, msg_buff, 2);
	
	} else if(adpt_addr == IT6151_EDP_SLAVE_ADDR_WRITE) {
		it6151_mipirx->addr = IT6151_EDP_SLAVE_ADDR_WRITE;
		ret = i2c_master_send(it6151_edp, msg_buff, 2);

	}else {
		usr_msg("[it6151_i2c_write_byte]error:  no this dev_addr");
	}
	if(ret < 0) {
		if(adpt_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE) {
			usr_msg("IT6151_write_mipirx:  ret = %d", ret);
		} else if(adpt_addr == IT6151_EDP_SLAVE_ADDR_WRITE) {
			usr_msg("IT6151_write_edp:	ret = %d", ret);
		}

		mutex_unlock(&it6151_i2c_access);
		return ret;
	}
	mutex_unlock(&it6151_i2c_access);

	return ret;

#else
	struct i2c_msg msg;
	char msg_buff[] = {reg_addr, data};
	int ret = 0;


	mutex_lock(&it6151_i2c_access);

	if(adpt_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE) {
		msg.addr = adpt_addr;
		msg.flags = it6151_mipirx->flags & I2C_WRITE_FLAG;
		msg.len = 2;
		msg.buf = msg_buff;
		ret = i2c_transfer(it6151_mipirx->adapter, &msg, 1);
		usr_msg("IT6151_MIPIRX: ret = %d", ret);
		
	} else if(adpt_addr == IT6151_EDP_SLAVE_ADDR_WRITE) {
		msg.addr = adpt_addr;
		msg.flags = it6151_mipirx->flags & I2C_WRITE_FLAG;
		msg.len = 2;
		msg.buf = msg_buff;
		ret = i2c_transfer(it6151_edp->adapter, &msg, 1);
		usr_msg("IT6151_EDP:  ret = %d", ret);

	}else {
		usr_msg("[it6151_i2c_read_byte]error:  no this dev_addr! \n");
	}
	if(ret < 0) {
		if(adpt_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE) {
			usr_msg("IT6151_write_mipirx:  ret = %d", ret);
		} else if(adpt_addr == IT6151_EDP_SLAVE_ADDR_WRITE) {
			usr_msg("IT6151_write_edp:  ret = %d", ret);
		}

		mutex_unlock(&it6151_i2c_access);
		return ret;
	}
	mutex_unlock(&it6151_i2c_access);

	return ret;
#endif
}


static int it6151_i2c_probe(struct i2c_client * client, const struct i2c_device_id * dev_id)
{
	int ret;
	
	struct it6151_info *info = kmalloc(sizeof(struct it6151_info), GFP_KERNEL);
	if(IS_ERR(info)) {
		usr_msg("error: devm_kmalloc");
		return -ENOMEM;
	}
	usr_msg("---> moved in function %s", __func__);
	mutex_init(&it6151_i2c_access);
	
	usr_msg("--->in function: %s, client->name = %s", __func__, client->name);
	if(0 == strcmp(client->name, it6151_id_table[0].name)) {
		if (!(it6151_edp = (struct i2c_client *)kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
			ret = -ENOMEM;
			goto exit;
		}
		memset(it6151_edp, 0, sizeof(struct i2c_client));

		it6151_edp = client;
		
		i2c_set_clientdata(client, (void *)info);
		ret = usr_get_dts_info(info, client);
		if(ret < 0) {
			usr_msg("in function: %s, usr_get_dts_info failed", __func__);
			goto exit;
		}
		it6151_power_on(info);
		// it6151_create_chip_init_wq(info);
		// it6151_i2c_init_flag = 1;
		// add_it6151_mipi_i2c_drv();		// add mipirx i2c driver
		
		// if(ret < 0) {
		// 	usr_msg("in function: %s, it6151_chip_reg_init_config failed", __func__);
		// }
		// it6151_edp_drv_register_flag = 1;
		usr_msg("---> moved out function %s", __func__);
		// wake_up_interruptible(&wait_it6151_head);
	} else if(0 == strcmp(client->name, it6151_id_table[1].name)) {
		usr_msg("--->in function: %s, client->name = %s", __func__, client->name);
		if (!(it6151_mipirx = (struct i2c_client *)kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
			ret = -ENOMEM;
			goto exit;
		}
		memset(it6151_mipirx, 0, sizeof(struct i2c_client));

		it6151_mipirx = client;
		it6151_mipi_drv_register_flag = 1;
		it6151_chip_reg_init_config();
	} else
		usr_msg("client name not matched");
	
	return 0;
	
exit:
	kfree(info);
	return ret;	
}

static const struct of_device_id it6151_match_table[] = {
	{.compatible = "it6151_edp"},
	{.compatible = "it6151_mipirx"},
	/** keep this */
};
MODULE_DEVICE_TABLE(i2c, it6151_match_table);
	

static struct i2c_driver it6151_edp_driver = {
	.probe = it6151_i2c_probe,
	.driver = {
		//.owner = THIS_MODULE,
		.name = IT6151_NAME,
		.of_match_table = of_match_ptr(it6151_match_table),
	},
	.id_table = it6151_id_table,
};

#if 0
static void add_it6151_mipi_i2c_drv(void)
{
	usr_msg("add i2c driver for it6151 address 0x6c");
	i2c_add_driver(&it6151_edp_driver);
}
#endif

module_i2c_driver(it6151_edp_driver);
// module_i2c_driver(it6151_mipi_driver);


MODULE_AUTHOR("V <machester@aliyun.com>");
MODULE_DESCRIPTION("DRM Driver for mipi to edp transder chip it6151");
MODULE_LICENSE("GPL");














