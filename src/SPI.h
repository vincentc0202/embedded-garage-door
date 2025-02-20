#ifndef __STM32L476R_NUCLEO_SPI_H
#define __STM32L476R_NUCLEO_SPI_H

#include "stm32l476xx.h"

void SPI1_GPIO_Init(void);
void SPI1_Init(void);
uint16_t SPI_Transfer_Data(uint16_t write_data);

#endif 
