/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "string.h"

/*---------------------------------------------*/
osThreadId LedOnTaskHandle;
osThreadId LedOffTaskHandle;
osThreadId LedBlinkTaskHandle;
osMessageQId MyQueueHandle;

char rxMessage[6];

/* Private function prototypes -----------------------------------------------*/
void ledOnTask_init(void const *argument);
void ledOffTask_init(void const *argument);
void ledBlinkTask_init(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{	
	/*Create simple Queue to pass single 16bit integer*/
	osMessageQDef(myQueue, 1, uint16_t);
	MyQueueHandle = osMessageCreate(osMessageQ(myQueue), NULL);

	/* Create the thread(s) */
	/* definition and creation of LedOnTask */
	osThreadDef(LedOnTask, ledOnTask_init, osPriorityAboveNormal, 0, 128);
	LedOnTaskHandle = osThreadCreate(osThread(LedOnTask), NULL);

	/* definition and creation of LedOffTask */
	osThreadDef(LedOffTask, ledOffTask_init, osPriorityHigh, 0, 128);
	LedOffTaskHandle = osThreadCreate(osThread(LedOffTask), NULL);

	/* definition and creation of LedBlinkTask */
	osThreadDef(LedBlinkTask, ledBlinkTask_init, osPriorityNormal, 0, 128);
	LedBlinkTaskHandle = osThreadCreate(osThread(LedBlinkTask), NULL);

	/*configure interrupt to recieve six bytes via uart*/
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&rxMessage, 6);

	/*Suspend all task before the start of the kernel*/
	osThreadSuspend(LedOnTaskHandle);
	osThreadSuspend(LedOffTaskHandle);
	osThreadSuspend(LedBlinkTaskHandle);
}


/* USER CODE BEGIN Header_ledOnTask_init */
/**
 * @brief  Function implementing the LedOnTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_ledOnTask_init */
void ledOnTask_init(void const *argument)
{	
	for (;;) {	
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		osDelay(1000);
	}
}


/**
 * @brief Function implementing the LedOffTask thread.
 * @param argument: Not used
 * @retval None
 */
void ledOffTask_init(void const *argument)
{	
	for (;;) {	
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		osDelay(1000);
	}
}


/**
 * @brief Function implementing the LedBlinkTask thread.
 * @param argument: Not used
 * @retval None
 */
void ledBlinkTask_init(void const *argument)
{	
	osEvent myEvent;
	volatile uint16_t duration;

	for (;;) {
		for (volatile uint8_t i = 0; i < 1; i++) {	
			myEvent = osMessageGet(MyQueueHandle,10);
			duration = myEvent.value.v;
		}
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		osDelay(duration);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		osDelay(duration);
	}
}


/**
 * @brief Uart recieve call back function
 * @param argument: Not used
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{	
	volatile uint16_t duration;
	volatile static uint8_t runningTaskId = 0;

	/*configure interrupt to recieve six bytes via uart*/
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&rxMessage, 6);

	/*suspend all 3 task and setup interrupt for receiving another byte through byte*/
	switch (runningTaskId) {
	case 1:
		if (osThreadSuspend(LedOnTaskHandle) != osOK) {
			HAL_UART_Transmit(&huart1, (uint8_t *)"ERROR\n", 6, 100);
		}
		break;
	case 2:
		osThreadSuspend(LedOffTaskHandle);
		break;
	case 3:
		osThreadSuspend(LedBlinkTaskHandle);
		break;
	}

	/*run 1 task and suspend other 2 according to the received data*/
	if (strncmp(rxMessage, "110000", 6) == 0) {
		osThreadResume(LedOnTaskHandle);
		runningTaskId = 1;
		
	} else if (strncmp(rxMessage, "000000", 6) == 0) {
		osThreadResume(LedOffTaskHandle);
		runningTaskId = 2;
		
	} else if (strncmp(rxMessage, "22", 2) == 0) {		
		/*convert blink duration from string to int*/
		duration  = (rxMessage[2] - 48) * 1000;
		duration += (rxMessage[3] - 48) * 100;
		duration += (rxMessage[4] - 48) * 10;
		duration += (rxMessage[5] - 48) * 1;
		/*pass the duration value to LedBlinkTask through message queue*/
		osMessagePut(MyQueueHandle, duration, osWaitForever);
		osThreadResume(LedBlinkTaskHandle);
		runningTaskId = 3;
	}
}
