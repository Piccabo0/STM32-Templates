#include "led.h"

static void LED_GPIO_Init(GPIO_TypeDef *gpio, uint32_t pin)
{
	GPIO_InitTypeDef gpioInit;

	gpioInit.GPIO_Pin = pin;
	gpioInit.GPIO_Mode = GPIO_Mode_OUT;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInit.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(gpio, &gpioInit);
}

void LED_Init(void)
{
	RCC_AHB1PeriphClockCmd(LED1_CLK, ENABLE);

	LED_GPIO_Init(LED1_GPIO, LED1_PIN);

	LED1_Off();
}

void LED1_On(void)
{
	GPIO_ResetBits(LED1_GPIO, LED1_PIN);
}

void LED1_Off(void)
{
	GPIO_SetBits(LED1_GPIO, LED1_PIN);
}

void LED1_Toggle(void)
{
	LED1 = !LED1;
}
