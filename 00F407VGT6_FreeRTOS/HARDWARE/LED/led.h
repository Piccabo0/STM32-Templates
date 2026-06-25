#ifndef __LED_H
#define __LED_H

#include "sys.h"

#define LED_ON_LEVEL   0U
#define LED_OFF_LEVEL  1U

#define LED1_PIN       GPIO_Pin_15
#define LED1_GPIO      GPIOA
#define LED1_CLK       RCC_AHB1Periph_GPIOA
#define LED1           PAout(15)

void LED_Init(void);
void LED1_On(void);
void LED1_Off(void);
void LED1_Toggle(void);

#endif
