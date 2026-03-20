/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include<stdio.h>
#include<string.h>

/*======================= MACROS =======================*/
#define EMERGENCY_LED_PIN  GPIO_PIN_14
#define TASK1_LED_PIN      GPIO_PIN_10
#define TASK2_LED_PIN      GPIO_PIN_11
#define TASK3_LED_PIN      GPIO_PIN_12

#define	LED_PORT GPIOG

#define EMERGENCY_TASK_BIT	0x01
#define TASK1_BIT 			0x02
#define TASK2_BIT 			0x04
#define TASK3_BIT 			0x08

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
UART_HandleTypeDef huart1;

//osThreadId defaultTaskHandle;
//osTimerId myTimer01Handle;
//osMutexId myMutex01Handle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
//void StartDefaultTask(void const * argument);
//void Callback01(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*================ TASK PROTOTYPES ==================*/
void Task1(void *arg);
void Task2(void *arg);
void Task3(void *arg);
void Emergency_Task(void *arg);
void Control_Task(void *arg);
void UART_Task(void *arg);
void Timer_Task1(TimerHandle_t xTimer); //To print health for every 15sec
void Timer_Task2(TimerHandle_t xTimer); //To overcome bouncing effect

/* =============== PRINTF ============================*/
//To use printf()
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart1, (uint8_t*) ptr, (uint16_t) len, HAL_MAX_DELAY);
	return len;
}

/*==================== HANDLES =======================*/
SemaphoreHandle_t LED_Mutex;
SemaphoreHandle_t UART_Mutex;

TimerHandle_t TimerHandle1, TimerHandle2;
EventGroupHandle_t EventGroupHandle;
TaskHandle_t Emer_Handle, Task1_Handle, Task2_Handle, Task3_Handle,
		Control_Handle, UART_Handle;

uint8_t rx_char;
uint8_t status[50];
uint8_t i = 0;
/*============== COMMON LED TOGGLE FUNCTION ===========*/
void Toggle_LED(GPIO_TypeDef *port, uint16_t pin) {
	xSemaphoreTake(LED_Mutex, portMAX_DELAY);
	HAL_GPIO_TogglePin(port, pin);
	xSemaphoreGive(LED_Mutex);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

	/*================ MUTEX CREATION =====================*/
	LED_Mutex = xSemaphoreCreateMutex(); //Mutex for Common Toggle LED function
	UART_Mutex = xSemaphoreCreateMutex(); //Mutex for UART

	/*=============== SOFTWARE TIMER CREATION =============*/
	TimerHandle1 = xTimerCreate("Timerpc", pdMS_TO_TICKS(15000), pdTRUE, 0,
			Timer_Task1);
	TimerHandle2 = xTimerCreate("Timer2_pc", pdMS_TO_TICKS(50), pdFALSE, 0,
			Timer_Task2);

	/*================= TASK CREATIONS ====================*/
	xTaskCreate(Emergency_Task, "Eme_task_pc", 128, NULL, 2, &Emer_Handle);
	xTaskCreate(Task1, "Task1_pc", 128, NULL, 1, &Task1_Handle);
	xTaskCreate(Task2, "Task2_pc", 128, NULL, 1, &Task2_Handle);
	xTaskCreate(Task3, "Task3_pc", 128, NULL, 1, &Task3_Handle);
	xTaskCreate(Control_Task, "Control_pc", 128, NULL, 2, &Control_Handle);
	xTaskCreate(UART_Task, "UART_pc", 128, NULL, 2, &UART_Handle);

	/*================EVENT GROUP CREATION============================*/
	EventGroupHandle = xEventGroupCreate();

	//Suspending all the tasks initially
	vTaskSuspend(Emer_Handle);
	vTaskSuspend(Task1_Handle);
	vTaskSuspend(Task2_Handle);
	vTaskSuspend(Task3_Handle);

	//Start the software timer
	xTimerStart(TimerHandle1, 0);

	HAL_UART_Receive_IT(&huart1, &rx_char, 1);

	//Start the scheduler
	vTaskStartScheduler();

	/* USER CODE END 2 */

	/* Create the mutex(es) */
	/* definition and creation of myMutex01 */
//  osMutexDef(myMutex01);
//  myMutex01Handle = osMutexCreate(osMutex(myMutex01));
	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* Create the timer(s) */
	/* definition and creation of myTimer01 */
//  osTimerDef(myTimer01, Callback01);
//  myTimer01Handle = osTimerCreate(osTimer(myTimer01), osTimerPeriodic, NULL);
	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
//  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
//  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);
	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	//osKernelStart();
	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG,
	GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : PA0 PA1 PA2 PA3 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PG10 PG11 PG12 PG13
	 PG14 */
	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
			| GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
uint8_t bit;
//CONTROLLER TASK
void Control_Task(void *arg) {
	for (;;) {
		bit = xEventGroupWaitBits(EventGroupHandle, 0x01 | 0x02 | 0x04 | 0x08,
		pdTRUE, pdFALSE, portMAX_DELAY); // read the bit value and clear
		switch (bit) {
		case 0x01:	//If Emergency bit is set resume it and suspend remaining.
			vTaskResume(Emer_Handle);
			vTaskSuspend(Task1_Handle);
			vTaskSuspend(Task2_Handle);
			vTaskSuspend(Task3_Handle);
			break;

		case 0x02:		//If Task1 bit is set resume it and suspend remaining.
			vTaskResume(Task1_Handle);
			vTaskSuspend(Emer_Handle);
			vTaskSuspend(Task2_Handle);
			vTaskSuspend(Task3_Handle);
			break;

		case 0x04:		//If Task2 bit is set resume it and suspend remaining.
			vTaskResume(Task2_Handle);
			vTaskSuspend(Emer_Handle);
			vTaskSuspend(Task1_Handle);
			vTaskSuspend(Task3_Handle);
			break;

		case 0x08:		//If task3 bit is set resume it and suspend remaining.
			vTaskResume(Task3_Handle);
			vTaskSuspend(Emer_Handle);
			vTaskSuspend(Task2_Handle);
			vTaskSuspend(Task1_Handle);
			break;

		}
	}
}

//call back function when interrupt is given
uint32_t count;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTimerStartFromISR(TimerHandle2, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		status[i] = rx_char;
		i++;

		HAL_UART_Receive_IT(&huart1, &rx_char, 1);  // Restart UART interrupt

		if ((i == 7) || (i == 14)) {
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			vTaskNotifyGiveFromISR(UART_Handle, &xHigherPriorityTaskWoken); // Notify UART task that new data arrived
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

//Emergency task
void Emergency_Task(void *arg) {
	for (;;) {
		Toggle_LED(LED_PORT, EMERGENCY_LED_PIN);
		vTaskDelay(pdMS_TO_TICKS(250));
	}
}

//Task1 - Toggle LED for 500ms
void Task1(void *arg) {
	for (;;) {
		Toggle_LED(LED_PORT, TASK1_LED_PIN);
		vTaskDelay(pdMS_TO_TICKS(500));

	}
}
//Task2 - Toggle LED for 100ms
void Task2(void *arg) {
	for (;;) {
		Toggle_LED(LED_PORT, TASK2_LED_PIN);
		vTaskDelay(pdMS_TO_TICKS(1000));

	}
}
//Task3 - Toggle LED for 150ms
void Task3(void *arg) {
	for (;;) {
		Toggle_LED(LED_PORT, TASK3_LED_PIN);
		vTaskDelay(pdMS_TO_TICKS(1500));

	}
}

//Timer task to print the number of event changes for every 15 sec
void Timer_Task1(TimerHandle_t xTimer) {
	i = 0;
	//print how many times event changed
	xSemaphoreTake(UART_Mutex, portMAX_DELAY);
	printf("\r\nNo of times Events Changed: %d", count);
	xSemaphoreGive(UART_Mutex);
}

void Timer_Task2(TimerHandle_t xTimer) {
	if ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) == 1) {
		xEventGroupSetBits(EventGroupHandle, EMERGENCY_TASK_BIT);
		count++;
	} else if ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)) == 1) {
		xEventGroupSetBits(EventGroupHandle, TASK1_BIT);
		count++;
	} else if ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2)) == 1) {
		xEventGroupSetBits(EventGroupHandle, TASK2_BIT);
		count++;
	} else if ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3)) == 1) {
		xEventGroupSetBits(EventGroupHandle, TASK3_BIT);
		count++;
	}
}

uint8_t health[500]; //health buffer

//UART task
void UART_Task(void *arg) {
	for (;;) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if ((!strncmp((char*) status, "STATUS?", 7))) {
			vTaskList(health); //Lists all the tasks
			xSemaphoreTake(UART_Mutex, portMAX_DELAY);
			printf("\r\n%s", health);
			xSemaphoreGive(UART_Mutex);
		}
		//user changing to emergency mode
		else if (!(strncmp((char*) status, "SET_MODE=EMERG", 14))) {
			xEventGroupSetBits(EventGroupHandle, EMERGENCY_TASK_BIT);
			count++;
		}
		//user changing to task1 mode
		else if (!(strncmp((char*) status, "SET_MODE=TASK1", 14))) {
			xEventGroupSetBits(EventGroupHandle, TASK1_BIT);
			count++;
		}
		//user changing to task2 mode
		else if (!(strncmp((char*) status, "SET_MODE=TASK2", 14))) {
			xEventGroupSetBits(EventGroupHandle, TASK2_BIT);
			count++;

		}
		//user changing to task3 mode
		else if (!(strncmp((char*) status, "SET_MODE=TASK3", 14))) {
			xEventGroupSetBits(EventGroupHandle, TASK3_BIT);
			count++;
		}
	}
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
//void StartDefaultTask(void const * argument)
//{
/* USER CODE BEGIN 5 */
/* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
/* USER CODE END 5 */
//}
/* Callback01 function */
//void Callback01(void const * argument)
//{
/* USER CODE BEGIN Callback01 */
/* USER CODE END Callback01 */
//}
/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM1) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
