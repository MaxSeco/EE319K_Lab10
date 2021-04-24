// ADC.c
// Runs on TM4C123
// Provide functions that initialize ADC0, channel 5, PD2

// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"



// ADC initialization function 
// Initialize ADC for PD2, analog channel 5
// Input: sac sets hardware averaging
// Output: none
// Activating hardware averaging will improve SNR
// Activating hardware averaging will slow down conversion
// Max sample rate: <=125,000 samples/second
// Sequencer 0 priority: 1st (highest)
// Sequencer 1 priority: 2nd
// Sequencer 2 priority: 3rd
// Sequencer 3 priority: 4th (lowest)
// SS3 triggering event: software trigger
// SS3 1st sample source: Ain5 (PD2)
// SS3 interrupts: flag set on completion but no interrupt requested
void ADC_Init(uint32_t sac){ 
	uint32_t delay;
	
	SYSCTL_RCGCGPIO_R |= 0x08;													// 1) Activate clock for Port D
	while (( SYSCTL_PRGPIO_R&0x08) == 0) {};							
	GPIO_PORTD_DIR_R &= ~0x4;														// 2) Make PD2 input
	GPIO_PORTD_AFSEL_R |= 0x4;													// 3) Enable alternate function on PD2
	GPIO_PORTD_DEN_R &= ~0x4;														// 4) Disable digital for PD2
	GPIO_PORTD_AMSEL_R |= 0x4;													// 5) Enable analog functionon PD2
	SYSCTL_RCGCADC_R |= 0x01;														// 6) Activate ADC0
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	delay = SYSCTL_RCGCADC_R;														// Extra time to stabilize
	ADC0_PC_R = 0x01;																		// 7)Configure for 125K max samples/sec
	ADC0_SAC_R = sac;																		// sets hardware averaging
	ADC0_SSPRI_R = 0x0123;															// 8) Seq 3 is highest priority
	ADC0_ACTSS_R &= ~0x0008;														// 9) Disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000;															// 10) seq3 is software trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5;				// 11) Ain5 (PD2)
  ADC0_SSCTL3_R = 0x0006;															// 12) no TS0 D0, yes IE0 END0 
																											// SSCTL specifies the mode of the ADC sample
																											// Bits 3-0 are TS0, IE0, END0, D0 respectively
																											// TS0 = 1 to measure temperature, TS0 = 0 to measure analog voltage
																											// IE0 set for both interrupt and busy-wait synchronization
																											// D0 = 1 to activate differential sampling, such as measuring difference ADC1-ADC0 
																											// D0 will be 0
																											// END0 will always be set when using seq3 signifying thsi sample is end of the sequence
	ADC0_IM_R &= ~0x0008;																// 13) disable SS3 interrupts
  ADC0_ACTSS_R |= 0x0008;															// 14) enable sample sequencer 3
}


//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void){  
	uint32_t data;			
	ADC0_PSSI_R = 0x0008;								// 1) initiate conversatio on sequencer 3 (SS3)
	while((ADC0_RIS_R&0x08) == 0) {};		// 2) wait for conversion. Bit 3 will be set when conversion (averaging) is done.
		data = ADC0_SSFIFO3_R&0xFFF;			// 3) read 12-bit result
		ADC0_ISC_R = 0x0008;							// 4) acknowledge completion
		return data;	
}


