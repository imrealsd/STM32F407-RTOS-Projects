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

char rxMessege[100];
char singleByte;

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
	/* Create the thread(s) */
	/* definition and creation of LedOnTask */
	osThreadDef(LedOnTask, ledOnTask_init, osPriorityNormal, 0, 128);
	LedOnTaskHandle = osThreadCreate(osThread(LedOnTask), NULL);

	/* definition and creation of LedOffTask */
	osThreadDef(LedOffTask, ledOffTask_init, osPriorityNormal, 0, 128);
	LedOffTaskHandle = osThreadCreate(osThread(LedOffTask), NULL);

	/* definition and creation of LedBlinkTask */
	osThreadDef(LedBlinkTask, ledBlinkTask_init, osPriorityNormal, 0, 128);
	LedBlinkTaskHandle = osThreadCreate(osThread(LedBlinkTask), NULL);

	/*configure interrupt to recieve single bytes via uart*/
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&singleByte, 1);


	/*Suspend all task*/
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
	/* USER CODE BEGIN ledOnTask_init */
	/* Infinite loop */
	for (;;)
	{
		osDelay(1);
	}
	/* USER CODE END ledOnTask_init */
}


/**
 * @brief Function implementing the LedOffTask thread.
 * @param argument: Not used
 * @retval None
 */
void ledOffTask_init(void const *argument)
{
	/* USER CODE BEGIN ledOffTask_init */
	/* Infinite loop */
	for (;;)
	{
		osDelay(1);
	}
	/* USER CODE END ledOffTask_init */
}

/**
 * @brief Function implementing the LedBlinkTask thread.
 * @param argument: Not used
 * @retval None
 */
void ledBlinkTask_init(void const *argument)
{
	/* USER CODE BEGIN ledBlinkTask_init */
	/* Infinite loop */
	for (;;)
	{
		osDelay(1);
	}
	/* USER CODE END ledBlinkTask_init */
}


/**
 * @brief Uart recieve call back function
 * @param argument: Not used
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	volatile static uint8_t index = 0;
	rxMessege[index] = singleByte; 
	index++;

	if (strncmp(rxMessege, "LED_ON", 6) == 0) {

	} else if (strncmp(rxMessege, "LED_OFF", 6) == 0) {

	} else if (strncmp(rxMessege, "LED_BLINK", 6) == 0) {

	}
}
