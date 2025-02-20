/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */

#ifndef __STM32L476R_NUCLEO_UART_H
#define __STM32L476R_NUCLEO_UART_H

#include "stm32l476xx.h"

#define BufferSize 32
#define IO_SIZE 256

void UART1_Init(void);
void UART2_Init(void);
void UART1_GPIO_Init(void);
void UART2_GPIO_Init(void);

void USART_Init(USART_TypeDef* USARTx);

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);

void UART_print(char *data);
void UART_onInput(char* inputs, uint32_t size);

#endif
