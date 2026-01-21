/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "st7735.h"
#include "GFX_FUNCTIONS.h"
#include "fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIMER_COUNTER_FREQ_HZ 1000000UL // 1MHz

#define block_size 8
#define scr_w 160
#define scr_h 128

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
uint16_t readValueX;
uint16_t readValueY;

int a = 0;
int k = 0;

int Page = 1;
int Mode_Speed = 0;
int t_delay = 10000;

int x_head[1];
int y_head[1];

int x_body[320];
int y_body[320];

int x_black[1];
int y_black[1];

int x_target = 32;
int y_target = 32;


int i = 160;
int j = 128;

int count = 0;

int n = 0;

//int seed = 1;

int direction = 0;

char buffer[50];//String on LCD
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HomeMenu() {
	ST7735_FillScreen(ST7735_BLACK);


	fillRect(48, 16, 64, 16, ST7735_BLACK);
	ST7735_WriteString(53, 16, "SPEED", Font_11x18, ST7735_WHITE, ST7735_BLACK);


	drawRect(47,47,64,26,ST7735_WHITE);
	ST7735_WriteString(64, 56, "Slow", Font_7x10, ST7735_WHITE, ST7735_BLACK);


	drawRect(47,79,64,26,ST7735_WHITE);
	ST7735_WriteString(64, 88, "Fast", Font_7x10, ST7735_WHITE, ST7735_BLACK);

	ST7735_WriteString(25, 115, "Press B to Enter", Font_7x10, ST7735_WHITE, ST7735_BLACK);


}

void SlowSelect() {

	fillRect(47,47,64,26,ST7735_RED);
	ST7735_WriteString(64, 56, "Slow", Font_7x10, ST7735_YELLOW, ST7735_RED);

	fillRect(47,79,64,26,ST7735_BLACK);
	drawRect(47,79,64,26,ST7735_WHITE);
	ST7735_WriteString(64, 88, "Fast", Font_7x10, ST7735_WHITE, ST7735_BLACK);
}

void FastSelect() {

	fillRect(47,47,64,26,ST7735_BLACK);
	drawRect(47,47,64,26,ST7735_WHITE);
	ST7735_WriteString(64, 56, "Slow", Font_7x10, ST7735_WHITE, ST7735_BLACK);


	fillRect(47,79,64,26,ST7735_RED);
	ST7735_WriteString(64, 88, "Fast", Font_7x10, ST7735_YELLOW, ST7735_RED);
}

void SetBuzzerFrequency(uint32_t frequency_hz)
{
    if (frequency_hz == 0)
    {
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        return;
    }

    // Calculate ARR for the desired frequency
    // ARR = (Timer_Counter_Frequency / Desired_Frequency) - 1
    uint32_t arr_value = (TIMER_COUNTER_FREQ_HZ / frequency_hz) - 1;

    // Ensure ARR does not exceed maximum (16-bit timer)
    if (arr_value > 0xFFFF) // For a 16-bit timer (like TIM2)
    {
        arr_value = 0xFFFF; // Cap at max value if frequency is too low
    }
    if (arr_value < 1) // Ensure ARR is at least 1 to avoid division by zero or invalid state
    {
        arr_value = 1;
    }

    // Set the new ARR value
    __HAL_TIM_SET_AUTORELOAD(&htim1, arr_value);

    // Set the Pulse (Duty Cycle) - 50% for a simple tone
    // CCR = (ARR + 1) / 2
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (arr_value + 1) / 2); // Dùng TIM_CHANNEL_2

    // Start PWM on the selected channel
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); // Dùng TIM_CHANNEL_2
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_5) {
		if ((Page == 1)) {
			SlowSelect();
			Mode_Speed = 1;//slow
			t_delay = 1000;

		} else if ((Page == 2) && (k == 0)) {
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);//RED LED
			ST7735_WriteString(53, 32, "PAUSE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_WriteString(53, 60, "Press Y", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_WriteString(40, 75, "to Continue", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			k = 1;
		}
	} else if ((GPIO_Pin == GPIO_PIN_4)) {
		if(Page == 1) {
			FastSelect();
			Mode_Speed = 2;//fast
			t_delay = 200;

		} else if ((Page == 2) && (k == 1)) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);//YELLOW LED
			fillRect(32, 32, 96, 55, ST7735_BLACK);
			fillRect(x_head[0], y_head[0], block_size, block_size, ST7735_RED);
			fillRect(x_target, y_target, block_size, block_size, ST7735_BLUE);
			for (int n = 0; n < count; n++) {
				fillRect(x_body[n], y_body[n], block_size, block_size, ST7735_YELLOW);
			}
			k = 0;
		}

	} else if (GPIO_Pin == GPIO_PIN_3) {
		if((Page == 1) && (Mode_Speed != 0)) {
			Page = 2;
			ST7735_FillScreen(ST7735_BLACK);
			k = 0;

		} else {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);//GREEN LED
		}


	} else if (GPIO_Pin == GPIO_PIN_15) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);//BLUE LED

		    NVIC_SystemReset(); // Hàm reset hệ thống

	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start(&hadc1);
  HAL_ADC_Start(&hadc2);


  ST7735_Init();

  x_head[0] = 0;
  y_head[0] = 16;
  x_black [0] = scr_w - block_size;
  y_black [0] = 0;

  ST7735_FillScreen(ST7735_BLACK);


  HomeMenu();
  while (Page == 1) {
	  Page = Page;
  }

  while(Page == 2) {

  for (int n = 0; n < scr_w; n ++) {
	  ST7735_DrawPixel(n, 15, ST7735_WHITE);
  }


  sprintf(buffer, "Score %d", count);
  ST7735_WriteString(96, 0, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);
  ST7735_WriteString(8, 0, "X: Pause", Font_7x10, ST7735_WHITE, ST7735_BLACK);
//  x_target = 96;
//  y_target = 96;
  HAL_ADC_PollForConversion(&hadc1,1000);
  readValueY = HAL_ADC_GetValue(&hadc1);
  int seed = readValueY;

  srand(seed);

      int ran_x = rand() % i;
      int ran_y = rand() % j;


      while ((ran_x % block_size != 0) || (ran_y % block_size != 0) ||
    		  (ran_y <= 16) || ((ran_x == x_head[0]) && (ran_y == y_head[0])) ||
			  ((ran_x == x_target) && (ran_y == y_target))) {

	    	  seed++;
    	  	  ran_x = rand() % i;
	    	  ran_y = rand() % j;
      }
	  x_target = ran_x;
	  y_target = ran_y;


  fillRect(x_target, y_target, block_size, block_size, ST7735_BLUE);
  HAL_Delay(2000);
  fillRect(x_head[0], y_head[0], block_size, block_size, ST7735_RED);

  direction = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	/* begin*/


	 /* end*/

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


			  // Eat food

			  if((x_head[0] == x_target) && (y_head[0] == y_target)) {
		           SetBuzzerFrequency(3000);
		           HAL_Delay(100);
		           SetBuzzerFrequency(0);
				  count++;
				  for(int n = count-1; n >= 0; n--) {
					  x_body[n+1] = x_body[n];
					  y_body[n+1] = y_body[n];
				  }

				  x_body[0] = x_black[0];
				  y_body[0] = y_black[0];
				  fillRect(x_body[0], y_body[0], block_size, block_size, ST7735_YELLOW);

				  //

			      ran_x = rand() % i;
			      ran_y = rand() % j;

			      for (int n = 0; n < count; n++) {
				      while ((ran_x % block_size != 0) || (ran_y % block_size != 0) || (ran_y <= 16) ||
				    		  ((ran_x == x_head[0]) && (ran_y == y_head[0])) || ((ran_x == x_body[n]) && (ran_y == y_body[n])) ||
							  ((ran_x == x_target) && (ran_y == y_target))) {
					    	  seed++;
				    	  	  ran_x = rand() % i;
					    	  ran_y = rand() % j;
				      }
			      }

				  x_target = ran_x;
				  y_target = ran_y;
				  fillRect(x_target, y_target, block_size, block_size, ST7735_BLUE);
				  //

			    sprintf(buffer, "Score %d", count);
				ST7735_WriteString(96, 0, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);


			  }

			  //Move without body
			  if (count == 0) {
				  HAL_Delay(t_delay);
				  if (k == 0) {
					  x_black[0] = x_head[0];
					  y_black[0] = y_head[0];

					  fillRect(x_black[0], y_black[0], block_size, block_size, ST7735_BLACK);
				  }
			  }

			  //Move with body
			  if (count > 0) {
				  HAL_Delay(t_delay);
				  if (k == 0) {
					  x_black[0] = x_body[0];
					  y_black[0] = y_body[0];
					  fillRect(x_black[0], y_black[0], block_size, block_size, ST7735_BLACK);

					  for (int n = 0; n < (count-1); n++){
						  x_body[n] = x_body[n+1];
						  y_body[n] = y_body[n+1];
						  fillRect(x_body[n], y_body[n], block_size, block_size, ST7735_YELLOW);
					  }
					  x_body[count-1] = x_head[0];
					  y_body[count-1] = y_head[0];

					  fillRect(x_body[count -1], y_body[count -1], block_size, block_size, ST7735_YELLOW);
				  }
			  }

			  //Read Joystick's Value
			  if (k == 0) {
				    HAL_ADC_PollForConversion(&hadc1,1000);
				    readValueY = HAL_ADC_GetValue(&hadc1);
				    HAL_ADC_PollForConversion(&hadc2,1000);
				    readValueX = HAL_ADC_GetValue(&hadc2);

				    if ((readValueX > 2500)  && (direction != 4)) {//down
				    	direction = 2;
				    } else if ((readValueY < 1000) && (direction != 1)) {//left
				    	direction = 3;
				    } else if ((readValueX < 1000) && (direction != 2)) {//up
				    	direction = 4;
				    } else if ((readValueY > 2500) && (direction != 3)) {//right
				    	direction = 1;
				    }

			   // Direction Modify

				  if (direction == 1) {
					  if (x_head[0] == scr_w - block_size) {
						  x_head[0] = 0;
					  } else {
						  x_head[0] = x_head[0] + block_size;
					  }
					  fillRect(x_head[0], y_head[0], block_size, block_size, ST7735_RED);
				  } else if (direction == 2) {
					  if (y_head[0] == scr_h - block_size) {
						  y_head[0] = 16;
					  } else {
						  y_head[0] = y_head[0] + block_size;
					  }
					  fillRect(x_head[0], y_head[0], block_size, block_size, ST7735_RED);
				  } else if (direction == 3) {
					  if (x_head[0] == 0) {
						  x_head[0] = scr_w - block_size;
					  } else {
						  x_head[0] = x_head[0] - block_size;
					  }
					  fillRect(x_head[0], y_head[0], block_size, block_size, ST7735_RED);
				  } else if (direction ==  4) {
					  if (y_head[0] == 16) {
						  y_head[0] = scr_h - block_size;
					  } else {
						  y_head[0] = y_head[0] - block_size;
					  }
					  fillRect(x_head[0], y_head[0], block_size, block_size, ST7735_RED);
				  }

				  //Game Over

				  for (int n = 0; n < count; n++) {
					  if ((x_head[0] == x_body[n]) && (y_head[0] == y_body[n])) {
				           SetBuzzerFrequency(4000);
				           HAL_Delay(1000);
				           SetBuzzerFrequency(3500);
				           HAL_Delay(1000);
				           SetBuzzerFrequency(2500);
				           HAL_Delay(1000);
				           SetBuzzerFrequency(0);

				           //k=1
				           ST7735_FillScreen(ST7735_BLACK);
						  while(k == 0) {
							  HAL_Delay(1000);
						  	  ST7735_WriteString(56, 32, "GAME", Font_11x18, ST7735_RED, ST7735_BLACK);
						  	  ST7735_WriteString(56, 57, "OVER", Font_11x18, ST7735_RED, ST7735_BLACK);
						  	  HAL_Delay(1000);
						  	  ST7735_WriteString(56, 32, "GAME", Font_11x18, ST7735_YELLOW, ST7735_BLACK);
						  	  ST7735_WriteString(56, 57, "OVER", Font_11x18, ST7735_YELLOW, ST7735_BLACK);

								ST7735_WriteString(53, 92, "Press A", Font_7x10, ST7735_WHITE, ST7735_BLACK);
								ST7735_WriteString(35, 105, "to Play Again", Font_7x10, ST7735_WHITE, ST7735_BLACK);
								k = 2;
						  }

					  }
				  }
			  }





  }
}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 23;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB6 PB7 PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
