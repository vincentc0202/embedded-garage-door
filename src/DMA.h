/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Project
 */

#ifndef __STM32L476G_DISCOVERY_DMA_H
#define __STM32L476G_DISCOVERY_DMA_H

#include "stm32l476xx.h"

void DMA_Init_UARTx(DMA_Channel_TypeDef * tx, USART_TypeDef * uart);

void DMA1_Channel6_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);

#endif /* __STM32L476G_DISCOVERY_DMA_H */
