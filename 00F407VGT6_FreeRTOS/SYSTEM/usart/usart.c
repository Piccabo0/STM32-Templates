#include "sys.h"
#include "usart.h"

#if 1
#pragma import(__use_no_semihosting)
struct __FILE
{
	int handle;
};

FILE __stdout;

void _sys_exit(int x)
{
	(void)x;
}

int fputc(int ch, FILE *f)
{
	(void)f;
	while ((USART1->SR & 0x40U) == 0U)
	{
	}
	USART1->DR = (u8)ch;
	return ch;
}
#endif

#if EN_USART1_RX
u8 USART_RX_BUF[USART_REC_LEN];
u16 USART_RX_STA = 0;
#endif

void uart_init(u32 bound)
{
	GPIO_InitTypeDef gpioInit;
	USART_InitTypeDef usartInit;
	NVIC_InitTypeDef nvicInit;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	gpioInit.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	gpioInit.GPIO_Mode = GPIO_Mode_AF;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &gpioInit);

	usartInit.USART_BaudRate = bound;
	usartInit.USART_WordLength = USART_WordLength_8b;
	usartInit.USART_StopBits = USART_StopBits_1;
	usartInit.USART_Parity = USART_Parity_No;
	usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usartInit.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &usartInit);

#if EN_USART1_RX
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	nvicInit.NVIC_IRQChannel = USART1_IRQn;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 12;
	nvicInit.NVIC_IRQChannelSubPriority = 0;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInit);
#endif

	USART_Cmd(USART1, ENABLE);
}

#if EN_USART1_RX
void USART1_IRQHandler(void)
{
	static u8 rxState = 0;
	static u8 rxIndex = 0;
	u8 data;

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		data = (u8)USART_ReceiveData(USART1);

		if ((USART_RX_STA & 0x8000U) == 0U)
		{
			switch (rxState)
			{
				case 0:
					if (data == 0xFFU)
					{
						USART_RX_BUF[0] = data;
						rxState = 1;
						rxIndex = 1;
					}
					break;

				case 1:
					if (rxIndex < USART_REC_LEN)
					{
						USART_RX_BUF[rxIndex] = data;
						rxIndex++;

						if (data == 0xFEU)
						{
							USART_RX_STA = 0x8000U | rxIndex;
							rxState = 0;
							rxIndex = 0;
						}
					}
					else
					{
						rxState = 0;
						rxIndex = 0;
					}
					break;

				default:
					rxState = 0;
					rxIndex = 0;
					break;
			}
		}

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}
#endif
