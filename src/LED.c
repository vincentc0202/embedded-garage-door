/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */

#include "LED.h"

void LED_Init(void) {
	// Enable GPIO Clocks
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //PA5 for LED 
	
	// Initialize Green LED
  GPIOA->MODER &= ~GPIO_MODER_MODE5; // setting LED mode to output (01)
	GPIOA->MODER |= GPIO_MODER_MODE5_0;
		
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;	// setting OT5 to 0 by doing reset operation
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5;	// setting LED to no pull-up / pull-down
}

void LED_On(void){
	GPIOA->ODR |= GPIO_ODR_OD5;
}

void LED_Off(void){
	GPIOA->ODR &= ~GPIO_ODR_OD5;
}

void LED_Toggle(void){
	GPIOA->ODR ^= GPIO_ODR_OD5;
}