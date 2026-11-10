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
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "tim.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum {
	LED_STATE_OFF,
	LED_STATE_ON,
	LED_STATE_FADING
} led_state_t;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PI 3.14159265
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile led_state_t led_state ;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*-----------------------------------------------------------------*/

static int32_t ccr = 0;
static int32_t step = 5;

void update_fade(void)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim4);
    ccr += step;

    if (ccr >= (int32_t)arr) { ccr = arr; step = -step; }
    if (ccr <= 0)            { ccr = 0;   step = -step; }

    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, (uint32_t)ccr);
}

/*-----------------------------------------------------------------*/

static float phase = 0.0f;

void update_sine(void)
{
    const float f_sine      = 1.0f;   // Hz
    const float update_rate = 1000.0f; // Hz
    const float dphase = 2.0f * (float)PI * f_sine / update_rate;

    phase += dphase;
    if (phase >= 2.0f*(float)PI) phase -= 2.0f*(float)PI;

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim4);
    float s  = sinf(phase);                       // [-1,1]
    uint32_t ccr = (uint32_t)( arr * (0.5f + 0.5f*s) ); // [0,arr]
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, ccr);
}

/*-----------------------------------------------------------------*/

static int32_t step_fade = 12 ;
int32_t led_fade()
{
	int32_t ccr = __HAL_TIM_GET_COMPARE(&htim4,TIM_CHANNEL_2);
	ccr -= step_fade; if(ccr < 0 ) ccr = 0;
	__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,(uint32_t)ccr);
	return ccr ;
}


void led_update()
{
	static led_state_t last_state = LED_STATE_OFF;
	switch (led_state) {

		case LED_STATE_ON:
			if(last_state != LED_STATE_ON)
			{
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,TIM4->ARR);
			}
			last_state = LED_STATE_ON;
			break;

		case LED_STATE_FADING:
			last_state = LED_STATE_FADING;
		    if (led_fade() == 0)
		    {
		        led_state = LED_STATE_OFF;
		    }
			break;

		case LED_STATE_OFF:
			if(last_state != LED_STATE_OFF)
			{
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,0);
			}
			last_state = LED_STATE_OFF;
			break;
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
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_TIM4_Init();
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_time =	0;

  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
    uint32_t now =	HAL_GetTick();

    if (now-last_time >= 1) {
		//update_fade();
		//update_sine();
		led_update();
		last_time = now;
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static uint32_t last_tick = 0 ;
static GPIO_PinState last_btn = 0 ;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == B1_Pin)
	{
		//debounce
		GPIO_PinState btn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
		if ( btn != last_btn ) //edge
		{
			last_btn = btn;
			uint32_t now  = HAL_GetTick();
			if ( now - last_tick > 10 )  // delay
			{
				last_tick = now;

				if (btn == GPIO_PIN_SET ) // rising edge
				{
					led_state = LED_STATE_ON;
				}
				else // falling edge
				{
					led_state = LED_STATE_FADING;
				}

			}
		}
	}
}


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
