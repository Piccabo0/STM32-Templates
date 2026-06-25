#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "MyTask.h"
#include "led.h"
#include "usart.h"

#define LED_TASK_STACK_SIZE    128
#define SERIAL_TASK_STACK_SIZE 160

#define LED_TASK_PRIORITY      1
#define SERIAL_TASK_PRIORITY   2

#define LED_CMD_HEADER         0xFFU
#define LED_CMD_DEVICE         0x01U
#define LED_CMD_OFF            0x00U
#define LED_CMD_ON             0x01U
#define LED_CMD_BLINK          0x02U
#define LED_CMD_TAIL           0xFEU
#define LED_CMD_LENGTH         4U

typedef enum
{
	LED_MODE_OFF = 0,
	LED_MODE_ON,
	LED_MODE_BLINK,
} LedMode_t;

static TaskHandle_t ledTaskHandle;
static TaskHandle_t serialTaskHandle;
static SemaphoreHandle_t serialTxMutex;
static volatile LedMode_t ledMode = LED_MODE_OFF;

static void Serial_SendByte(uint8_t data)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{
	}
	USART_SendData(USART1, data);
}

static void Serial_EchoFrame(uint8_t *frame, uint16_t length)
{
	uint16_t i;

	if (serialTxMutex != NULL)
	{
		xSemaphoreTake(serialTxMutex, portMAX_DELAY);
	}

	for (i = 0; i < length; i++)
	{
		Serial_SendByte(frame[i]);
	}

	if (serialTxMutex != NULL)
	{
		xSemaphoreGive(serialTxMutex);
	}
}

static void LED_ApplyCommand(uint8_t *frame, uint16_t length)
{
	if (length != LED_CMD_LENGTH)
	{
		return;
	}

	if ((frame[0] != LED_CMD_HEADER) ||
	    (frame[1] != LED_CMD_DEVICE) ||
	    (frame[3] != LED_CMD_TAIL))
	{
		return;
	}

	switch (frame[2])
	{
		case LED_CMD_OFF:
			ledMode = LED_MODE_OFF;
			LED1_Off();
			break;

		case LED_CMD_ON:
			ledMode = LED_MODE_ON;
			LED1_On();
			break;

		case LED_CMD_BLINK:
			ledMode = LED_MODE_BLINK;
			break;

		default:
			break;
	}
}

static void LedTask(void *argument)
{
	(void)argument;

	while (1)
	{
		switch (ledMode)
		{
			case LED_MODE_ON:
				LED1_On();
				vTaskDelay(pdMS_TO_TICKS(50));
				break;

			case LED_MODE_BLINK:
				LED1_Toggle();
				vTaskDelay(pdMS_TO_TICKS(500));
				break;

			case LED_MODE_OFF:
			default:
				LED1_Off();
				vTaskDelay(pdMS_TO_TICKS(50));
				break;
		}
	}
}

static void SerialTask(void *argument)
{
	uint16_t length;
	uint16_t i;
	uint8_t frame[USART_REC_LEN];

	(void)argument;

	while (1)
	{
		if ((USART_RX_STA & 0x8000U) != 0U)
		{
			taskENTER_CRITICAL();
			length = USART_RX_STA & 0x3FFFU;
			if (length > USART_REC_LEN)
			{
				length = USART_REC_LEN;
			}
			for (i = 0; i < length; i++)
			{
				frame[i] = USART_RX_BUF[i];
			}
			USART_RX_STA = 0;
			taskEXIT_CRITICAL();

			Serial_EchoFrame(frame, length);
			LED_ApplyCommand(frame, length);
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void MyTask(void)
{
	LED_Init();
	uart_init(9600);

	serialTxMutex = xSemaphoreCreateMutex();
	configASSERT(serialTxMutex != NULL);

	configASSERT(xTaskCreate(LedTask,
	                         "LED",
	                         LED_TASK_STACK_SIZE,
	                         NULL,
	                         LED_TASK_PRIORITY,
	                         &ledTaskHandle) == pdPASS);

	configASSERT(xTaskCreate(SerialTask,
	                         "Serial",
	                         SERIAL_TASK_STACK_SIZE,
	                         NULL,
	                         SERIAL_TASK_PRIORITY,
	                         &serialTaskHandle) == pdPASS);

	vTaskStartScheduler();
}
