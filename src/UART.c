/*
 * ECE 153B
 *
 * Name(s): Jason & Vincent
 * Section:
 * Project
 */


#include "UART.h"
#include "DMA.h"
#include <string.h>
#include <stdio.h>

static volatile DMA_Channel_TypeDef * tx;
static volatile char inputs[IO_SIZE];
static volatile uint8_t data_t_0[IO_SIZE];
static volatile uint8_t data_t_1[IO_SIZE];
static volatile uint8_t input_size = 0;
static volatile uint8_t pending_size = 0;
static volatile uint8_t * active = data_t_0;
static volatile uint8_t * pending = data_t_1;

#define SEL_0 1
#define BUF_0_EMPTY 2
#define BUF_1_EMPTY 4
#define BUF_0_PENDING 8
#define BUF_1_PENDING 16

void transfer_data(char ch);
void on_complete_transfer(void);

// UART1 is for Bluetooth, UART2 is for Termite
void UART1_Init(void) {
	UART1_GPIO_Init();
	
	// Enable USART1 clock
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN; 
	
	// Select system clock for USART1 peripheral independent clock configuration register (01)
	RCC->CCIPR &= ~RCC_CCIPR_USART1SEL;
	RCC->CCIPR |= RCC_CCIPR_USART1SEL_0;
	
	// Select DMA 1 channel 4, because Channel 4 is mapped to USART1_TX (why do we use USART1_TX instead of RX?)
	tx = DMA1_Channel4;
	// Initialize DMA with bluetooth
	DMA_Init_UARTx(DMA1_Channel4, USART1);
	tx->CMAR =(uint32_t)active;

	DMA1_CSELR->CSELR &= ~DMA_CSELR_C4S;
	DMA1_CSELR->CSELR |= DMA_CSELR_C4S & 0x222222222;
	
	USART_Init(USART1);
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 0);
}

void UART2_Init(void) {
	UART2_GPIO_Init();
	
	// Enable USART2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; 
	
	// Select system clock for USART2 peripheral independent clock configuration register (01)
	RCC->CCIPR &= ~RCC_CCIPR_USART2SEL;
	RCC->CCIPR |= RCC_CCIPR_USART2SEL_0;

	// Select DMA 1 channel 7
	tx = DMA1_Channel7;

	DMA_Init_UARTx(DMA1_Channel7, USART2);
	tx->CMAR =(uint32_t)active;

	DMA1_CSELR->CSELR &= ~DMA_CSELR_C7S;
	DMA1_CSELR->CSELR |= 0X02000000;
	
	USART_Init(USART2);
	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(USART2_IRQn, 0);
}

// UART1 for bluetooth should use PB6 and PB7
// Note: please double check that we use GPIOB instead of GPIOA, simple bug that I missed
void UART1_GPIO_Init(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	
	// PB6 and PB7
	// Set mode to alternate function mode
	GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	GPIOB->MODER |= (GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);
	
	// Alternate function 7
	GPIOB->AFR[0] |= (GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6_3 | GPIO_AFRL_AFSEL7_3);
	
	// Set output speed as very high
	GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7);
	
	// Set output type to push pull
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);
	
	// Set PUPDR to pull up
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0);
}

void UART2_GPIO_Init(void) {
	// Enable Clock A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	// PA2 and PA3
	// Set mode to alternate function mode
	GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
	GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);
	
	// Alternate function 7 (0111)
	GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_3 | GPIO_AFRL_AFSEL3_3);
	
	// Set output speed as very high
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
	
	// Set output type to push pull
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
	
	// Set PUPDR to pull up
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPD2_0 | GPIO_PUPDR_PUPD3_0);
}

void USART_Init(USART_TypeDef* USARTx) {
	// Disable USART
	USARTx->CR1 &= ~USART_CR1_UE;
	
	// Set word length to 8 bits
	USARTx->CR1 &= ~USART_CR1_M;
	
	// Set oversampling mode to oversample by 16
	USARTx->CR1 &= ~USART_CR1_OVER8;
	
	// Set number of stop bits to 1
	USARTx->CR2 &= ~USART_CR2_STOP;
	
	// Set the Baud rate to 9600
	// fCLK is 80 MHz
	// (80*10^6)/(9600) = 8333.3
	USARTx->BRR = 8333;
	
	// Enable transmitter and receiver
	USARTx->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_TCIE | USART_CR1_RXNEIE);
	USARTx->CR3 |= USART_CR3_DMAT;
	
	// Enable USART
	USARTx->CR1 |= USART_CR1_UE;
}

/**
 * This function accepts a string that should be sent through UART
*/
void UART_print(char* data) {
	// Check DMA status. If DMA is ready, send data
	if (!(tx->CCR & DMA_CCR_EN)) {
		//Transfer char array to buffer
		sprintf(active, data);
		tx->CNDTR = (uint8_t)strlen(data);
		tx->CCR |= DMA_CCR_EN;
	} 
	// If DMA is not ready, put the data aside
	else {
		sprintf(pending, data);
		pending_size = (uint8_t)strlen(data);
	}
}

/**
 * This function should be invoked when a character is accepted through UART
*/
void transfer_data(char ch) {
	// Append character to input buffer
	inputs[input_size++] = ch;
	
	// If the character is end-of-line, invoke UART_onInput
	if (ch == '\n'){
		UART_onInput(inputs, input_size);
		input_size = 0;
	}
}

/**
 * This function should be invoked when DMA transaction is completed
*/
void on_complete_transfer(void) {
	tx->CCR &= ~DMA_CCR_EN;
	// Check for pending data
	if (pending_size > 0) {
		// Switch active/pending buffer
		uint8_t* temp = active;
		active = pending;
		pending = temp;
		
		// Send data
		tx->CMAR = (uint32_t)active;
		tx->CNDTR = pending_size;

		// Start data transfer
		tx->CCR |= DMA_CCR_EN;
		pending_size = 0;
	}
}

void USART1_IRQHandler(void){
	// Clear pending bit
	NVIC_ClearPendingIRQ(USART1_IRQn);
	
	// Character received
	if (USART1->ISR & USART_ISR_RXNE) {
		transfer_data((uint8_t)(USART1->RDR));
	} 
	// Sending data complete
	else if (USART1->ISR & USART_ISR_TC) {
		on_complete_transfer();
		
		// Clear transfer complete flag
		USART1->ICR |= USART_ICR_TCCF;
  }
}

void USART2_IRQHandler(void){
	// Clear pending bit
	NVIC_ClearPendingIRQ(USART2_IRQn);
	
	// Character received
	if (USART2->ISR & USART_ISR_RXNE) {
		transfer_data((uint8_t)(USART2->RDR));
	// Sending data complete
	} 
	else if (USART2->ISR & USART_ISR_TC) {
		on_complete_transfer();
		
		// Clear transfer complete flag
		USART2->ICR |= USART_ICR_TCCF;
  }
}