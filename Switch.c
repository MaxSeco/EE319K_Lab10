// Switch.c
// This software to input from switches or buttons
// Runs on TM4C123
// Program written by: James Mahon and Ngoc Dao
// Date Created: 3/6/17 
// Last Modified: 1/14/21
// Lab number: 10
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// Initialize port E for buttons
void Switch_Init() {
	SYSCTL_RCGCGPIO_R |= 0x10; 		// activate port E
	__asm__{
		NOP
		NOP
	}
		
	GPIO_PORTE_DIR_R &= ~(0x0F);      // make PE3-0 in
  GPIO_PORTE_DEN_R |= 0x0F;         // enable digital I/O on PE3-0
}

// returns data from port E
uint32_t Switch_In(void){ 
  return GPIO_PORTE_DATA_R&0x0F;
}
