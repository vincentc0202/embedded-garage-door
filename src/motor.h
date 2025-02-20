#ifndef __STM32L476G_DISCOVERY_MOTOR_H
#define __STM32L476G_DISCOVERY_MOTOR_H

#include "stm32l476xx.h"

void Motor_Init(void);
void rotate(void);
void setDire(int8_t direction);

#endif /* __STM32L476G_DISCOVERY_MOTOR_H */
