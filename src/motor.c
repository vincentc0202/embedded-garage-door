/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */

#include "stm32l476xx.h"
#include "motor.h"

static const uint32_t MASK = (GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD8 | GPIO_ODR_OD9);

// step1 - step8 is code added by me, following code from Lab 2 (half-stepping motor clockwise)
static const uint32_t step1 = (GPIO_ODR_OD5 | GPIO_ODR_OD9) & ~(GPIO_ODR_OD6 | GPIO_ODR_OD8);
static const uint32_t step2 = GPIO_ODR_OD5 & ~(GPIO_ODR_OD6 | GPIO_ODR_OD8 | GPIO_ODR_OD9);
static const uint32_t step3 = (GPIO_ODR_OD5 | GPIO_ODR_OD8) & ~(GPIO_ODR_OD6 | GPIO_ODR_OD9);
static const uint32_t step4 = GPIO_ODR_OD8 & ~(GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD9);
static const uint32_t step5 = (GPIO_ODR_OD6 | GPIO_ODR_OD8) & ~(GPIO_ODR_OD5 | GPIO_ODR_OD9);
static const uint32_t step6 = GPIO_ODR_OD6 & ~(GPIO_ODR_OD5 | GPIO_ODR_OD8 | GPIO_ODR_OD9);
static const uint32_t step7 = (GPIO_ODR_OD6 | GPIO_ODR_OD9) & ~(GPIO_ODR_OD5 | GPIO_ODR_OD8);
static const uint32_t step8 = GPIO_ODR_OD9 & ~(GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD8);

static const uint32_t HalfStep[8] = {step1, step2, step3, step4, step5, step6, step7, step8};

static volatile int8_t dire = 0;
static volatile uint8_t step = 0;

void Motor_Init(void) {	
	RCC->AHB2ENR |=  RCC_AHB2ENR_GPIOCEN; // enable clock for GPIO Port C
	
	// set pins 5, 6, 8, and 9 on Port C as output (bits 01)
	GPIOC->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE8 | GPIO_MODER_MODE9);
	GPIOC->MODER |= (GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0);
	
	// set output speed of pins to fast (bits 10)
	GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED5 | GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED8 | GPIO_OSPEEDR_OSPEED9);
	GPIOC->OSPEEDR |= (GPIO_OSPEEDR_OSPEED5_1 | GPIO_OSPEEDR_OSPEED6_1 | GPIO_OSPEEDR_OSPEED8_1 | GPIO_OSPEEDR_OSPEED9_1);

	// set output type of pins to push-pull (bit 0)
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT5 | GPIO_OTYPER_OT6 | GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9);
	
	// set pins to no pull-up/no pull-down (bits 00)
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD9);
}

void rotate() {
	GPIOC->ODR &= ~MASK;
	// Clockwise
	if (dire == 1) {	// side note: earlier I had issues bc I wrote "if (dire)" instead of "if (dire == 1)" --> non-zero values in if-statements resolve to true
		step = (step + 1) % 7;
		GPIOC->ODR |= HalfStep[step];
	// Counterclockwise
	} else if (dire == -1) {
		if (step == 0) {
			step = 7;
		}
		else {
			step--;
		}
		GPIOC->ODR |= HalfStep[step];
	}
}

void setDire(int8_t direction) {
	if (direction > 0) {
		dire = 1;	// clockwise rotation
	}
	else if (direction < 0) {
		dire = -1;	// counter clockwise rotation
	}
	else {
		dire = 0; 	// stop
	}
}
	


