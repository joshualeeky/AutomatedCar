#include "MKL25Z4.h"
#include "cmsis_os2.h"
#define GREEN_LED_1 11 // PortC Pin 7
#define GREEN_LED_2 10 // PortC Pin 0
#define GREEN_LED_3 6  // PortC Pin 3
#define GREEN_LED_4 5  // PortC Pin 4
#define GREEN_LED_5 4  // PortC Pin 5
#define GREEN_LED_6 3  // PortC Pin 6
#define GREEN_LED_7 0  // PortC Pin 10
#define GREEN_LED_8 7  // PortC Pin 11
#define RED_LED 2    // portC Pin 1
#define MASK(x) (1 << (x))

void initLED() {
	// Enable Clock to PORTB and PORTD
	SIM->SCGC5 |= (SIM_SCGC5_PORTC_MASK);
	
	// Configure MUX settings to make all 3 pins GPIO
	PORTC->PCR[GREEN_LED_1] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[GREEN_LED_1] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[GREEN_LED_2] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[GREEN_LED_2] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[GREEN_LED_3] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[GREEN_LED_3] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[GREEN_LED_4] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[GREEN_LED_4] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[GREEN_LED_5] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[GREEN_LED_5] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[GREEN_LED_6] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[GREEN_LED_6] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[GREEN_LED_7] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[GREEN_LED_7] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[GREEN_LED_8] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[GREEN_LED_8] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[RED_LED] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[RED_LED] |= PORT_PCR_MUX(1);
	
	// Set Data Direction Registers for PortC
	PTC->PDDR |= MASK(GREEN_LED_1);
	PTC->PDDR |= MASK(GREEN_LED_2);
	PTC->PDDR |= MASK(GREEN_LED_3);
	PTC->PDDR |= MASK(GREEN_LED_4);
	PTC->PDDR |= MASK(GREEN_LED_5);
	PTC->PDDR |= MASK(GREEN_LED_6);
	PTC->PDDR |= MASK(GREEN_LED_7);
	PTC->PDDR |= MASK(GREEN_LED_8);
	PTC->PDDR |= MASK(RED_LED);	
	
	PTC->PCOR |= MASK(GREEN_LED_1);
	PTC->PCOR |= MASK(GREEN_LED_2);
	PTC->PCOR |= MASK(GREEN_LED_3);
	PTC->PCOR |= MASK(GREEN_LED_4);
	PTC->PCOR |= MASK(GREEN_LED_5);
	PTC->PCOR |= MASK(GREEN_LED_6);
	PTC->PCOR |= MASK(GREEN_LED_7);
	PTC->PCOR |= MASK(GREEN_LED_8);
	PTC->PCOR |= MASK(RED_LED);
}


void red_toggle() {
	PTC->PTOR |= MASK(RED_LED);
}

void green_moving(int count){
	switch(count) {
		case 0:
			PTC->PSOR |= MASK(GREEN_LED_1);
			break;
		case 1:
			PTC->PSOR |= MASK(GREEN_LED_2);
			break;
		case 2:
			PTC->PSOR |= MASK(GREEN_LED_3);
			break;
		case 3:
			PTC->PSOR |= MASK(GREEN_LED_4);
			break;
		case 4:
			PTC->PSOR |= MASK(GREEN_LED_5);
			break;
		case 5:
			PTC->PSOR |= MASK(GREEN_LED_6);
			break;
		case 6:
			PTC->PSOR |= MASK(GREEN_LED_7);
			break;
		case 7:
			PTC->PSOR |= MASK(GREEN_LED_8);
			break;
	}
}

void red_off() {
	PTC->PCOR |= MASK(RED_LED);
}

void green_on(){
	PTC->PSOR |= MASK(GREEN_LED_1);
	PTC->PSOR |= MASK(GREEN_LED_2);
	PTC->PSOR |= MASK(GREEN_LED_3);
	PTC->PSOR |= MASK(GREEN_LED_4);
	PTC->PSOR |= MASK(GREEN_LED_5);
	PTC->PSOR |= MASK(GREEN_LED_6);
	PTC->PSOR |= MASK(GREEN_LED_7);
	PTC->PSOR |= MASK(GREEN_LED_8);
}


void green_off(){
	PTC->PCOR |= MASK(GREEN_LED_1);
	PTC->PCOR |= MASK(GREEN_LED_2);
	PTC->PCOR |= MASK(GREEN_LED_3);
	PTC->PCOR |= MASK(GREEN_LED_4);
	PTC->PCOR |= MASK(GREEN_LED_5);
	PTC->PCOR |= MASK(GREEN_LED_6);
	PTC->PCOR |= MASK(GREEN_LED_7);
	PTC->PCOR |= MASK(GREEN_LED_8);
}

void led_start(){
	red_off();
	green_on();
	osDelay(500);	
	green_off();
	osDelay(500);	
	green_on();
	osDelay(500);	
	green_off();
	osDelay(500);
}

void led_moving(){
	static int count = 0;
	green_off();
	red_toggle();
	green_moving(count);
	osDelay(500);
	count++;
	count %= 8;
}

void led_stationary(){
	green_on();
	red_toggle();
	osDelay(250);
}


