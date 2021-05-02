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
#include <stdlib.h>
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
#define NUM_PLATFORMS	 	5
#define JUMP_SPEED			-10										// screen rotated 90 degrees to the right so "up" is negative x direction
#define GRAVITY 				1											// gravity towards positive x direction
#define MAX_BULLETS			10
#define BULLET_SPEED		-8
#define SCREEN_WIDTH  	126											// original jump speed -14 gravity 2
#define	SCREEN_HEIGHT		62
#define PLAYER_WIDTH		8
#define PLAYER_HEIGHT		10
#define PLATFORM_WIDTH 	10
#define MAX_FALLING_SPEED	8

typedef enum {dead, alive} status_t;
struct sprite {
	int32_t x; 							// x coordinate 0-127
	int32_t y; 							// y coordinate 0-63
	const uint8_t *image;  	// ptr->image
	int32_t vx, vy;					// pixels/50ms
	status_t life;					// dead/alive
};
typedef struct sprite sprite_t;

sprite_t Player;
sprite_t Bullets[MAX_BULLETS];
sprite_t Platforms[NUM_PLATFORMS];

int NeedToDraw;
int highestPlatformIndex;				// saves the highest/last platform index, so newer platforms can be placed above that only 

// Initializes all game objects at the beginning of game
void Init(void) {
	// Initialize Player
	Player.x = 116;
	Player.y = 24;
	Player.vx = JUMP_SPEED;									// player has initial y velocity to jump
	Player.image = Player1;
	Player.life = alive;
	
	// Initialize Platforms
	Platforms[0].x = (SCREEN_WIDTH*3)/4;			// first platform's coordinates will be determined
	Platforms[0].y = SCREEN_HEIGHT/2;
	Platforms[0].image = Platform1;
	Platforms[0].life = alive;
	highestPlatformIndex = NUM_PLATFORMS-1;
	for (int i = 1; i < NUM_PLATFORMS; i++) {
		Platforms[i].x = Platforms[i-1].x - Random()%24 - 8;     // next platform x coordinate based on the previous
		Platforms[i].y = (Random()%(SCREEN_HEIGHT-PLATFORM_WIDTH))+PLATFORM_WIDTH;
		Platforms[i].image = Platform1;
		Platforms[i].life = alive;
	}
}


// Called by SysTick to move all the objects on screen
// Does not output to the LCD
void Move(uint32_t input) {
	int8_t movePlatforms = 0;								// 1 if platforms move, 0 if player moves.
	// Physics for the Player's y-direction
	if (Player.life == alive) {
			NeedToDraw = 1;
			//uint32_t adcData = ADC_In();
			//Player.y = 62 -((62-8)*adcData/4096);	// adcData 0-4095, screen 0-63 pixels wide, player width 8 pixels. Slide pot now moves player.
			
			if ((input&0x02) == 0x02) { 			// using buttons to move for now because ADC is unreliable
				Player.y += 3;
			} 
			if ((input&0x01) == 0x01) {
				Player.y += -3;
			} 
			
		// physics for the Player's x-direction
			if (Player.x >= 113) {
				Player.x = SCREEN_WIDTH - PLAYER_HEIGHT;
				Player.vx = JUMP_SPEED;
			} 
			if (Player.vx < MAX_FALLING_SPEED) {
					Player.vx += GRAVITY;
			}
			if (Player.x > 65) {														// if player is below a certain line, player falls normally
				Player.x += Player.vx;												
			} else if (Player.vx < 0){											// we enter this "else if" if the player is above a certain line 
				Player.x = 65;
				movePlatforms = 1;
			} else {																				// if player is no longer moving up, player is ready to fall down
				Player.x = 66;																// we no longer need to move the platforms
				movePlatforms = 0;
			}
			
	}
	
	// moves the platforms when player is above a certain line
	if (movePlatforms) {
		for (int i = 0; i < NUM_PLATFORMS; i++) {
			if (Platforms[i].life == alive) {
				Platforms[i].x += abs(Player.vx);
				if (Platforms[i].x > 124) {
					Platforms[i].x = Platforms[highestPlatformIndex].x - Random()%24 - 8;
					Platforms[i].y = (Random()%(SCREEN_HEIGHT-PLATFORM_WIDTH))+PLATFORM_WIDTH;
					highestPlatformIndex = i;
				}
			}
		}
		movePlatforms = 0;
	}
			

	// Moves the bullets
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (Bullets[i].life == alive) {
			NeedToDraw = 1;
			if ((Bullets[i].y > 62) || (Bullets[i].y < 0) ||(Bullets[i].x < 0) || (Bullets[i].x > 123)) {
					Bullets[i].life = dead;
		  } else {
				Bullets[i].x += Bullets[i].vx;	// moves Bullets based on its velocity
				Bullets[i].y += Bullets[i].vy;
			}
		}
	}
}


// Actually outputs to the LCD
void Draw(void) {
	SSD1306_ClearBuffer();

	// draws player
	if (Player.life == alive) {
		SSD1306_DrawBMP(Player.x, Player.y, Player.image, 0, SSD1306_INVERSE);
	}
	
	// draws bullets
	for (int i = 0; i< MAX_BULLETS; i++) {
		if (Bullets[i].life == alive) {
			SSD1306_DrawBMP(Bullets[i].x, Bullets[i].y, Bullets[i].image, 0, SSD1306_INVERSE);
		}
	}
	
	// draws platforms
	for (int i = 0; i < NUM_PLATFORMS; i++) {
		if (Platforms[i].life == alive) {
			SSD1306_DrawBMP(Platforms[i].x, Platforms[i].y, Platforms[i].image, 0, SSD1306_INVERSE);
		}
	}
	
	SSD1306_OutBuffer();
	NeedToDraw = 0;
}

// This function initalizes a bullet at the player's location
// Inputs: x and y velocities of the projectile
// Outputs: None
void Fire(int vx, int vy) {
	// searches bullet array for a dead bullet and bring it to life
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (Bullets[i].life == dead) {
			Bullets[i].x = Player.x;
			Bullets[i].y = Player.y - PLAYER_WIDTH/2;
			Bullets[i].image = Laser2;
			// To prevent bullets moving slower and being under Player
			if (Player.vx < 0) {
				Bullets[i].vx = vx + Player.vx; // bullet is at least as fast as the player
			} else {
				Bullets[i].vx = vx;
			}
			Bullets[i].vy = vy;
			Bullets[i].life = alive;
			return;
		}
	}
}

void Collisions(void) {
	// checks for player collision with platforms
	for (int i = 0; i < NUM_PLATFORMS; i++) {
		if (Platforms[i].life == alive) {
			int xDiff = abs(Player.x + PLAYER_HEIGHT - Platforms[i].x);
			int yDiff = abs((Player.y-PLAYER_WIDTH) - (Platforms[i].y-PLATFORM_WIDTH));
			if ((xDiff < 4) && (yDiff < 5) && Player.vx > 0) {
				Player.vx = JUMP_SPEED;
				return;
			}
		}
	}
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

// Is called every 50ms to update the physics of the game, does not output to LCD
void SysTick_Handler(void){ 
	PF2 ^= 0x04;     // Heartbeat
	static uint32_t lastinput = 0;				// this is to prevent rapid fire when button is held down
	uint32_t input  = Switch_In();
	
	// fires if PE2 is pressed
	if ((input&0x04)  == 0x04 && lastinput == 0) {
		Fire(BULLET_SPEED, 0);
	}
	
	
	Move(input); 			// moves everything on screen
	Collisions(); // checks for collisions
	lastinput = input&0x04;
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
 

int main(void){
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
	SysTick_Init(4000000); // interrupts every 50ms
	Random_Init(NVIC_ST_CURRENT_R);
	ADC_Init(4);				
	//Sound_Init();
	Switch_Init();
	Init();
  //SSD1306_ClearBuffer();
  //SSD1306_DrawBMP(2, 62, SpaceInvadersMarquee, 0, SSD1306_WHITE);
  //SSD1306_OutBuffer();
  EnableInterrupts();
	
  //SSD1306_ClearBuffer();
  //SSD1306_DrawBMP(47, 63, PlayerShip0, 0, SSD1306_WHITE); // player ship bottom
  //SSD1306_DrawBMP(53, 55, Bunker0, 0, SSD1306_WHITE);
  //SSD1306_OutBuffer();											// 25 ms

  while(Player.life == alive){
    PF1 ^= 0x02;
		if (NeedToDraw) {
			Draw();
		}
  }
}







