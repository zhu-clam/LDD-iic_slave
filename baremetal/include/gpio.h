#ifndef __GPIO_H__
#define __GPIO_H__

/*
gpio0 <--> d7 (led on FPGA board)
gpio1 <--> d8 (led on FPGA board)
gpio2 <--> FPGA_A_PB3 (push button on FPGA board)
gpio3 <--> FPGA_A_PB1 (push button on FPGA board)
gpio4 <--> FPGA_A_PB4 (push button on FPGA board)
*/

#include "datatype.h"

enum{
	GPIO_A = 0,
	GPIO_B,
	GPIO_C,
	GPIO_D
};

void CK_Gpio_Output(unsigned int pin, unsigned char val);
unsigned int CK_Gpio_Input(unsigned int pin);
void dw_Gpio_Output(unsigned int port, unsigned int pin, unsigned char val);
unsigned int dw_Gpio_Input(unsigned int port, unsigned int pin);

void CK_Gpio_Init();
void CK_Gpio_Test_Output();
void CK_Gpio_Test_Input();
void CK_Gpio_Test_Intc();
void CK_Gpio_Test();

#endif