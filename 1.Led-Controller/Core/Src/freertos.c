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

#define USER_TASK_SUSPEND 1234
#define USER_TASK_RUN     4321

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
	osThreadDef(LedOnTask, ledOnTask_init, osPriorityNormal, 0, 128);
	LedOnTaskHandle = osThreadCreate(osThread(LedOnTask), NULL);

	/* definition and creation of LedOffTask */
	osThreadDef(LedOffTask, ledOffTask_init, osPriorityNormal, 0, 128);
	LedOffTaskHandle = osThreadCreate(osThread(LedOffTask), NULL);

	/**
	 * definition and creation of LedBlinkTask
	 * blink task has highest priority because at the end of ISR 
	 * blink task will suspend first (if running) then led on/off task will run
	 * otherwise led/on off task will run first then blink task will change led state.
	 */
	osThreadDef(LedBlinkTask, ledBlinkTask_init , osPriorityAboveNormal, 0, 128);
	LedBlinkTaskHandle = osThreadCreate(osThread(LedBlinkTask), NULL);

	/*configure interrupt to recieve six bytes via uart*/
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&rxMessage, 6);

	/*suspend all 3 task before the start of the scheduler*/
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
	/*Get the signal from ISR change the led state and suspend itself*/
	for (;;) {	
		/*wait for the task run signal*/
		osSignalWait(USER_TASK_RUN, osWaitForever);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		osThreadSuspend(NULL);
	}
}


/**
 * @brief Function implementing the LedOffTask thread.
 * @param argument: Not used
 * @retval None
 */
void ledOffTask_init(void const *argument)
{	
	/*Get the signal from ISR change the led state and suspend itself*/
	for (;;) {	
		/*wait for the task run signal*/
		osSignalWait(USER_TASK_RUN, osWaitForever);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		osThreadSuspend(NULL);
	}
}


/**
 * @brief Function implementing the LedBlinkTask thread.
 * @param argument: Not used
 * @retval None
 */
void ledBlinkTask_init(void const *argument)
{	
	osEvent myMessegeEvent;
	osEvent mySignalEvent;
	volatile uint16_t duration;

	/*Get the signal from ISR change the led state and run until suspend signal is recieved from ISR*/
	for (;;) {
		/*check for the messege to get blink duration value*/
		myMessegeEvent = osMessageGet(MyQueueHandle,10);
		if (myMessegeEvent.status == osEventMessage) {
			duration = myMessegeEvent.value.v;
		}
		
		/*blink led*/
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_Delay(duration);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_Delay(duration);

		/*check for the suspend signal to suspend itself*/
		mySignalEvent = osSignalWait(USER_TASK_SUSPEND, 10);
		if (mySignalEvent.status == osEventSignal) {
			osThreadSuspend(NULL);
		}
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

	/**
	 * If Blink task running & led on/off request is recieved form bluetooth/uart:
	 * send signal to ruuning task (ask it to suspend itself)
	 * cannot suspend task from ISR , "API's ending with 'FromISR' can only be used inside ISR"
	 * osThreadSuspend() calls xTaskSuspend() native API
	 * osThreadResume() calls xTaskResumeFromISR() native API
	 */
	if (strncmp(rxMessage, "110000", 6) == 0) {
		osThreadResume(LedOnTaskHandle);
		osSignalSet(LedOnTaskHandle, USER_TASK_RUN);

		if (runningTaskId == 3) {
			osSignalSet(LedBlinkTaskHandle, USER_TASK_SUSPEND);
		}
		runningTaskId = 1;
		
	} else if (strncmp(rxMessage, "000000", 6) == 0) {
		osThreadResume(LedOffTaskHandle);
		osSignalSet(LedOffTaskHandle, USER_TASK_RUN);

		if (runningTaskId == 3) {
			osSignalSet(LedBlinkTaskHandle, USER_TASK_SUSPEND);
		}
		runningTaskId = 2;
	
	/**
	 * If Blink task is running & Blink request is received through bluetooh/uart
	 * just send the updated "duration" to blink task , no need to suspend/resume
	 */
	} else if (strncmp(rxMessage, "22", 2) == 0) {		
		/*convert blink duration from string to int*/
		duration  = (rxMessage[2] - 48) * 1000;
		duration += (rxMessage[3] - 48) * 100;
		duration += (rxMessage[4] - 48) * 10;
		duration += (rxMessage[5] - 48) * 1;
		/*pass the duration value to LedBlinkTask through message queue*/
		osMessagePut(MyQueueHandle, duration, osWaitForever);

		/*Resume blink task only when it was suspended previously to run other task*/
		if (runningTaskId != 3) {
			osThreadResume(LedBlinkTaskHandle);
			runningTaskId = 3;
		}
	}
}
