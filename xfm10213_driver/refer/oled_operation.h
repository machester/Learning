#ifndef __OLED_OPERATION_
#define __OLED_OPERATION_

#include "font.h"
#include "oled_driver.h"





void oled_mark_dot(unsigned char x, unsigned y);
void oled_write_horizontal_line(unsigned char line_index, unsigned char start, unsigned char end);
void oled_write_vertical_line(unsigned char vertical_index, unsigned char start, unsigned char end);
void oled_power_on(void);
void oled_init(void);
void oled_clean(void);
int oled_write_cmd(unsigned char cmd);
int oled_write_data(unsigned char data);
void oled_fill(unsigned char data);
void oled_on(void);
void oled_off(void);
void oled_set_pos(unsigned char x, unsigned char y);
void oled_test(void);


#endif
