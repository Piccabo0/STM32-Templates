#include "FreeRTOS.h"
#include "task.h"

#include "MyTask.h"

#define LED_TASK_STACK_SIZE    128
#define SERIAL_TASK_STACK_SIZE 128
#define KEY_TASK_STACK_SIZE    128

#define LED_TASK_PRIORITY      1
#define SERIAL_TASK_PRIORITY   2
#define KEY_TASK_PRIORITY      2

#define SERIAL_FRAME_MAX_LEN   64

typedef enum
{
	LED_MODE_OFF = 0,
	LED_MODE_ON,
} LedMode_t;

static TaskHandle_t ledTaskHandle;
static TaskHandle_t serialTaskHandle;
static TaskHandle_t keyTaskHandle;

static volatile LedMode_t ledMode = LED_MODE_OFF;
static volatile uint8_t serialRxFrame[SERIAL_FRAME_MAX_LEN];
static volatile uint8_t serialRxLength = 0;
static volatile uint8_t serialRxFlag = 0;

static void LED_Init(void)
{
	GPIO_InitTypeDef gpioInit;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = GPIO_Pin_8;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInit);
}

static void LED_On(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

static void LED_Off(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

static void Key_Init(void)
{
	GPIO_InitTypeDef gpioInit;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	gpioInit.GPIO_Mode = GPIO_Mode_IPD;
	gpioInit.GPIO_Pin = GPIO_Pin_0;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInit);
}

static void Serial_Init(uint32_t baudrate)
{
	GPIO_InitTypeDef gpioInit;
	USART_InitTypeDef usartInit;
	NVIC_InitTypeDef nvicInit;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

	gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInit.GPIO_Pin = GPIO_Pin_9;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInit);

	gpioInit.GPIO_Mode = GPIO_Mode_IPU;
	gpioInit.GPIO_Pin = GPIO_Pin_10;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInit);

	usartInit.USART_BaudRate = baudrate;
	usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usartInit.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	usartInit.USART_Parity = USART_Parity_No;
	usartInit.USART_StopBits = USART_StopBits_1;
	usartInit.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &usartInit);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	nvicInit.NVIC_IRQChannel = USART1_IRQn;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 12;
	nvicInit.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvicInit);

	USART_Cmd(USART1, ENABLE);
}

static void Serial_SendByte(uint8_t data)
{
	USART_SendData(USART1, data);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{
	}
}

static void Serial_SendArray(uint8_t *array, uint8_t length)
{
	uint8_t i;

	for (i = 0; i < length; i++)
	{
		Serial_SendByte(array[i]);
	}
}

static void LedTask(void *argument)
{
	(void)argument;

	LED_Off();

	while (1)
	{
		switch (ledMode)
		{
			case LED_MODE_ON:
				LED_On();
				vTaskDelay(pdMS_TO_TICKS(100));
				break;

			case LED_MODE_OFF:
			default:
				LED_Off();
				vTaskDelay(pdMS_TO_TICKS(100));
				break;
		}
	}
}

static void SerialTask(void *argument)
{
	uint8_t frame[SERIAL_FRAME_MAX_LEN];
	uint8_t length;
	uint8_t i;

	(void)argument;

	while (1)
	{
		if (serialRxFlag != 0)
		{
			taskENTER_CRITICAL();
			length = serialRxLength;
			for (i = 0; i < length; i++)
			{
				frame[i] = serialRxFrame[i];
			}
			serialRxFlag = 0;
			taskEXIT_CRITICAL();

			if (length > 0)
			{
				Serial_SendArray(frame, length);
			}
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

static void KeyTask(void *argument)
{
	uint8_t keyPressed = 0;

	(void)argument;

	while (1)
	{
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == SET)
		{
			if (keyPressed == 0)
			{
				keyPressed = 1;
				ledMode = (ledMode == LED_MODE_OFF) ? LED_MODE_ON : LED_MODE_OFF;
			}
		}
		else
		{
			keyPressed = 0;
		}

		vTaskDelay(pdMS_TO_TICKS(20));
	}
}

void MyTask(void)
{
	LED_Init();
	Key_Init();
	Serial_Init(9600);

	xTaskCreate(LedTask,
	            "LED",
	            LED_TASK_STACK_SIZE,
	            NULL,
	            LED_TASK_PRIORITY,
	            &ledTaskHandle);

	xTaskCreate(SerialTask,
	            "Serial",
	            SERIAL_TASK_STACK_SIZE,
	            NULL,
	            SERIAL_TASK_PRIORITY,
	            &serialTaskHandle);

	xTaskCreate(KeyTask,
	            "KEY",
	            KEY_TASK_STACK_SIZE,
	            NULL,
	            KEY_TASK_PRIORITY,
	            &keyTaskHandle);

	vTaskStartScheduler();
}

void USART1_IRQHandler(void)
{
	static uint8_t rxState = 0;
	static uint8_t rxIndex = 0;

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		uint8_t rxData = (uint8_t)USART_ReceiveData(USART1);

		switch (rxState)
		{
			case 0:
				if (rxData == 0xFF)
				{
					serialRxFrame[0] = rxData;
					rxState = 1;
					rxIndex = 1;
				}
				break;

			case 1:
				if (rxIndex < SERIAL_FRAME_MAX_LEN)
				{
					serialRxFrame[rxIndex] = rxData;
					rxIndex++;

					if (rxData == 0xFE)
					{
						serialRxLength = rxIndex;
						serialRxFlag = 1;
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

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}
