#ifndef __STM32L476R_NUCLEO_I2C_H
#define __STM32L476R_NUCLEO_I2C_H

#include <stdint.h>
#include <stddef.h>
#include "stm32l476xx.h"

#define READ_FROM_SLAVE 1
#define WRITE_TO_SLAVE  0

int8_t I2C_Start(I2C_TypeDef * I2Cx, uint32_t DevAddress, uint8_t Size, uint8_t Direction);
void I2C_Stop(I2C_TypeDef * I2Cx);
void I2C_WaitLineIdle(I2C_TypeDef * I2Cx);
int8_t I2C_SendData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size);
int8_t I2C_ReceiveData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size);
void I2C_GPIO_Init(void);
void I2C_Initialization(void);
void CODEC_Initialization(void);

#endif
