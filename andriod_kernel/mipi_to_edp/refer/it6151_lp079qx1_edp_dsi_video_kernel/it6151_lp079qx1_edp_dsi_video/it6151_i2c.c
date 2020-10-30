#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define IT6151_EDP_SLAVE_ADDR_WRITE						(0x5C << 0)
#define	IT6151_MIPIRX_SLAVE_ADDR_WRITE 				(0x6C << 0)

#define DEVICE_NAME "IT6151"

static struct i2c_client *it6151_mipirx = NULL;
static struct i2c_client *it6151_edp = NULL;

static const struct i2c_device_id it6151_i2c_id[] = 
{
	{"it6151_edp",		0},
	{"it6151_mipirx",	0},
};

static int it6151_i2c_driver_probe(struct i2c_client *client, const struct i2c_device_id *id);

static const struct of_device_id it6151_of_match[] = {
	{.compatible = "it6151_edp"},
	{.compatible = "it6151_mipirx"},
};

static struct i2c_driver it6151_i2c_driver = {
	.driver = {
		.name    = DEVICE_NAME,
		.of_match_table = it6151_of_match,
	},
	.probe       = it6151_i2c_driver_probe,
	.id_table    = it6151_i2c_id,
};

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
static DEFINE_MUTEX(it6151_i2c_access);
/**********************************************************
  *
  *   [I2C Function For Read/Write fan5405] 
  *
  *********************************************************/
int it6151_i2c_read_byte(u8 dev_addr, u8 addr, u8 *returnData)
{
	char     cmd_buf[1]={0x00};
	char     readData = 0;
	int      ret=0;

#if 0
#ifndef BUILD_LK
	  printk("[Kernel/LCM] [it6151_read_byte] dev_addr = 0x%x, read_data[0x%x] = 0x%x \n", dev_addr, addr, *returnData);
#else
	  printf("[LK/LCM] [it6151_read_byte] dev_addr = 0x%x, read_data[0x%x] = 0x%x \n", dev_addr, addr, *returnData);
#endif
#endif

	mutex_lock(&it6151_i2c_access);

	if(dev_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE)
	{		
		it6151_mipirx->ext_flag=((it6151_mipirx->ext_flag) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_DIRECTION_FLAG;
		
		cmd_buf[0] = addr;
		ret = i2c_master_send(it6151_mipirx, &cmd_buf[0], (1<<8 | 1));
		
		if (ret < 0)
		{
			it6151_mipirx->ext_flag=0;
		
			mutex_unlock(&it6151_i2c_access);
		
			return 0;
		}
		
		readData = cmd_buf[0];
		*returnData = readData;
		
		it6151_mipirx->ext_flag=0;
	}
	else if(dev_addr == IT6151_EDP_SLAVE_ADDR_WRITE)
	{
		it6151_edp->ext_flag=((it6151_edp->ext_flag) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_DIRECTION_FLAG;

		cmd_buf[0] = addr;
		ret = i2c_master_send(it6151_edp, &cmd_buf[0], (1<<8 | 1));

		if (ret < 0)
		{
			it6151_edp->ext_flag=0;

			mutex_unlock(&it6151_i2c_access);

			return 0;
		}

		readData = cmd_buf[0];
		*returnData = readData;

		it6151_edp->ext_flag=0;
	}
	else
	{
#ifndef BUILD_LK
		printk("[Kernel/LCM] [it6151_i2c_read_byte]error:  no this dev_addr! \n");
#else
		printf("[LK/LCM] [it6151_i2c_read_byte]error:  no this dev_addr! \n");
#endif
	}
	
	mutex_unlock(&it6151_i2c_access);

	return 1;
}

int it6151_i2c_write_byte(u8 dev_addr, u8 addr, u8 writeData)
{
	char    write_data[2] = {0};
	int     ret=0;

#if 0
#ifndef BUILD_LK
	printk("[Kernel/LCM] [it6151_i2c_write] dev_addr = 0x%x, write_data[0x%x] = 0x%x \n", dev_addr, addr, writeData);
#else
	printf("[LK/LCM] [it6151_i2c_write] dev_addr = 0x%x, write_data[0x%x] = 0x%x \n", dev_addr, addr, writeData);
#endif
#endif

	mutex_lock(&it6151_i2c_access);

	write_data[0] = addr;
	write_data[1] = writeData;

	if(dev_addr == IT6151_MIPIRX_SLAVE_ADDR_WRITE)
	{
		it6151_mipirx->addr = IT6151_MIPIRX_SLAVE_ADDR_WRITE;
		it6151_mipirx->ext_flag=((it6151_mipirx->ext_flag) & I2C_MASK_FLAG) | I2C_DIRECTION_FLAG;
		
		ret = i2c_master_send(it6151_mipirx, write_data, 2);
		
		if(ret < 0)
		{
			it6151_mipirx->ext_flag=0;
			mutex_unlock(&it6151_i2c_access);
			
			return 0;
		}
		
		it6151_mipirx->ext_flag=0;
	}
	else if(dev_addr == IT6151_EDP_SLAVE_ADDR_WRITE)
	{
		it6151_edp->addr = IT6151_EDP_SLAVE_ADDR_WRITE;
		it6151_edp->ext_flag=((it6151_edp->ext_flag) & I2C_MASK_FLAG) | I2C_DIRECTION_FLAG;

		ret = i2c_master_send(it6151_edp, write_data, 2);

		if (ret < 0)
		{
			it6151_edp->ext_flag=0;
			mutex_unlock(&it6151_i2c_access);
				
			return 0;
		}

		it6151_edp->ext_flag=0;
	}	
	else
	{
#ifndef BUILD_LK
		printk("[Kernel/LCM] [it6151_i2c_write_byte]error:  no this dev_addr! \n");
#else
		printf("[LK/LCM] [it6151_i2c_write_byte]error:  no this dev_addr! \n");
#endif	
	 
	}

	mutex_unlock(&it6151_i2c_access);

	return 1;
}

static int match_id(const struct i2c_client *client, const struct i2c_device_id *id)
{
	if (strcmp(client->name, id->name) == 0)
		return true;
	else
		return false;
}

static int it6151_i2c_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{
	int err=0; 

#ifndef BUILD_LK
	printk("[Kernel/LCM] [it6151_i2c_driver_probe] start!\n");
#else
	printf("[LK/LCM] [it6151_i2c_driver_probe] start!\n");
#endif	

	if(match_id(client, &it6151_i2c_id[0]))
	{
		if (!(it6151_mipirx = (struct i2c_client *)kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) 
		{
		err = -ENOMEM;
		goto exit;
		}

		memset(it6151_mipirx, 0, sizeof(struct i2c_client));

		it6151_mipirx = client;    
	}
	else if(match_id(client, &it6151_i2c_id[1]))
	{
		if (!(it6151_edp = (struct i2c_client *)kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) 
		{
			err = -ENOMEM;
			goto exit;
		}
		
		memset(it6151_edp, 0, sizeof(struct i2c_client));
	
		it6151_edp = client; 	 
	}
	else
	{
#ifndef BUILD_LK
		printk("[Kernel/LCM] [it6151_i2c_driver_probe] error!\n");
#else
		printf("[LK/LCM] [it6151_i2c_driver_probe] error!\n");
#endif	

		err = -EIO;
		goto exit;
	}

#ifndef BUILD_LK
	printk("[Kernel/LCM] [it6151_i2c_driver_probe] %s i2c sucess!\n", client->name);
#else
	printf("[LK/LCM] [it6151_i2c_driver_probe] %s i2c sucess!\n", client->name);
#endif	

 	return 0;

exit:
 	return err;
}

static int __init it6151_init(void)
{    
	printk("[LCM] [it6151_init] init start\n");

	if(i2c_add_driver(&it6151_i2c_driver)!=0)
	{
#ifndef BUILD_LK
		printk("[Kernel/LCM] [it6151_init] failed to register it6151 i2c driver.\n");
#else
		printf("[LK/LCM] [it6151_init] failed to register it6151 i2c driver.\n");
#endif	
	}
	else
	{
#ifndef BUILD_LK
		printk("[Kernel/LCM] [it6151_init] Success to register it6151 i2c driver.\n");
#else
		printf("[LK/LCM] [it6151_init] Success to register it6151 i2c driver.\n");
#endif	
	}

	return 0;
}

static void __exit it6151_exit(void)
{
 	i2c_del_driver(&it6151_i2c_driver);
}

module_init(it6151_init);
module_exit(it6151_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C it6151 Driver");
MODULE_AUTHOR("James Lo<james.lo@mediatek.com>");
#endif
