#include "MKL25Z4.h" 
#include "Motor_Control.h"
#include "cmsis_os2.h"
#define ECHO		1
#define TRIGGER	2
#define MASK(x) (1 << (x))
//unsigned int counter = 0;
//volatile int intFlag = 0;
//static int end = 0;

void initUltrasound(void) {
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK);
	
	PORTA->PCR[TRIGGER] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[TRIGGER] = PORT_PCR_MUX(1);
	
	PTA->PDDR |= MASK(TRIGGER);
	PTA->PCOR |= MASK(TRIGGER);
	
	PORTA->PCR[ECHO] &= ~(PORT_PCR_MUX_MASK | PORT_PCR_IRQC_MASK | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK);
	PORTA->PCR[ECHO] |= (
		PORT_PCR_MUX(1) | // GPIO
		PORT_PCR_IRQC(0x0A) //| //1010 Interrupt on falling edge
		//PORT_PCR_PE(1) |
		//PORT_PCR_PS(0)
	);
	
	NVIC_SetPriority(PORTA_IRQn, 3);
  NVIC_ClearPendingIRQ(PORTA_IRQn);
  NVIC_EnableIRQ(PORTA_IRQn);
}

void USpulse() {
	PTA->PSOR |= MASK(TRIGGER);
	osDelay(1);
	PTA->PCOR |= MASK(TRIGGER);
}


