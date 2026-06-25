#include "delay.h"

static uint32_t cycles_per_us = 0;

void delay_init(u8 sysclk_mhz)
{
	(void)sysclk_mhz;

	cycles_per_us = SystemCoreClock / 1000000U;
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(u32 nus)
{
	uint32_t start;
	uint32_t ticks;

	if (cycles_per_us == 0U)
	{
		delay_init(0);
	}

	start = DWT->CYCCNT;
	ticks = nus * cycles_per_us;
	while ((uint32_t)(DWT->CYCCNT - start) < ticks)
	{
	}
}

void delay_ms(u16 nms)
{
	while (nms != 0U)
	{
		delay_us(1000U);
		nms--;
	}
}
