#include <main.h>

#define Nop() udelay(1) //һ��ָ�����ڣ�1uS��
#define	SlaveAddress	0x70	//IICд��ʱ�ĵ�ַ�ֽ����ݣ�+1Ϊ��ȡ
#define CUBE_ON 	0xFF
#define CUBE_OFF	0x00
#define KEY_VAL_REG		0x01
u8_t ack;

/*******************************************************************
 * ����ԭ��: void start_iic()
 * ����:     ����I2C����,������I2C��ʼ����.
 * ********************************************************************/
void start_iic()
{
	gpio_direction_output(S5P6818_GPIOD(6),1); //SCL OUTPUT 1
	Nop();
	gpio_direction_output(S5P6818_GPIOD(7),1); //SDA OUTPUT 1

	Nop(); //��ʵ��������ʱ�����4.7us����ʱ
	Nop();
	Nop();
	Nop();
	Nop();
	gpio_set_value(S5P6818_GPIOD(7),0); //SDA 0 ������ʼ�ź�
	Nop(); //��ʼ��������4us
	Nop();
	Nop();
	Nop();
	Nop();
	gpio_set_value(S5P6818_GPIOD(6),0); //ǯסI2C���ߣ�׼�����ͻ��������
	Nop();
	Nop();
}
/*******************************************************************
 * ����ԭ��: void stop_iic()
 * ����:     ����I2C����,������I2C��������.
 * ********************************************************************/
void stop_iic()
{
	gpio_direction_output(S5P6818_GPIOD(7),0); //SDA OUTPUT 0
	Nop();
	gpio_direction_output(S5P6818_GPIOD(6),1); //SCL OUTPUT 1

	Nop();   //������������ʱ�����4��s
	Nop();
	Nop();
	Nop();
	Nop();
	gpio_direction_output(S5P6818_GPIOD(7),1); //SDA OUTPUT 1   ����I2C���߽����ź�
	Nop();
	Nop();
	Nop();
	Nop();
}
/******************************************************************
 * ����ԭ��:void send_byte(u8_t c)
 * ����: ������c���ͳ�ȥ,�����ǵ�ַ,Ҳ����������,�����ȴ�Ӧ��,����
 * ��״̬λ���в���.(��Ӧ����Ӧ��ʹack=0 ��)
 * ��������������ack=1; ack=0��ʾ��������Ӧ����𻵡�
********************************************************************/
void send_byte(u8_t c)
{

	u8_t BitCnt;

	Nop();
	for(BitCnt=0;BitCnt<8;BitCnt++) //Ҫ���͵����ݳ���Ϊ8λ
	{
		if(c&0x80)
			gpio_direction_output(S5P6818_GPIOD(7),1);   //�жϷ���λ
		else
			gpio_direction_output(S5P6818_GPIOD(7),0);
		c<<=1;
		Nop();
		gpio_direction_output(S5P6818_GPIOD(6),1);//��ʱ����Ϊ�ߣ�֪ͨ��������ʼ��������λ
		Nop();
		Nop();//��֤ʱ�Ӹߵ�ƽ���ڴ���4��s*/
		Nop();
		Nop();
		Nop();
		gpio_direction_output(S5P6818_GPIOD(6),0);
	}
	Nop();
	Nop();
	gpio_direction_output(S5P6818_GPIOD(7),1);//8λ��������ͷ������ߣ�׼������Ӧ��λ
	Nop();
	Nop();
	gpio_direction_output(S5P6818_GPIOD(6),1);
	gpio_direction_input(S5P6818_GPIOD(7));
	Nop();
	Nop();
	Nop();
if(gpio_get_value(S5P6818_GPIOD(7))==1)
	ack=0;
else
	ack=1;//�ж��Ƿ���յ�Ӧ���ź�
	gpio_direction_output(S5P6818_GPIOD(6),0);
	Nop();
	Nop();
}
/******************************************************************
 * ����ԭ��: u8_t recv_byte()
 * ����: �������մ���������������,���ж����ߴ���(����Ӧ���ź�)
 * ���������Ӧ������
********************************************************************/
u8_t recv_byte()
{
	u8_t retc = 0;
	u8_t BitCnt;

	gpio_direction_output(S5P6818_GPIOD(7),1);
	gpio_direction_input(S5P6818_GPIOD(7));//��������Ϊ���뷽ʽ
	Nop();
	for(BitCnt=0;BitCnt<8;BitCnt++){
		Nop();
		gpio_direction_output(S5P6818_GPIOD(6),0);//��ʱ����Ϊ�ͣ�׼����������λ
		Nop();
		Nop();//ʱ�ӵ͵�ƽ���ڴ���4.7��s
		Nop();
		Nop();
		Nop();
		gpio_direction_output(S5P6818_GPIOD(6),1);//��ʱ����Ϊ��ʹ��������������Ч
		Nop();
		Nop();
		retc=retc<<1;
		if(gpio_get_value(S5P6818_GPIOD(7))==1)
			retc=retc+1;//������λ,���յ�����λ����retc��
		Nop();
		Nop();
	}
	gpio_direction_output(S5P6818_GPIOD(6),0);
	Nop();
	Nop();
	return(retc);
}
/********************************************************************
 * ԭ��:void ack_iic(u8_t a)
 * ���ܣ�����������Ӧ���ź�,(������Ӧ����Ӧ���ź�)
 * ���룺Ϊ0Ӧ��Ϊ1��Ӧ��
********************************************************************/
void ack_iic(u8_t a)
{

	Nop();
	if(a==0)
		gpio_direction_output(S5P6818_GPIOD(7),0);//�ڴ˷���Ӧ����Ӧ���ź�
	else
		gpio_direction_output(S5P6818_GPIOD(7),1);
	Nop();
	Nop();
	Nop();
	gpio_direction_output(S5P6818_GPIOD(6),1);
	Nop();
	Nop();//ʱ�ӵ͵�ƽ���ڴ���4��s
	Nop();
	Nop();
	Nop();
	gpio_direction_output(S5P6818_GPIOD(6),0);//��ʱ���ߣ�ǯסI2C�����Ա��������
	Nop();
	Nop();
}
/*******************************************************************
 * ����ԭ��: u8_t isend_str(uint8 sla,uint8 *s,uint8 no)
 * ����:���������ߵ����͵�ַ���ӵ�ַ,���ݣ��������ߵ�ȫ����,������
 * ��ַsla����������sָ������ݣ�����no���ֽڡ�
 * �������1��ʾ�����ɹ��������������
 * ���� sla-��������ַ��s-Ҫ���͵����ݣ�no-Ҫ���͵����ݵĸ���ע��
 * 1��ʹ��ǰ�����ѽ�������
 * 2��������������ֵ�ַΪ���ֽ����͵������е�һ������Ϊ�ӵ�ַ������ӵ�ַΪ˫�ֽ���ǰ��������Ϊ�ӵ�ַ��
********************************************************************/
u8_t isend_str(u8_t sla,u8_t *s,u8_t no)
{
	u8_t i;

	start_iic();//��������
	send_byte(sla);//����������ַ
	if(ack==0){
		serial_printf(0, "ackisend %02x\r\n",ack );
		return(0);
	}
	for(i=0;i<no;i++){
		send_byte(*s);//��������
		if(ack==0)
			return(0);
		s++;
	}
	stop_iic();//��������
	return(1);
}
/*******************************************************************
//����ԭ��:u8_t irecv_str(u8_t sla,u8_t *suba,u8_t *s,u8_t subano,u8_t no)
//����:���������ߵ����͵�ַ���ӵ�ַ,�����ݣ��������ߵ�ȫ����,������
//��ַsla,�ӵ�ַsuba�����������ݷ���sָ��Ĵ洢������no���ֽ�
//�������1��ʾ�����ɹ��������������
//����sla-��������ַ
//suba-�ӵ�ַ��ָ�ĵ�ַ
//s-�������ݵĵ�ַ
//subano-�ӵ�ַ���ֽڸ���
//noҪ��ȡ�����ݸ�
//ע��
//1��ʹ��ǰ�����ѽ�������
********************************************************************/
u8_t irecv_str(u8_t sla,u8_t *suba,u8_t *s,u8_t subano,u8_t no)
{
	u8_t i;
	start_iic();//��������
	send_byte(sla);//����������ַ
	if(ack==0){
		serial_printf(0, "ackirecv1 %02x\r\n",ack );
		return(0);
	}
	for(i=0;i<subano;i++){
		send_byte(*suba);//���������ӵ�ַ
		if(ack==0){
			serial_printf(0, "ackirecv2 %02x\r\n",ack );
			return(0);
		}
		suba++;
	}
	start_iic();
	send_byte(sla+1);
	if(ack==0){
		serial_printf(0, "ackirecv3 %02x\r\n",ack );
		return(0);
	}
	for(i=0;i<no-1;i++){
		*s=recv_byte();//��������
		ack_iic(0);//���;ʹ�λ
		s++;
	}
	*s=recv_byte();
	ack_iic(1);//���ͷ�Ӧλ
	stop_iic();//��������
	return(1);
}
unsigned char switch_value(unsigned char num)
{

	unsigned char cube_val;
	switch (num) {
		case 0x01: //key =0, 7led=0xfc
			cube_val = 0xfc;
			break;
		case 0x02: //key = 1,7led=0x0c
			cube_val = 0x60;
			break;
		case 0x03: //key = 2,7led=0xda
			cube_val = 0xda;
			break;
		case 0x04: //key = 3,7led=0xf2
			cube_val = 0xf2;
			break;
		case 0x09: //key = 4,7led=0x66
			cube_val = 0x66;
			break;
		case 0x0a: //key = 5,7led=0xb6
			cube_val = 0xb6;
			break;
		case 0x0b: //key = 6,7led=0xbe
			cube_val = 0xbe;
			break;
		case 0x0c: //key = 7,7led=0xe0
			cube_val = 0xe0;
			break;
		case 0x11: //key = 8,7led=0xfe
			cube_val = 0xfe;
			break;
		case 0x12: //key = 9,7led=0xf6
			cube_val = 0xf6;
			break;
		case 0x13: //key = A,7led=0xEE
			cube_val = 0xee;
			break;
		case 0x14: //key = b,7led=0x3E
			cube_val = 0x3e;
			break;
		case 0x19: //key = C,7led=0x9C
			cube_val = 0x9c;
			break;
		case 0x1a: //key = d,7led=0x9A
			cube_val = 0x7a;
			break;
		case 0x1b: //key = e,7led=0xDE
			cube_val = 0x9e;
			break;
		case 0x1c: //key = F,7led=0x8E
			cube_val = 0x8e;
			break;
		default:
			break;
	}
	return cube_val;
}

void set_7led_value(u8_t addr, u8_t cube_val)
{
	u8_t status = 0x00;
	u8_t s[2] = {addr,cube_val};

	irecv_str(SlaveAddress,&addr,&status,1,1);

	while(status != cube_val) {
		mdelay(1);
		isend_str(SlaveAddress,s,2);
		irecv_str(SlaveAddress,&addr,&status,1,1);

	}
}
/*
 * ͨ����ȡZLG7290��ż�ֵ�ļĴ�����õ�ǰ���µļ�ֵ
 *
 * */
unsigned char get_key_value(unsigned char addr)
{
	unsigned char value = 0;
	while(!value) {
		mdelay(1);
		irecv_str(SlaveAddress,&addr,&value,1,1);
	}

	if (value != 0)
		return value;

	return -1;
}
/*-------------------------MAIN FUNCTION------------------------------*/
/**********************************************************************
 * @brief		Main program body
 * @param[in]	None
 * @return 		int
 **********************************************************************/
int tester_i2c(int argc, char * argv[])
{
	int  i ;
	unsigned char reg[8]; //�������ܼĴ�����ַ
	unsigned char cube_val; //���ת���������
	unsigned char key_val;


	//��������ΪGPIOģʽ
	gpio_set_cfg(S5P6818_GPIOD(6), 0x0);        //GPIOD6, I2C_2_SCL
	gpio_set_cfg(S5P6818_GPIOD(7), 0x0);		//GPIOD7, I2c_2_SDA
	mdelay(100);

	/*
	 * ��ʼ������ܼĴ�����ַ
	 * */
	for (i = 0; i < 8; i++)
		reg[i]= 0x10 + i;

	//switch_value(buf,cube_val);

	//�������е������
	for(i = 0; i < 8; i++)
		set_7led_value(reg[i], CUBE_ON);
	mdelay(500);
	//�ر����е������
	for(i = 0; i < 8; i++)
		set_7led_value(reg[i], CUBE_OFF);

	while(1){
		key_val = get_key_value(KEY_VAL_REG);
		cube_val = switch_value(key_val);
		for(i = 0; i < 8; i++)
			set_7led_value(reg[i], cube_val);
	}


	return 0;
}
