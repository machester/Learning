#include "oled_operation.h"


#define ssd1306_cmd_addr          ((unsigned char)0x00)
#define ssd1306_data_addr         ((unsigned char)0x40)

#define MAX_WIDTH				 128
#define MAX_HEIGHT				 64

void oled_mark_dot(unsigned char x, unsigned y)
{

}

void oled_write_horizontal_line(unsigned char line_index , unsigned char start, unsigned char end)
{

}


void oled_write_vertical_line(unsigned char vertical_index , unsigned char start, unsigned char end)
{

}

void oled_power_on(void)
{
	oled_init();
}

void oled_init(void)
{
 	mdelay(200);
	oled_write_cmd(0xAE); 	//display off
	oled_write_cmd(0x20);	//Set Memory Addressing Mode
    oled_write_cmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid    
    oled_write_cmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7    
    oled_write_cmd(0xc8);	//Set COM Output Scan Direction
	oled_write_cmd(0x00); 	//---set low column address
	oled_write_cmd(0x10); 	//---set high column address
	oled_write_cmd(0x40); 	//--set start line address
	oled_write_cmd(0x81); 	//--set contrast control register
	oled_write_cmd(0xff); 	// black light 0x00~0xff
	oled_write_cmd(0xa1); 	//--set segment re-map 0 to 127
	oled_write_cmd(0xa6); 	//--set normal display
	oled_write_cmd(0xa8); 	//--set multiplex ratio(1 to 64)
	oled_write_cmd(0x3F); 	//
	oled_write_cmd(0xa4); 	//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	oled_write_cmd(0xd3); 	//-set display offset
	oled_write_cmd(0x00); 	//-not offset
	oled_write_cmd(0xd5); 	//--set display clock divide ratio/oscillator frequency
	oled_write_cmd(0xf0); 	//--set divide ratio
	oled_write_cmd(0xd9); 	//--set pre-charge period
	oled_write_cmd(0x22); 	//
	oled_write_cmd(0xda); 	//--set com pins hardware configuration
	oled_write_cmd(0x12);
	oled_write_cmd(0xdb); 	//--set vcomh
	oled_write_cmd(0x20); 	//0x20,0.77xVcc
	oled_write_cmd(0x8d); 	//--set DC-DC enable
	oled_write_cmd(0x14); 	//
	oled_write_cmd(0xaf); 	//--turn on oled panel

    

}

void oled_clean(void)
{
    oled_fill(0x00);
}

int oled_write_cmd(unsigned char cmd)
{
    int ret;
    ret = oled_i2c_send_byte(oled_i2c_info->oled_client, ssd1306_cmd_addr, cmd);
    if(ret < 0)
         err_msg("---> send cmd, client_addr = 0x%x, sub_addr = 0x%x, cmd = 0x%x, ret = %d",
         oled_i2c_info->oled_client->addr, ssd1306_cmd_addr, cmd, ret);
    return ret;
}

int oled_write_data(unsigned char data)
{
    int ret;
    ret = oled_i2c_send_byte(oled_i2c_info->oled_client, ssd1306_data_addr, data);
    if (ret < 0)
        err_msg("--->send data, client_addr = 0x%x, sub_addr = 0x%x, data = 0x%x, ret = %d",
        oled_i2c_info->oled_client->addr, ssd1306_data_addr, data, ret);
    return ret;
}

void oled_fill(unsigned char data)
{
    unsigned char index, loop;
    for (index = 0; index < 8; index++)
    {
        oled_write_cmd(0xb0 + index);   //page0-page1
        oled_write_cmd(0x00);           //low column start address
        oled_write_cmd(0x10);           //high column start address
        for (loop = 0; loop < 128; loop++)
            oled_write_data(data);
    }
}

void oled_on(void)
{
    oled_write_cmd(0x8d);
    oled_write_cmd(0x14);
    oled_write_cmd(0xaf);
}

void oled_off(void)
{
    oled_write_cmd(0x8d);
    oled_write_cmd(0x10);
    oled_write_cmd(0xae);
}

void oled_set_pos(unsigned char x, unsigned char y)
{
    oled_write_cmd(0xb0 + y);
    oled_write_cmd(((x & 0xf0) >> 4) | 0x10);
    oled_write_cmd((x & 0x0f));
}

void oled_test(void)
{
    int index;
	for (index = 0; index < 3; index++) {
		oled_fill(0xff);
		mdelay(2000);
		oled_fill(0x00);
		mdelay(2000);
	}
}

