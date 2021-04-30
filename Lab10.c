// Lab10.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/16/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "SSD1306.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "TExaS.h"
#include "Switch.h"
//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PB54                  (*((volatile uint32_t *)0x400050C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PB5       (*((volatile uint32_t *)0x40005080)) 
#define PB4       (*((volatile uint32_t *)0x40005040)) 
//*******************************************************************************************************************************
#define NUM_ENEMIES		5
typedef enum {dead, alive} status_t;
struct sprite {
	int32_t x; 							// x coordinate 0-127
	int32_t y; 							// y coordinate 0-63
	const uint8_t *image;  	// ptr->image
	int32_t vx, vy;					// pixels/50ms
	status_t life;					// dead/alive
};
typedef struct sprite sprite_t;

sprite_t Enemys[NUM_ENEMIES];							
sprite_t Player;						

int NeedToDraw;

void Init(void) {
	Player.x = 116;
	Player.y = 24;
	Player.image = Player1;
	Player.life = alive;
	/*for (int i = 0; i < NUM_ENEMIES; i++) {
		Enemys[i].x = 20*i + 12;
		Enemys[i].y = 10;									// along the top
		Enemys[i].image = AlienBossA;
		Enemys[i]*/
	//Enemys[0].vx = -2;	Enemys[0].vy = 1;
	//Enemys[1].vx = -1;	Enemys[1].vy = 1;
	//Enemys[2].vx = 0;	Enemys[2].vy = 2;
	//Enemys[3].vx = 1;	Enemys[3].vy = 1;
	//Enemys[4].vx = 2;	Enemys[4].vy = 1;
}

void Move(void) {
	if (Player.life == alive) {
			NeedToDraw = 1;
			uint32_t adcData = ADC_In();
			Player.y = 62-((62-8)*adcData/4096);							// maps player position by converting adcData 0-4095 to the screen 0-63 pixels wide
	}

	/*for (int i = 0; i < NUM_ENEMIES; i++) {
		if (Enemys[i].life == alive) {
			NeedToDraw = 1;
			if ((Enemys[i].y > 62) || (Enemys[i].y < 10) ||(Enemys[i].x < 0) || (Enemys[i].x > 118)) {
					Enemys[i].life = dead;
		  } else {
				Enemys[i].x += Enemys[i].vx;	// moves enemy based on its velocity
				Enemys[i].y += Enemys[i].vy;
			}
		}
	}*/
}

void Draw(void) {
	SSD1306_ClearBuffer();
	
	if (Player.life == alive) {
		SSD1306_DrawBMP(Player.x, Player.y, Player.image, 0, SSD1306_INVERSE);
	}
	/*for (int i = 0; i< NUM_ENEMIES; i++) {
		if (Enemy[i].life == alive) {
			SSD1306_DrawBMP(Enemys[i].x, Enemys[i].y, Enemys[i].image, 0, SSD1306_INVERSE);
		}
	}*/
	
	SSD1306_OutBuffer();
	NeedToDraw = 0;
}

// **************SysTick_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(uint32_t period){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = period-1;
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 0x00000007;
}

void SysTick_Handler(void){ 
	PF2 ^= 0x04;     // Heartbeat
	Move();
}
//*******************************************************************************************************************************
// TExaSdisplay logic analyzer shows 7 bits 0,PB5,PB4,PF3,PF2,PF1,0 
// edit this to output which pins you use for profiling
// you can output up to 7 pins
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PB54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// edit this to initialize which pins you use for profiling
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x22;      // activate port B,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF3,2,1
  GPIO_PORTB_DIR_R |=  0x30;   // output on PB4 PB5
  GPIO_PORTB_DEN_R |=  0x30;   // enable on PB4 PB5  
}
//********************************************************************************
 
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

int main(void){
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
	SysTick_Init(4000000); // interrupts every 50ms
	ADC_Init(4);
	Sound_Init();
	Init();
  //SSD1306_ClearBuffer();
  //SSD1306_DrawBMP(2, 62, SpaceInvadersMarquee, 0, SSD1306_WHITE);
  //SSD1306_OutBuffer();
  EnableInterrupts();
	
  //SSD1306_ClearBuffer();
  //SSD1306_DrawBMP(47, 63, PlayerShip0, 0, SSD1306_WHITE); // player ship bottom
  //SSD1306_DrawBMP(53, 55, Bunker0, 0, SSD1306_WHITE);
  //SSD1306_OutBuffer();											// 25 ms

  while(1){
    PF1 ^= 0x02;
		if (NeedToDraw) {
			Draw();
		}
  }
}







