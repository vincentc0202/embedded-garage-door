#include "SPI.h"
#include "SysTimer.h"

void SPI1_GPIO_Init(void) {
	// Enable the GPIO Clock
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);
	
	// Set PA4, PB3, PB4, and PB5 to Alternative Functions (10)
	GPIOA->MODER &= ~GPIO_MODER_MODE4;
	GPIOA->MODER |= GPIO_MODER_MODE4_1;
	GPIOB->MODER &= ~(GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5);
	GPIOB->MODER |= (GPIO_MODER_MODE3_1 | GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1);
	
	// Configure their AFR to SPI1
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL4;
	GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL4_2 | GPIO_AFRL_AFSEL4_0);
	
	// Set PB3, PB4, and PB5 to AF5
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3 | GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5);
	GPIOB->AFR[0] |= (GPIO_AFRL_AFSEL3_2 | GPIO_AFRL_AFSEL3_0);
	GPIOB->AFR[0] |= (GPIO_AFRL_AFSEL4_2 | GPIO_AFRL_AFSEL4_0);
	GPIOB->AFR[0] |= (GPIO_AFRL_AFSEL5_2 | GPIO_AFRL_AFSEL5_0);
	
	// Set GPIO Pins to:
	
	// Very high output speed (11)
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED4;
	GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED3 | GPIO_OSPEEDR_OSPEED4 | GPIO_OSPEEDR_OSPEED5);
	
	// Output type push-pull (0)
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT4;
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT3 | GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5);
	
	// No pull-up/down (00)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5);
}


void SPI1_Init(void) {
    // Enable SPI1 clock and reset SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;         // Enable SPI1 clock in APB2 peripheral clock register
    RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;      // Reset SPI1
    RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;     // Clear the reset for SPI1

    // Disable SPI before configuring it
    SPI1->CR1 &= ~SPI_CR1_SPE;

    // Configure for Full Duplex Communication (0 for undirectional mode)
    SPI1->CR1 &= ~SPI_CR1_BIDIMODE;            

    // Configure for 2-line Unidirectional Data Mode (0)
    SPI1->CR1 &= ~SPI_CR1_RXONLY;            

    // Disable Output in Bidirectional Mode 
    SPI1->CR1 &= ~SPI_CR1_BIDIOE;             

    // Set Frame Format: MSB First, 16-bit data format, Motorola mode
    SPI1->CR1 &= ~SPI_CR1_LSBFIRST;             // Set MSB first (LSBFIRST = 0)
    SPI1->CR2 |= SPI_CR2_DS; 
    SPI1->CR2 &= ~SPI_CR2_FRF;

		// Configure Clock 	TODO: check why it's set to 1
    SPI1->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA); 

    // Set Baud Rate Prescaler to 16 						// f_PRESC = f_SPICLK / (2^(1 + BR))
    SPI1->CR1 |= SPI_CR1_BR; 										// SPI clock = APB2 clock / 2^(1 + BR) 
		SPI1->CR1 &= ~SPI_CR1_BR_2;									// BR = 011

    // Disable Hardware CRC Calculation
    SPI1->CR1 &= ~SPI_CR1_CRCEN;                // Set CRCEN to 0 to disable CRC

    // Set as Master
    SPI1->CR1 |= SPI_CR1_MSTR;                  // Set MSTR to 1 for master configuration

    // Disable Software SSM
    SPI1->CR1 &= ~SPI_CR1_SSM;                 

    // Enable NSS Pulse Management
    SPI1->CR2 |= SPI_CR2_NSSP;                  

		// Enable Output for SPIO_GPIO_Init()
		SPI1->CR2 |= SPI_CR2_SSOE;  

    // Set FIFO Reception Threshold to 1/2
    //SPI1->CR2 |= SPI_CR2_FRXTH;	// this was masked instead of reset? possible bug
		SPI1->CR2 &= ~SPI_CR2_FRXTH;   		
		
		// Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE; 
}

uint16_t SPI_Transfer_Data(uint16_t write_data){
	// Wait for TXE (Transmit buffer empty)
	while ((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);
	
	// Write data
	SPI1->DR = write_data;
	
	// Wait for not busy
	while ((SPI1->SR & SPI_SR_BSY) == SPI_SR_BSY);
	
	// Wait for RXNE (Receive buffer not empty)
	while ((SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE);
	
	// Read data
	return SPI1->DR;
}