#include "MKL25Z4.h"
#include "cmsis_os2.h"

#define PTB0_Pin 0 // TPM1_CH0
#define PTB1_Pin 1 // TPM1_CH1
#define leftMotors TPM1_C1V // Green Wire, chip 1 A1 ? Vig switched TPM1_C0V and TPM1_C1V so not sure if wire color comment is still correct.
#define rightMotors TPM1_C0V // Blue Wire, chip 1 B1 ? Vig switched TPM1_C0V and TPM1_C1V so not sure if wire color comment is still correct.
#define PTB2_Pin 2 // TPM2_CH0
#define PTB3_Pin 3 // TPM2_CH1
#define leftRevMotors TPM2_C1V // Yellow Wire, chip 2 A1 ? Vig switched TPM2_C0V and TPM2_C1V so not sure if wire color comment is still correct.
#define rightRevMotors TPM2_C0V // Orange Wire, chip 2 B1 ? Vig switched TPM2_C0V and TPM2_C1V so not sure if wire color comment is still correct.

void initPWM(void){
	
	// Enable Clock Gating for PORTB
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
	// Configure Mode ALT3 for the PWM pin operation - Pg 162 & 184
	PORTB->PCR[PTB0_Pin] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[PTB0_Pin] |= PORT_PCR_MUX(3);
	
	PORTB->PCR[PTB1_Pin] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[PTB1_Pin] |= PORT_PCR_MUX(3);
	
	// Enable Clock Gating for Timers - Pg 208
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK; // Enable clock gate for TPM1 module
	
	// Select clock for TPM module - Pg 196
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // MCGFLLCLK clock or MCGPLLCLK/2 selected
	
	// Set Modulo Value [(Clk Speed)/(Prescalar)]/MOD = PWM Frequency
	// 48MHz/128 = 375kHz/3750 = 100Hz << Something wrong with calculations, currently f = 82Hz
	// The TPM period when using up counting is (MOD + 0x0001) × period of the TPM counter clock.
	TPM1->MOD = 2000; // Pg 554
	
	// Edge-Aligned PWM - Pg 553
	TPM1->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
	TPM1->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7)); // LPTPM counter increments on every LPTPM counter clock, Prescalar = 128
	TPM1->SC &= ~(TPM_SC_CPWMS_MASK); 
	TPM1->SC |= TPM_SC_CPWMS(0); // LPTPM counter operates in up counting mode.
	
	// Enable PWM on TPM1 Channel 0 & 1 -> PTB0 - Pg 556
	TPM1_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
	TPM1_C0SC |= (TPM_CnSC_MSB(1) | TPM_CnSC_MSA(0) | TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0)); // Edge-aligned PWM, High-true pulses (clear Output on match, set Output on reload)
	
	TPM1_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
	TPM1_C1SC |= (TPM_CnSC_MSB(1) | TPM_CnSC_MSA(0) | TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0)); // Edge-aligned PWM, High-true pulses (clear Output on match, set Output on reload)


	// Code for Timer module 2
	
	// Configure Mode ALT3 for the PWM pin operation - Pg 162 & 184
	PORTB->PCR[PTB2_Pin] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[PTB2_Pin] |= PORT_PCR_MUX(3);
	
	PORTB->PCR[PTB3_Pin] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[PTB3_Pin] |= PORT_PCR_MUX(3);
	
	// Enable Clock Gating for Timers - Pg 208
	SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK; // Enable clock gate for TPM2 module
	
	// Set Modulo Value [(Clk Speed)/(Prescalar)]/MOD = PWM Frequency
	TPM2->MOD = 2000; 
	
	// Edge-Aligned PWM - Pg 553
	TPM2->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
	TPM2->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7)); // LPTPM counter increments on every LPTPM counter clock, Prescalar = 128
	TPM2->SC &= ~(TPM_SC_CPWMS_MASK); 
	TPM2->SC |= TPM_SC_CPWMS(0);// LPTPM counter operates in up counting mode.
	
	// Enable PWM on TPM1 Channel 0 -> PTB0 - Pg 556
	TPM2_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
	TPM2_C0SC |= (TPM_CnSC_MSB(1) | TPM_CnSC_MSA(0) | TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0)); // Edge-aligned PWM, High-true pulses (clear Output on match, set Output on reload)
	
	TPM2_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
	TPM2_C1SC |= (TPM_CnSC_MSB(1) | TPM_CnSC_MSA(0) | TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0)); // Edge-aligned PWM, High-true pulses (clear Output on match, set Output on reload)

}


void forward(int speed){
	leftRevMotors = 0;
	rightRevMotors = 0;
	leftMotors = speed*(1700/100);
	rightMotors = speed*(1800/100);
}

void reverse(int speed){
	leftMotors = 0;
	rightMotors = 0;
	leftRevMotors = speed*(1300/100);
	rightRevMotors = speed*(1300/100);
}

void stop(void){
	leftRevMotors = 0;
	rightRevMotors = 0;
	leftMotors = 0;
	rightMotors = 0;
}

void leftTurn(int speed){
	leftRevMotors = speed*(500/100);
	rightRevMotors = 0;
	leftMotors = 0;
	rightMotors = speed*(800/100);
}

void rightTurn(int speed){
	leftRevMotors = 0;
	rightRevMotors = speed*(500/100);
	leftMotors = speed*(800/100);
	rightMotors = 0;
}

void fRight(int speed){
	leftRevMotors = 0;
	rightRevMotors = 0;
	leftMotors = speed*(1400/100);
	rightMotors = speed*(310/100);
}

void fLeft(int speed){
	leftRevMotors = 0;
	rightRevMotors = 0;
	leftMotors = speed*(310/100);
	rightMotors = speed*(1400/100);
}
