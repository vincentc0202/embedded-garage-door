/*
 * ECE 153B
 *
 * Name(s): Jason & Vincent
 * Section:
 * Project
 */

#include "stm32l476xx.h"
#include "SysClock.h"
#include "SysTimer.h"
#include "LED.h"
#include "DMA.h"
#include "UART.h"
#include "motor.h"
#include "SPI.h"
#include "I2C.h"
#include "accelerometer.h"
#include <stdio.h>
#include <stdbool.h> 

enum State {
	CLOSED = 0,
	OPENED = 1,
	CLOSING = 2,
	OPENING = 3,
	STOPPED = 4
};

static char buffer[IO_SIZE];
static enum State state = CLOSED;
static int8_t counter = 0;
static bool wait = false;

// initialize accelerometer values
double x,y,z;

void UART_onInput(char* inputs, uint32_t size) {
	// From part c, the door should stay in that state for at least 3s before reverting
	// notice that counter is set to 30, because delay(100) causes counter++ to increment every 0.1 sec
	if (wait) {
		sprintf(buffer, "Door on cooldown. Wait %d seconds\n", 30 - counter);
	}
	
	if (inputs[0] == '1') {
		if (state == OPENED) {
			sprintf(buffer, "Door already opened.\n\n");
		}
		else {
			setDire(-1);
			state = OPENING;
			counter = 0;
			wait = true;
			sprintf(buffer, "Door opening.\n\n");	
		}
	}
	else if (inputs[0] == '2') {
		if (state == CLOSED) {
			sprintf(buffer, "Door already closed.\n\n");
		}
		else {
			setDire(1);				
			state = CLOSING;
			counter = 0;
			wait = true;
			sprintf(buffer, "Door closing.\n\n");
		}
	}
	else if (inputs[0] == '0') {
		setDire(0);
		state = STOPPED;
		counter = 0;
		wait = true;
		sprintf(buffer, "Door stopped.\n\n");
	}
	else {
		setDire(0);
		sprintf(buffer, "Invalid input.\n\n");
	}
	
	UART_print(buffer);
}

const char* printState (enum State s) {
    switch (s) {
        case CLOSED:
            return "Closed";
        case OPENED:
            return "Opened";
        case CLOSING:
            return "Closing";
        case OPENING:
            return "Opening";
				case STOPPED:
						return "Stopped";
    }
}

// SPI is used for accelerometer
// I2C is used for temperature sensor
// UART is used for the Termite terminal / bluetooth
int main(void) {
	// Switch System Clock = 80 MHz
	System_Clock_Init(); 
	Motor_Init();
	SysTick_Init();
	
	UART1_Init();	// bluetooth
	//UART2_Init();		// termite
	
	LED_Init();
	
	SPI1_GPIO_Init();
	SPI1_Init();
	initAcc();
	
	I2C_GPIO_Init();
	I2C_Initialization();
	
	// the general scheme is sprintf() then UART_print()
	// sprintf() loads the string into the buffer, then UART_print() actually prints the value in the buffer
	sprintf(buffer, "Program Started. 1 to open, 2 to close, and 0 to stop the door.\n\n");
	UART_print(buffer);
	
	// Variables for I2C temperature
	uint8_t SecondaryAddress;
	uint8_t Data_Receive;
	uint8_t Data_Send;
	// CHECK VALUES
	uint8_t highTemp = 30;
	uint8_t lowTemp = 23;

	// initially read from accelerometer
	readValues(&x, &y, &z);
	
	uint8_t PrintCounter = 0; // used to measure 10 cycles of delay (each delay is 0.1 sec) to print every second

	while(1) {
		// Read temp with I2C
		SecondaryAddress = 0b1001000 << 1;
		Data_Send = 0; // in the tc74 data sheet, the command byte for reading temp is 0x00

		I2C_SendData(I2C1, SecondaryAddress, &Data_Send, 1);
		I2C_ReceiveData(I2C1, SecondaryAddress, &Data_Receive, 1);

		// Read accelerometer with SPI
		readValues(&x, &y, &z);
		
		// Opening door based on temperature reading
		if (state == CLOSED) {
			// check if temp is greater than threshold, if so then open door	
			if (Data_Receive >= highTemp && !wait) {
					state = OPENING;			
					setDire(-1);
					wait = true;
					sprintf(buffer, "Temperature too high, opening door.\n");
					UART_print(buffer);
				}
		}
		else if (state == OPENED) {
			// if temp is lower than threshold, then begin closing the door
			if (Data_Receive <= lowTemp && !wait) {
					state = CLOSING;
					setDire(1);
					wait = true;
					sprintf(buffer, "Temperature too low, closing door.\n");
					UART_print(buffer);
			}
		}
		else if (state == OPENING) {
			// check accelerometer z-axis, if it surpasses a certain value, then the door is fully opened
			if (z >= 0.9) {
					state = OPENED;
					setDire(0);
					sprintf(buffer, "Door opened.\n");
					UART_print(buffer);
			}
		}
		else if (state == CLOSING) {
			// check if accelerometer z-axis is within a "closed" range (nearly vertical)
			if (z <= 0.05) {
					state = CLOSED;
					setDire(0);
					sprintf(buffer, "Door closed.\n");
					UART_print(buffer);
			}
		}
		if (wait) {
			if (counter >= 30) {
					counter = 0;
					wait = false;
					sprintf(buffer, "Door cooldown finished.\n");
					UART_print(buffer);
			}
			else {
					counter++;
			}
		}
		
		PrintCounter++;
		
		// Print values
		if (PrintCounter >= 10) {
			sprintf(buffer, "Temperature: %i\n Acceleration: %.2f, %.2f, %.2f\n State: %s\n\n", Data_Receive, x, y, z, printState(state));
			UART_print(buffer);	
			PrintCounter = 0;
		}
		
		LED_Toggle();
		delay(100);
	}
}


