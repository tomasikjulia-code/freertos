/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint16_t DiodeDelay = 100;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
TaskHandle_t Handlery[5];
BaseType_t Returned[5];

/* USER CODE END Variables */
osThreadId BlinkingLed1Handle;
osThreadId BlinkingLed2Handle;
osThreadId BlinkingLed3Handle;
osThreadId BlinkingLed4Handle;
osThreadId TaskManagerHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Led1Start(void const * argument);
void Led2Start(void const * argument);
void Led3Start(void const * argument);
void Led4Start(void const * argument);
void TaskManagerStart(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
__weak void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of BlinkingLed1 */
//  osThreadDef(BlinkingLed1, Led1Start, osPriorityNormal, 0, 512);
//  BlinkingLed1Handle = osThreadCreate(osThread(BlinkingLed1), NULL);
//
//  /* definition and creation of BlinkingLed2 */
//  osThreadDef(BlinkingLed2, Led2Start, osPriorityIdle, 0, 512);
//  BlinkingLed2Handle = osThreadCreate(osThread(BlinkingLed2), NULL);
//
//  /* definition and creation of BlinkingLed3 */
//  osThreadDef(BlinkingLed3, Led3Start, osPriorityIdle, 0, 512);
//  BlinkingLed3Handle = osThreadCreate(osThread(BlinkingLed3), NULL);
//
//  /* definition and creation of BlinkingLed4 */
//  osThreadDef(BlinkingLed4, Led4Start, osPriorityIdle, 0, 512);
//  BlinkingLed4Handle = osThreadCreate(osThread(BlinkingLed4), NULL);
//
//  /* definition and creation of TaskManager */
//  osThreadDef(TaskManager, TaskManagerStart, osPriorityIdle, 0, 512);
//  TaskManagerHandle = osThreadCreate(osThread(TaskManager), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
Returned[0] = xTaskCreate((TaskFunction_t)Led1Start,
		"BlinkingLed1",
		configMINIMAL_STACK_SIZE,
		NULL,
		5,
		&Handlery[0]);
if(Returned[0] != pdPASS){
	vTaskDelete(Handlery[0]);
}


Returned[1] = xTaskCreate((TaskFunction_t)Led2Start,
		"BlinkingLed2",
		configMINIMAL_STACK_SIZE,
		NULL,
		5,
		&Handlery[1]);
if(Returned[1] != pdPASS){
	vTaskDelete(Handlery[1]);
}


Returned[2] = xTaskCreate((TaskFunction_t)Led3Start,
		"BlinkingLed3",
		configMINIMAL_STACK_SIZE,
		NULL,
		5,
		&Handlery[2]);
if(Returned[2] != pdPASS){
	vTaskDelete(Handlery[2]);
}

Returned[3] = xTaskCreate((TaskFunction_t)Led4Start,
		"BlinkingLed4",
		configMINIMAL_STACK_SIZE,
		NULL,
		5,
		&Handlery[3]);
if(Returned[3] != pdPASS){
	vTaskDelete(Handlery[3]);
}

Returned[4] = xTaskCreate((TaskFunction_t)TaskManagerStart,
		"TaskManager",
		configMINIMAL_STACK_SIZE,
		Handlery,
		5,
		&Handlery[4]);
if(Returned[4] != pdPASS){
	vTaskDelete(Handlery[4]);
}
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_Led1Start */
/**
  * @brief  Function implementing the BlinkingLed1 thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Led1Start */

void Led1Start(void const * argument)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);
    /* USER CODE BEGIN Led1Start */
    /* Infinite loop */
    for(;;)
    {
        printf("pierwsza dioda dziala\n");
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    /* USER CODE END Led1Start */
}
/* USER CODE BEGIN Header_Led2Start */
/**
* @brief Function implementing the BlinkingLed2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Led2Start */
void Led2Start(void const * argument)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(2000);
  /* USER CODE BEGIN Led2Start */
  /* Infinite loop */
  for(;;)
  {
	  printf("druga dioda dziala\n");
	  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
  /* USER CODE END Led2Start */
}

/* USER CODE BEGIN Header_Led3Start */
/**
* @brief Function implementing the BlinkingLed3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Led3Start */
void Led3Start(void const * argument)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(500);
  /* USER CODE BEGIN Led3Start */
  /* Infinite loop */
  for(;;)
  {
	  printf("trzecia dioda dziala\n");
	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
  /* USER CODE END Led3Start */
}

/* USER CODE BEGIN Header_Led4Start */
/**
* @brief Function implementing the BlinkingLed4 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Led4Start */
void Led4Start(void const * argument)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(3000);
  /* USER CODE BEGIN Led4Start */
  /* Infinite loop */
  for(;;)
  {
	  printf("czwarta dioda dziala\n");
	  HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
  /* USER CODE END Led4Start */
}

/* USER CODE BEGIN Header_TaskManagerStart */
/**
* @brief Function implementing the TaskManager thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TaskManagerStart */
void TaskManagerStart(void const * argument)
{
  /* USER CODE BEGIN TaskManagerStart */
	TaskHandle_t *p = argument;
	uint8_t LosowaPierwsza;
	uint16_t LosowaDruga;
	uint8_t TablicaStanow[4] = {0,0,0,0};
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 10);
	uint32_t seed  = HAL_ADC_GetValue(&hadc1);
	srand(seed); //ustawienie ziarna generatora liczb losowych
  /* Infinite loop */
  for(;;)
  {
	  LosowaPierwsza = GetRandom(0,3);
	  LosowaDruga = GetRandom(0,10000);

	  if(TablicaStanow[LosowaPierwsza]==0){
		  vTaskSuspend(p[LosowaPierwsza]);
		  TablicaStanow[LosowaPierwsza] = 1;
	  }else{
		  vTaskResume(p[LosowaPierwsza]);
		  TablicaStanow[LosowaPierwsza] = 0;
	  }

      vTaskDelay(pdMS_TO_TICKS(LosowaDruga));
  }
  /* USER CODE END TaskManagerStart */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
int GetRandom(int min, int max){
	return (rand() % (max - min + 1)) + min;
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == B1_Pin){
		if(DiodeDelay == 3000){
			DiodeDelay = 100;
		}else{
			DiodeDelay= DiodeDelay + 100;
		}
	}
}
/* USER CODE END Application */
