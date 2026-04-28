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
extern ADC_HandleTypeDef hadc1;
extern osMessageQId myQueue01Handle;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define TEMP_V25        0.76f  /* Napięcie przy 25°C w woltach */
#define TEMP_AVG_SLOPE  0.0025f /* Zmiana napięcia na stopień Celsjusza (2.5 mV/°C) */
#define ADC_MAX_VAL     4095.0f /* Dla 12-bitowego ADC */
#define VREF            3.3f    /* Napięcie referencyjne zasilania */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//-------------STRUKTURY-------------
typedef struct {
    float temperature;
    uint32_t timestamp; // Opcjonalnie: czas pomiaru
} TempData_t;

typedef struct {
    float resultValue;  // Wyliczone pole lub PI
    uint32_t iterations; // Liczba wykonanych iteracji
    char source[10];     // Nazwa źródła, np. "PI_GEN"
} CalcData_t;
typedef struct {
    uint32_t errorCode;
    char message[20];
} AlarmData_t;

//-------------UNIA STRUKTUR----------------
typedef enum {
    MSG_TYPE_TEMP,
    MSG_TYPE_CIRCLE,
    MSG_TYPE_EMERGENCY
} MsgType_t;

typedef struct {
    MsgType_t type;
    union {
        TempData_t temp;
        CalcData_t circle;
        AlarmData_t alarm;
    } data;
} CombinedMsg_t;

extern UART_HandleTypeDef huart1;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
int GetRandom(int min, int max){
    return (rand() % (max - min + 1)) + min;
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId ProducerTempHandle;
osThreadId ProducerCircleHandle;
osThreadId ProducerSignalHandle;
osThreadId ConsumerHandle;
osMessageQId tempQueueHandle;
osSemaphoreId mySemaphoreHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END FunctionPrototypes */

void StartProducerTemp(void const * argument);
void StartProducerCircle(void const * argument);
void StartProducerSignal(void const * argument);
void StartConsumer(void const * argument);

extern void MX_USB_HOST_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

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
__weak void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

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

  /* Create the semaphores(s) */
  /* definition and creation of mySemaphore */
  osSemaphoreDef(mySemaphore);
  mySemaphoreHandle = osSemaphoreCreate(osSemaphore(mySemaphore), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of tempQueue */
  osMessageQDef(tempQueue, 10, CombinedMsg_t);
  tempQueueHandle = osMessageCreate(osMessageQ(tempQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of ProducerTemp */
  osThreadDef(ProducerTemp, StartProducerTemp, osPriorityNormal, 0, 4096);
  ProducerTempHandle = osThreadCreate(osThread(ProducerTemp), NULL);

  /* definition and creation of ProducerCircle */
  osThreadDef(ProducerCircle, StartProducerCircle, osPriorityIdle, 0, 512);
  ProducerCircleHandle = osThreadCreate(osThread(ProducerCircle), NULL);

  /* definition and creation of ProducerSignal */
  osThreadDef(ProducerSignal, StartProducerSignal, osPriorityIdle, 0, 512);
  ProducerSignalHandle = osThreadCreate(osThread(ProducerSignal), NULL);

  /* definition and creation of Consumer */
  osThreadDef(Consumer, StartConsumer, osPriorityIdle, 0, 512);
  ConsumerHandle = osThreadCreate(osThread(Consumer), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartProducerTemp */
/**
  * @brief  Function implementing the ProducerTemp thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartProducerTemp */
void StartProducerTemp(void const * argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN StartProducerTemp */
  /* Inicjalizacja zmiennych do vTaskDelayUntil */
      TickType_t xLastWakeTime;
      const TickType_t xFrequency = 10; // 10 ticków (zazwyczaj 10 ms)

      /* Zmienne stanu i danych */
      float last_temperature = -273.15f; // Niewiarygodnie niska wartość początkowa
      float current_temperature = 0.0f;
      TempData_t tempDataToSend;

      /* Pobranie aktualnego czasu przed wejściem w pętlę */
      xLastWakeTime = xTaskGetTickCount();

      for(;;)
      {
          //Uruchomienie i odczyt ADC
          HAL_ADC_Start(&hadc1);
          if (HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK)
          {
              uint32_t adc_raw = HAL_ADC_GetValue(&hadc1);

              //Wyliczenie temperatury
              float voltage = ((float)adc_raw / ADC_MAX_VAL) * VREF;
              current_temperature = ((voltage - TEMP_V25) / TEMP_AVG_SLOPE) + 25.0f;

              // Sprawdzenie, czy temperatura się zmieniła (np. o więcej niż 0.2 stopnia)
              // Używamy fabs(), aby uniknąć fałszywych alarmów od szumu ADC
              if (fabs(current_temperature - last_temperature) >= 0.2f)
              {
                  // Aktualizacja ostatniej znanej temperatury
                  last_temperature = current_temperature;

                  //Uzupełnienie struktury
                  tempDataToSend.temperature = current_temperature;
                  tempDataToSend.timestamp = HAL_GetTick(); // Opcjonalnie

                  //Wysłanie do kolejki
                  CombinedMsg_t msg;
                  msg.type = MSG_TYPE_TEMP;
                  msg.data.temp.temperature = current_temperature;
                  msg.data.temp.timestamp = HAL_GetTick();

                  xQueueSend((QueueHandle_t)tempQueueHandle, &msg, 0);
              }
          }
          HAL_ADC_Stop(&hadc1); //Zatrzymanie ADC po odczycie

          //Usypianie zadania na dokładnie 10 ticków
          vTaskDelayUntil(&xLastWakeTime, xFrequency);
      }
  /* USER CODE END StartProducerTemp */
}

/* USER CODE BEGIN Header_StartProducerCircle */
/**
* @brief Function implementing the ProducerCircle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartProducerCircle */
void StartProducerCircle(void const * argument)
{
  /* USER CODE BEGIN StartProducerCircle */
	CalcData_t piData;

	    for(;;)
	    {
	        //Losowanie promienia r i ilości iteracji
	        int r = GetRandom(1, 100);
	        int iterations = GetRandom(1000, 10000);
	        int pointsInCircle = 0;

	        //Pętla Monte Carlo
	        for(int i = 0; i < iterations; i++)
	        {
	            // Losujemy współrzędne x i y w zakresie [-r, r]
	            int x = GetRandom(-r, r);
	            int y = GetRandom(-r, r);

	            // Sprawdzamy czy punkt (x,y) leży wewnątrz koła: x^2 + y^2 <= r^2
	            if ((x * x) + (y * y) <= (r * r))
	            {
	                pointsInCircle++;
	            }
	        }

	        // Wyliczenie Pi (lub pola powierzchni)
	        // Wzór: (Punkty_w_kole / Wszystkie_punkty) = (Pole_koła / Pole_kwadratu)
	        // Pole_kwadratu = (2r) * (2r) = 4r^2
	        float piEstimate = (4.0f * (float)pointsInCircle) / (float)iterations;
	        float circleArea = piEstimate * (float)r * (float)r;

	        //Uzupełnienie struktury
	        piData.resultValue = circleArea; // Wysyłamy pole powierzchni
	        piData.iterations = iterations;
	        strcpy(piData.source, "PI_CALC");

	        //Wysyłanie do kolejki
	        CombinedMsg_t msg;
	        msg.type = MSG_TYPE_CIRCLE;
	        msg.data.circle.resultValue = circleArea;
	        msg.data.circle.iterations = iterations;
	        strcpy(msg.data.circle.source, "PI_CALC");

	        xQueueSend((QueueHandle_t)tempQueueHandle, &msg, 0);

	        //Losowy czas uśpienia (0 - 5000 ms)
	        osDelay(GetRandom(0, 5000));
	    }
  /* USER CODE END StartProducerCircle */
}

/* USER CODE BEGIN Header_StartProducerSignal */
/**
* @brief Function implementing the ProducerSignal thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartProducerSignal */
void StartProducerSignal(void const * argument)
{
  /* USER CODE BEGIN StartProducerSignal */
	CombinedMsg_t alertMsg;

	  extern osMessageQId tempQueueHandle;
	  extern osSemaphoreId mySemaphoreHandle;

	  for(;;)
	  {
	    // Czekaj na semafor
	    if (osSemaphoreWait(mySemaphoreHandle, osWaitForever) == osOK)
	    {
	    //printf("tutaj jestem");
	      alertMsg.type = MSG_TYPE_EMERGENCY;
	      alertMsg.data.alarm.errorCode = 99;
	      strcpy(alertMsg.data.alarm.message, "PRZYCISK!");

	      //WYSYŁKA NA POCZĄTEK KOLEJKI
	      //tutaj if którys praawdza czy kolejka nie jest przepełniona przypadkiem dla imeout zero
	      if(xQueueSendToFront((QueueHandle_t)tempQueueHandle, &alertMsg, 0)== errQUEUE_FULL){
	    	  printf("BŁĄD PRZEPEŁNIENIA KOLEJKI \r\n");
	      }
	      osDelay(100);
	    }
	  }
  /* USER CODE END StartProducerSignal */
}

/* USER CODE BEGIN Header_StartConsumer */
/**
* @brief Function implementing the Consumer thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartConsumer */
void StartConsumer(void const * argument)
{
  /* USER CODE BEGIN StartConsumer */
	CombinedMsg_t received;

	  for(;;)
	  {
	    /* Czekaj na komunikat w kolejce nie dłużej niż 1 s (1000 ms) */
	    if (xQueueReceive((QueueHandle_t)tempQueueHandle, &received, pdMS_TO_TICKS(1000)) == pdPASS)
	    {
	    	//osDelay(5000); //symulacja bardzo dlugiego UART
	      /* Rozbieranie danych i bezpośrednie wysyłanie przez printf */
	      if (received.type == MSG_TYPE_TEMP)
	      {
	        printf("[EVENT] Sensor temperatury: %.2f C | Czas: %lu ms\r\n",
	               received.data.temp.temperature,
	               received.data.temp.timestamp);
	      }
	      else if (received.type == MSG_TYPE_CIRCLE)
	      {
	        printf("[EVENT] Monte Carlo (%s): Wynik = %.4f | Iteracje: %lu\r\n",
	               received.data.circle.source,
	               received.data.circle.resultValue,
	               received.data.circle.iterations);
	      }
	      else if (received.type == MSG_TYPE_EMERGENCY)
	      {
	    	//printf("CZAS DOSTARCZENIA ALARMU: %d\r\n", HAL_GetTick()); //DO SPRAWDZANIA CZASU DOSTANIA ALARMU
	        printf("\r\n!!!! [ALARM] %s (Kod: %lu) !!!!\r\n\r\n",
	               received.data.alarm.message,
	               received.data.alarm.errorCode);
	      }
	    }else
	      {
	      /* Wykona się, jeśli przez 1000ms nic nie wpadło do kolejki */
	      printf("INFO: Brak nowych danych w kolejce przez ostatnią sekundę...\r\n");
	      }

	  }
  /* USER CODE END StartConsumer */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  // Sprawdzamy czy przerwanie pochodzi z pinu 11 (PI11)
  if (GPIO_Pin == GPIO_PIN_11)
  {
    if (mySemaphoreHandle != NULL)
    {
      // Budzimy zadanie Emergency
    	//printf("CZAS ZGLOSZENIA: %d\r\n", HAL_GetTick()); //do sprawdzania czasu dsostania alarmu
    	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    	xSemaphoreGiveFromISR(mySemaphoreHandle, &xHigherPriorityTaskWoken);
    	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}
/* USER CODE END 4 */
/* USER CODE END Application */

