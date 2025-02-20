/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */
 
#include "DMA.h"
#include "SysTimer.h"

void DMA_Init_UARTx(DMA_Channel_TypeDef * tx, USART_TypeDef * uart) {
	// Enable the clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	
	// Wait 20us for DMA to finish setting up
	delay(20);
	
	// Disable the channel
	tx->CCR &= ~DMA_CCR_EN;
	
	// Disable memory-to-memory mode (want peripheral to memory)
	tx->CCR &= ~DMA_CCR_MEM2MEM;
	
	// Set channel priority to high (10)
	tx->CCR &= ~DMA_CCR_PL;
	tx->CCR |= DMA_CCR_PL_1;
	
	// Set peripheral size to 8-bit (00) --> 8 bits because we are sending 'chars'
	tx->CCR &= ~DMA_CCR_PSIZE;
	
	// Set memory size to 8-bit (00)
	tx->CCR &= ~DMA_CCR_MSIZE;
	
	// Disable peripheral increment mode
	tx->CCR &= ~DMA_CCR_PINC;
	
	// Enable memory increment mode
	tx->CCR |= DMA_CCR_MINC;
	
	// Disable circular mode
	tx->CCR &= ~DMA_CCR_CIRC;
	
	// Set data transfer direction to Memory-to-Peripheral (read FROM memory --> enabled)
	tx->CCR |= DMA_CCR_DIR;		
	
	// Set the data destination to the data register of the UART
	tx->CPAR = (uint32_t) &(uart->TDR);
	
	// Disable half transfer interrupt
	tx->CCR &= ~DMA_CCR_HTIE;
	
	// Disable transfer error interrupt
	tx->CCR &= ~DMA_CCR_TEIE;
	
	// Enable transfer complete interrupt
	tx->CCR |= DMA_CCR_TCIE;
}
