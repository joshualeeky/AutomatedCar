/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "MKL25Z4.h"                    // Device header
#include "Motor_Control.h"
#include "LED_Control.h"
#include "myUART.h"
#include "myUltrasound.h"
#include "Audio_Control.h"
#define MASK(x) (1 << (x))

 
/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
volatile unsigned int rx_data = 0x0U;
volatile unsigned int counter = 0;
//volatile unsigned int checker = 0;
 
osThreadId_t motorThreadID;
osThreadId_t brainThreadID;
osThreadId_t LEDThreadID;
osThreadId_t musicThreadID;
osThreadId_t SelfDriveID;
osThreadId_t avoidObstacleID;

osEventFlagsId_t instructionFlag;
osEventFlagsId_t LEDControlFlag;
osEventFlagsId_t MusicControlFlag;
osEventFlagsId_t SelfDriveControlFlag;
 
 const osThreadAttr_t highPriority = {
	.priority = osPriorityHigh
};

const osThreadAttr_t aboveNormalPriority = {
	.priority = osPriorityAboveNormal
};

const osThreadAttr_t realTime = {
	.priority = osPriorityRealtime
};


void UART2_IRQHandler(void) {
	NVIC_ClearPendingIRQ(UART2_IRQn);
	if (UART2->S1 & UART_S1_RDRF_MASK) {
		rx_data = (UART2->D);
		osEventFlagsSet(instructionFlag, 0x01U);
	}
	if (UART2->S1 & (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK)) {
		uint8_t temp = UART2->D;
	}
}

void PORTA_IRQHandler(void) {
	NVIC_ClearPendingIRQ(PORTA_IRQn);
	if ((PORTA->ISFR & MASK(ECHO)) && counter < 0xFF0/*0xDF0*/ && counter > 0xEF0/*0xCF0*/) { // Ensure the wave received was only short amount of time of delay
		osEventFlagsSet(SelfDriveControlFlag, 0x4U);
		//osEventFlagsClear(SelfDriveControlFlag, 0x2U); //Should clear itself
		//checker = counter; //To check how far the object is
		counter = 0xC2271;
	} else {
		counter = 0x4501;
	}
	PORTA->ISFR = 0xffffffff;
}

void self_drive(void *argument) {
	while (1) {
		osEventFlagsWait(SelfDriveControlFlag, 0x2U, osFlagsWaitAll, osWaitForever);
		
		osEventFlagsSet(MusicControlFlag, 0x2U);
		osEventFlagsClear(MusicControlFlag, 0x5U);
		osEventFlagsSet(LEDControlFlag, 0x4U);
		osEventFlagsClear(LEDControlFlag, 0x2U);
		osDelay(100); //To ensure all threads finish
		
		// move forward
		rx_data = 0x10;
		osEventFlagsSet(instructionFlag, 0x01U);
		
		while (1) {
			counter = 0;
			USpulse();
			while (counter < 0x4500) {
				counter++;
			}
			if (counter >= 0xC2271) {
				break;
			}
		}
	}
}

void avoidObstacle(void *argument) {
	static int end = 0;
	while (1) {
		osEventFlagsWait(SelfDriveControlFlag, 0x4U, osFlagsWaitAny, osWaitForever);
		if (!end) { 
			rx_data = 0x1E;
			osEventFlagsSet(instructionFlag, 0x01U);
			osDelay(1000);
			
			// leftTurn 100 deg
			rx_data = 0x14;
			osEventFlagsSet(instructionFlag, 0x01U);
			osDelay(1350);
			
			rx_data = 0x1E;
			osEventFlagsSet(instructionFlag, 0x01U);
			osDelay(1000);
			
			// loop around
			rx_data = 0x1A;
			osEventFlagsSet(instructionFlag, 0x01U);
			osDelay(4800);
			
			rx_data = 0x1E;
			osEventFlagsSet(instructionFlag, 0x01U);
			osDelay(1000);
			
			// leftTurn 90 deg
			rx_data = 0x14;
			osEventFlagsSet(instructionFlag, 0x01U);
			osDelay(1250);
			
			rx_data = 0x1E;
			osEventFlagsSet(instructionFlag, 0x01U);
			osDelay(1000);
			
			end = 1;
			osEventFlagsSet(SelfDriveControlFlag, 0x2U);
			//osEventFlagsClear(SelfDriveControlFlag, 0x4U); //Should clear itself
			
		} else if (end){ 
			// stop
			rx_data = 0x1E;
			osEventFlagsSet(instructionFlag, 0x01U);
			osDelay(100);
			rx_data = 0b100;
			osEventFlagsSet(instructionFlag, 0x01U);
		}
	}
}




void brain(void *argument) {
	static int connected = 0;
	for (;;) {
		osEventFlagsWait(instructionFlag, 0x01U, osFlagsWaitAll, osWaitForever);
		/** Read rx_data and set other thread flags */
		// Movement command
		if (!connected) {
			if (rx_data == 0b00000001) {
			osEventFlagsSet(LEDControlFlag, 0x1U);
			osEventFlagsSet(MusicControlFlag, 0x1U);
			connected = 1;
			}
		} else {
			if (rx_data & 0b00010000) {
				osThreadFlagsSet(motorThreadID, 0x1FU);
				osEventFlagsSet(MusicControlFlag, 0x2U);
				
				 //Stop movement command
				if (rx_data == 0b00011110){
					osEventFlagsSet(LEDControlFlag, 0x2U);
					osEventFlagsClear(LEDControlFlag, 0x4U);
				} 
				
				// Movement in any direction
				else {
					osEventFlagsSet(LEDControlFlag, 0x4U);
					osEventFlagsClear(LEDControlFlag, 0x2U);
				}
			}
			
			//Self-Driving
			else if (rx_data == 0b00000010) {
				osEventFlagsSet(SelfDriveControlFlag, 0x2U); 
			}
			
			// Finished
			else if (rx_data == 0b100) {
				osEventFlagsClear(MusicControlFlag, 0x2U);
				osEventFlagsSet(MusicControlFlag, 0x4U);
				osDelay(100);
			}
		}
	}
}


void music(void *argument) {
	int thisNote = 0;
	while (1) {
		unsigned int instructions = osEventFlagsWait(MusicControlFlag, 0x7U, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
		if (instructions == 0b001) {
			playStart();
			osDelay(500);
			osEventFlagsClear(MusicControlFlag, 0x1U); 
			osEventFlagsSet(MusicControlFlag, 0x2U); 
		}
		else if (instructions == 0b010) {
				playMain(thisNote);
				thisNote += 2;
				thisNote %= 196;
			}
		else if (instructions == 0b100) {
			playCelebrate();
			osEventFlagsClear(MusicControlFlag, 0x4U);
		}
	}
}

void LED(void *argument) {
	for (;;) {
		unsigned int instructions = osEventFlagsWait(LEDControlFlag, 0x7U, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
		
		// Connected
		if (instructions == 0b001) {
			led_start();
			osDelay(500);
			osEventFlagsClear(LEDControlFlag, 0x1U);
			osEventFlagsSet(LEDControlFlag, 0x2U);
		}
		
		// stationary
		else if  (instructions == 0b010) {
			led_stationary();
		}
		
		// moving
		else if (instructions == 0b100) {
			led_moving();
		}
	}
}

void motors(void *argument) {
	int speed = 100;
	while(1) {
	/*
	 Perform necessary if-else checks to compare rx_data with expected values.
	 Take appropriate action if any expected value is received
	 */
		osThreadFlagsWait(0x1FU, osFlagsWaitAny, osWaitForever);
		if (rx_data == 0x10) { //forward 0b00010000
			forward(speed);
		} 
		
		else if (rx_data == 0x12) { // backward 0b00010010
			reverse(speed);
		} 
		
		else if (rx_data == 0x14) { // left turn 0b00010100
			leftTurn(speed);
		} 
		
		else if (rx_data == 0x16) { // right turn 0b00010110
			rightTurn(speed);
		} 
		
		else if (rx_data == 0x18) { // slight left 0b00011000
			fLeft(speed);
		} 
		
		else if (rx_data == 0x1A) { // slight right 0b00011001
			fRight(speed);
		} 
		
		else if (rx_data == 0x1E) { // stop 0b00011110
			stop();
		}
	}
}

int main (void) {
	// System Initialization
	SystemCoreClockUpdate();
	
	// Initialize CMSIS-RTOS
	osKernelInitialize();
	
	// Initialize thread and event Flags
	instructionFlag = osEventFlagsNew(NULL);
	LEDControlFlag = osEventFlagsNew(NULL);
	MusicControlFlag = osEventFlagsNew(NULL);
	SelfDriveControlFlag = osEventFlagsNew(NULL);

	// Initialize registers
	initPWM();
	initUART2(BAUD_RATE);
	initLED();
	initBuzzer();
	initUltrasound();

	// Create new threads
	LEDThreadID = osThreadNew(LED, NULL, NULL);
  brainThreadID = osThreadNew(brain, NULL, &realTime);
	motorThreadID = osThreadNew(motors, NULL, &highPriority);
	musicThreadID = osThreadNew(music, NULL,NULL);
	SelfDriveID = osThreadNew(self_drive, NULL, &aboveNormalPriority);
	avoidObstacleID = osThreadNew(avoidObstacle, NULL, &aboveNormalPriority);

	// Start thread execution
	osKernelStart();                     
	for (;;) {}
}
