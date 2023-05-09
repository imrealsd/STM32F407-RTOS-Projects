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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId LiveTaskHandle;
osThreadId GeneeratorTaskHandle;
osThreadId DisplayTaskHandle;
osMessageQId myQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void liveTask_init(void const * argument);
void generatorTask_init(void const * argument);
void displayTask_init(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
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
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of myQueue */
  osMessageQDef(myQueue, 5, uint8_t);
  myQueueHandle = osMessageCreate(osMessageQ(myQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of LiveTask */
  osThreadDef(LiveTask, liveTask_init, osPriorityBelowNormal, 0, 128);
  LiveTaskHandle = osThreadCreate(osThread(LiveTask), NULL);

  /* definition and creation of GeneeratorTask */
  osThreadDef(GeneeratorTask, generatorTask_init, osPriorityNormal, 0, 128);
  GeneeratorTaskHandle = osThreadCreate(osThread(GeneeratorTask), NULL);

  /* definition and creation of DisplayTask */
  osThreadDef(DisplayTask, displayTask_init, osPriorityIdle, 0, 128);
  DisplayTaskHandle = osThreadCreate(osThread(DisplayTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_liveTask_init */
/**
  * @brief  Function implementing the LiveTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_liveTask_init */
void liveTask_init(void const * argument)
{
  /* USER CODE BEGIN liveTask_init */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END liveTask_init */
}

/* USER CODE BEGIN Header_generatorTask_init */
/**
* @brief Function implementing the GeneeratorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_generatorTask_init */
void generatorTask_init(void const * argument)
{
  /* USER CODE BEGIN generatorTask_init */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END generatorTask_init */
}

/* USER CODE BEGIN Header_displayTask_init */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_displayTask_init */
void displayTask_init(void const * argument)
{
  /* USER CODE BEGIN displayTask_init */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END displayTask_init */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
