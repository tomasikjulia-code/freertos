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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//definicje semaforow

osSemaphoreId dataSemHandle;
osSemaphoreDef(DATA_SEM);

//definicja mutexow
osMutexId dataMutexHandle;
osMutexDef(dataMutex);

// FUNKCJE DO OTWIERANIA I ZAMYKANIA MUTEXOW:
// osMutexWait(dataMutexHandle, osWaitForever);
// osMutexRelease(dataMutexHandle);

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */




/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
typedef struct {
	uint32_t MeasurementTime;
	uint16_t SamplingTime;
	float GyroMeasurement[3];
	float AccelMeasurement[3];
	uint32_t StartCountingTime;
	float MeanGyroMeasurement[3];
	float PowerGyro;
	float MeanAccelMeasurement[3];
	float PowerAccel;
	uint32_t StopCountingTime;
	uint8_t stage; // Flaga synchronizacji miedzy taskami
}SensorData_t;

SensorData_t g_data;

/* USER CODE END Variables */
osThreadId SensorTaskHandle;
osThreadId SensorCountingHandle;
osThreadId SensorMonitorHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartSensorTask(void const * argument);
void StartSensorCounting(void const * argument);
void StartSensorMonitor(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 2 */

//funkcja zwracająca liczby od -1 do 1 w rozkładzie normalnym (podzielona przez 1000 daje szum gaussowski)
double randn() {
    double u = ((double)rand() / (RAND_MAX)) * 2.0 - 1.0;
    double v = ((double)rand() / (RAND_MAX)) * 2.0 - 1.0;
    double r = u * u + v * v;
    if (r == 0 || r > 1) return randn();
    return (u * sqrt(-2.0 * log(r) / r))/1000.0f; // Zwraca rozkład normalny (śr=0, odchyl=1) podzielony przez 1000 zeby by
}

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
	dataSemHandle = osSemaphoreCreate(osSemaphore(DATA_SEM), 1);

	//inicjalizacja mutexow
	dataMutexHandle = xSemaphoreCreateMutex();

	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET); // Poprawiono z LED2 na LED3
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
  /* definition and creation of SensorTask */
  // Wyrownano priorytety, aby uniknac glodzenia taskow
  osThreadDef(SensorTask, StartSensorTask, osPriorityHigh, 0, 4*512);
  SensorTaskHandle = osThreadCreate(osThread(SensorTask), NULL);

  /* definition and creation of SensorCounting */
  osThreadDef(SensorCounting, StartSensorCounting, osPriorityNormal, 0, 4*512);
  SensorCountingHandle = osThreadCreate(osThread(SensorCounting), NULL);

  /* definition and creation of SensorMonitor */
  osThreadDef(SensorMonitor, StartSensorMonitor, osPriorityIdle, 0, 4*512);
  SensorMonitorHandle = osThreadCreate(osThread(SensorMonitor), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartSensorTask */
/**
  * @brief  Function implementing the SensorTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void const * argument)
{
  /* USER CODE BEGIN StartSensorTask */
	g_data.MeasurementTime = osKernelSysTick();
	float bias_gyro = 0.05f;
	float bias_accel = 0.04f;
	float g_const = 9.81f;
	uint8_t iterator = 0;

	// Zmienne lokalne do printowania
	float loc_Gyro[3] = {0};
	float loc_Accel[3] = {0};
	uint32_t loc_MeasTime = 0;
	uint16_t loc_SampTime = 0;
	uint8_t print_flag = 0;

  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

	  xSemaphoreTake(dataMutexHandle, portMAX_DELAY);
	  //osSemaphoreWait(dataSemHandle, osWaitForever);

	  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

      uint32_t current_tick = osKernelSysTick();

      g_data.SamplingTime = current_tick - g_data.MeasurementTime;
      g_data.MeasurementTime = current_tick;

      //pomiary żyroskopu
      g_data.GyroMeasurement[0] = 0.0f + randn()+bias_gyro;
      g_data.GyroMeasurement[1] = 0.0f + randn()+bias_gyro;
      g_data.GyroMeasurement[2] = 0.0f + randn()+bias_gyro;

      //pomiary akcelerometru
      g_data.AccelMeasurement[0] = 0.0f + randn()+bias_accel;
      g_data.AccelMeasurement[1] = 0.0f + randn()+bias_accel;
      g_data.AccelMeasurement[2] = g_const + randn()+bias_accel;

      //g_data.stage = 1; // Task A wykonal pomiar
      iterator = iterator + 1;

      if(iterator >= 100){
          iterator = 0;
          print_flag = 1;

          // Kopiowanie do zmiennych lokalnych wewnatrz muteksa
          loc_Gyro[0] = g_data.GyroMeasurement[0];
          loc_Gyro[1] = g_data.GyroMeasurement[1];
          loc_Gyro[2] = g_data.GyroMeasurement[2];
          loc_Accel[0] = g_data.AccelMeasurement[0];
          loc_Accel[1] = g_data.AccelMeasurement[1];
          loc_Accel[2] = g_data.AccelMeasurement[2];
          loc_MeasTime = g_data.MeasurementTime;
          loc_SampTime = g_data.SamplingTime;
      }

      xSemaphoreGive(dataMutexHandle);
      //osSemaphoreRelease(dataSemHandle);

      // Printowanie poza sekcja krytyczna
      if(print_flag) {
          print_flag = 0;
          printf("\n Pomiary Zyroskopu: ");
          printf("GyroX: %.4f | GyroY: %.4f | GyroZ: %.4f ", loc_Gyro[0], loc_Gyro[1], loc_Gyro[2]);

          printf("pomiary Akcelerometru: ");
          printf("AccelX: %.4f | AccelY: %.4f | AccelZ: %.4f ", loc_Accel[0], loc_Accel[1], loc_Accel[2]);

          printf("Czas wykonania pomiaru: %ld ", loc_MeasTime);
          printf("Czas pomiedzy pomiarami: %d \r\n", loc_SampTime);
      }

      vTaskDelay(10);
  }
  /* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartSensorCounting */
/**
* @brief Function implementing the SensorCounting thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorCounting */
void StartSensorCounting(void const * argument)
{
  /* USER CODE BEGIN StartSensorCounting */
	uint8_t iterator = 0;

	// Inicjalizacja wartosci poczatkowych (mozesz to zrobic przed wejsciem w petle)
	xSemaphoreTake(dataMutexHandle, portMAX_DELAY);
	//osSemaphoreWait(dataSemHandle, osWaitForever);

	g_data.MeanAccelMeasurement[0] = g_data.AccelMeasurement[0];
	g_data.MeanAccelMeasurement[1] = g_data.AccelMeasurement[1];
	g_data.MeanAccelMeasurement[2] = g_data.AccelMeasurement[2];
	g_data.MeanGyroMeasurement[0] = g_data.GyroMeasurement[0];
	g_data.MeanGyroMeasurement[1] = g_data.GyroMeasurement[1];
	g_data.MeanGyroMeasurement[2] = g_data.GyroMeasurement[2];

	xSemaphoreGive(dataMutexHandle);
	//osSemaphoreRelease(dataSemHandle);

	// Zmienne lokalne do printowania
	float loc_Gyro[3], loc_Accel[3], loc_MeanAccel[3], loc_MeanGyro[3];
	float loc_PowerAccel, loc_PowerGyro;
	uint32_t loc_MeasTime, loc_StartCountingTime, loc_StopCountingTime;
	uint16_t loc_SampTime;
	uint8_t print_flag = 0;

  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

	  xSemaphoreTake(dataMutexHandle, portMAX_DELAY);
	  //osSemaphoreWait(dataSemHandle, osWaitForever);

	  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

      // Sprawdzamy, czy Task A przygotowal nowe dane
      //if(g_data.stage == 1) {
          g_data.StartCountingTime = g_data.MeasurementTime;

          g_data.MeanAccelMeasurement[0] = 0.97 * g_data.MeanAccelMeasurement[0] + (1-0.97) * g_data.AccelMeasurement[0];
          g_data.MeanAccelMeasurement[1] = 0.97 * g_data.MeanAccelMeasurement[1] + (1-0.97) * g_data.AccelMeasurement[1];
          g_data.MeanAccelMeasurement[2] = 0.97 * g_data.MeanAccelMeasurement[2] + (1-0.97) * g_data.AccelMeasurement[2];
          g_data.MeanGyroMeasurement[0] = 0.97 * g_data.MeanGyroMeasurement[0] + (1-0.97) * g_data.GyroMeasurement[0];
          g_data.MeanGyroMeasurement[1] = 0.97 * g_data.MeanGyroMeasurement[1] + (1-0.97) * g_data.GyroMeasurement[1];
          g_data.MeanGyroMeasurement[2] = 0.97 * g_data.MeanGyroMeasurement[2] + (1-0.97) * g_data.GyroMeasurement[2];

          g_data.PowerAccel = (powf(g_data.MeanAccelMeasurement[0],2) + powf(g_data.MeanAccelMeasurement[1],2) + powf(g_data.MeanAccelMeasurement[2],2));
          g_data.PowerGyro = (powf(g_data.MeanGyroMeasurement[0],2) + powf(g_data.MeanGyroMeasurement[1],2) + powf(g_data.MeanGyroMeasurement[2],2));

          g_data.StopCountingTime = osKernelSysTick();
          g_data.stage = 2; // Task B zakonczyl prace, gotowe dla Task C

          iterator = iterator + 1;

          if(iterator >= 100){
              iterator = 0;
              print_flag = 1;

              // Kopiowanie do zmiennych lokalnych
              for(int i=0; i<3; i++) {
                  loc_Gyro[i] = g_data.GyroMeasurement[i];
                  loc_Accel[i] = g_data.AccelMeasurement[i];
                  loc_MeanAccel[i] = g_data.MeanAccelMeasurement[i];
                  loc_MeanGyro[i] = g_data.MeanGyroMeasurement[i];
              }
              loc_PowerAccel = g_data.PowerAccel;
              loc_PowerGyro = g_data.PowerGyro;
              loc_MeasTime = g_data.MeasurementTime;
              loc_SampTime = g_data.SamplingTime;
              loc_StartCountingTime = g_data.StartCountingTime;
              loc_StopCountingTime = g_data.StopCountingTime;
          }
      //}

	  xSemaphoreGive(dataMutexHandle);
      //osSemaphoreRelease(dataSemHandle);

	  // Printowanie poza sekcja krytyczna
	  if(print_flag) {
	      print_flag = 0;
          printf("\n Pomiary Zyroskopu: ");
          printf("GyroX: %.4f | GyroY: %.4f | GyroZ: %.4f ", loc_Gyro[0], loc_Gyro[1], loc_Gyro[2]);

          printf("pomiary Akcelerometru: ");
          printf("AccelX: %.4f | AccelY: %.4f | AccelZ: %.4f ", loc_Accel[0], loc_Accel[1], loc_Accel[2]);

          printf("srednia kroczaca dla Akcelerometru: ");
          printf("AccelMeanX: %.4f | AccelMeanY: %.4f | AccelMeanZ: %.4f ", loc_MeanAccel[0], loc_MeanAccel[1], loc_MeanAccel[2]);

          printf("srednia kroczaca dla zyroskopu: ");
          printf("GyroMeanX: %.4f | GyroMeanY: %.4f | GyroMeanZ: %.4f ", loc_MeanGyro[0], loc_MeanGyro[1], loc_MeanGyro[2]);

          printf("Moc Akcelerometru: %f ", loc_PowerAccel);
          printf("Moc zyroskopu: %f ", loc_PowerGyro);
          printf("Czas wykonania pomiaru: %ld ", loc_MeasTime);
          printf("Czas pomiedzy pomiarami: %d ", loc_SampTime);
          printf("Poczatek obliczen: %ld ", loc_StartCountingTime);
          printf("Koniec obliczen: %ld \r\n", loc_StopCountingTime);
	  }

      osDelay(10);
  }
  /* USER CODE END StartSensorCounting */
}

/* USER CODE BEGIN Header_StartSensorMonitor */
/**
* @brief Function implementing the SensorMonitor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorMonitor */
void StartSensorMonitor(void const * argument)
{
  /* USER CODE BEGIN StartSensorMonitor */
	float unfilteredAccelPower = 0;
	float unfilteredGyroPower = 0;

	// Flagi bledow do printowania
	uint8_t err_accel = 0, err_gyro = 0, err_sync = 0, err_samp = 0;

  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);

	  xSemaphoreTake(dataMutexHandle, portMAX_DELAY);
	  //osSemaphoreWait(dataSemHandle, osWaitForever);

	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);

	  // Wykonujemy sprawdzenie tylko gdy Task B zakonczyl swoja prace
	 //if(g_data.stage == 2) {
          unfilteredAccelPower = (powf(g_data.AccelMeasurement[0],2) + powf(g_data.AccelMeasurement[1],2) + powf(g_data.AccelMeasurement[2],2));
          unfilteredGyroPower = (powf(g_data.GyroMeasurement[0],2) + powf(g_data.GyroMeasurement[1],2) + powf(g_data.GyroMeasurement[2],2));

          float x = unfilteredAccelPower - g_data.PowerAccel;
          float y = unfilteredGyroPower - g_data.PowerGyro;

          if(fabs(x) > 0.3f){
              err_accel = 1;
          }
          if(fabs(y) > 0.3f){
              err_gyro = 1;
          }
          if(g_data.MeasurementTime != g_data.StartCountingTime){
              err_sync = 1;
          }
          if( 9 > g_data.SamplingTime || g_data.SamplingTime > 11){
              err_samp = 1;
          }

          g_data.stage = 0; // Czyscimy wage po udanym monitoringu
	 //}

	  xSemaphoreGive(dataMutexHandle);
	  //osSemaphoreRelease(dataSemHandle);

	  // Printowanie bledow poza sekcja krytyczna
	  if(err_accel) {
	      printf("BLAD: blad w wartosci mocy akcelerometru!!!!!\r\n");
	      err_accel = 0;
	  }
	  if(err_gyro) {
	      printf("BLAD: blad w wartosci mocy zyroskopu!!!!!\r\n");
	      err_gyro = 0;
	  }
	  if(err_sync) {
	      printf("BLAD: task B nie dziala na tej samej probce co task A!!!!!! \r\n");
	      err_sync = 0;
	  }
	  if(err_samp) {
	      printf("BLAD: Czas probkowania sie nie zgadza!!!!! \r\n");
	      err_samp = 0;
	  }

      osDelay(71);
  }
  /* USER CODE END StartSensorMonitor */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
