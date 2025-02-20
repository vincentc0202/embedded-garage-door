#ifndef __STM32L476R_NUCLEO_ACC_H
#define __STM32L476R_NUCLEO_ACC_H

#include "stm32l476xx.h"

void accWrite(uint8_t addr, uint8_t val);

uint8_t accRead(uint8_t addr);

void initAcc(void);

void readValues(double* x, double* y, double* z);


#endif 
