/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_lorawan.h"
#include "LoRaMac.h"
#include "lora_app.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "flash_if.h"
#include "LmHandler.h"
#include "lora_at.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void payload_max_check(int8_t *datarate, LoRaMacRegion_t *region, uint32_t *payload_length_max);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  Store_region_default(LORAMAC_REGION_EU868);

  /* Initialize all configured peripherals */
  MX_LoRaWAN_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  payload_max_check(&datarate, &region, &payload_length_max);

  while (1)
  {
    /* USER CODE END WHILE */
    MX_LoRaWAN_Process();

    /* USER CODE BEGIN 3 */
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief payload_length_max setting
  * @retval None
  */
void payload_max_check(int8_t *datarate, LoRaMacRegion_t *region, uint32_t *payload_length_max)
{
	uint8_t buff[KEY_PLUS_REGION_DIM];

	FLASH_Read((void*)buff,(const void*)ADDR_FLASH_PAGE_63, KEY_PLUS_REGION_DIM);
	if (buff[49] != 0xFF)
	{
		*region = buff[49];
	}
	else
	{
		*region = buff[48];
	}

	if (LmHandlerGetTxDatarate(datarate) != LORAMAC_HANDLER_SUCCESS)
	{
		return;
	}
	switch(*datarate)
	{
	   case 0: //DR_0
		   switch(*region)
		   {
			   case LORAMAC_REGION_EU868:
				   *payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   *payload_length_max = 11;
				   break;
			   case LORAMAC_REGION_AU915:
				   *payload_length_max = 51;
				   break;
			   default:
				   *payload_length_max = 11;
				   break;
		   }
		   break;
	   case 1://DR_1
		   switch(*region)
		   {
			   case LORAMAC_REGION_EU868:
				   *payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   *payload_length_max = 53;
				   break;
			   case LORAMAC_REGION_AU915:
				   *payload_length_max = 51;
			   default:
				   *payload_length_max = 11;
				   break;
		   }
		   break;
	   case 2://DR_2
		   switch(*region)
		   {
			   case LORAMAC_REGION_EU868:
				   *payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   *payload_length_max = 125;
				   break;
			   case LORAMAC_REGION_AU915:
				   *payload_length_max = 51;
				   break;
			   default:
				   *payload_length_max = 11;
				   break;
		   }
		   break;
	   case 3://DR_3
		   switch(*region)
		   {
			   case LORAMAC_REGION_EU868:
				   *payload_length_max = 115;
				   break;
			   case LORAMAC_REGION_US915:
				   *payload_length_max = 242;
				   break;
			   case LORAMAC_REGION_AU915:
				   *payload_length_max = 115;
				   break;
			   default:
				   *payload_length_max = 11;
				   break;
		   }
		   break;
	   case 4://DR_4
		   switch(*region)
		   {
			   case LORAMAC_REGION_EU868:
				   *payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_US915:
				   *payload_length_max = 242;
				   break;
			   case LORAMAC_REGION_AU915:
				   *payload_length_max = 222;
				   break;
			   default:
				   *payload_length_max = 11;
				   break;
		   }
		   break;
	   case 5://DR_5
		   switch(*region)
		   {
			   case LORAMAC_REGION_EU868:
				   *payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_AU915:
				   *payload_length_max = 222;
				   break;
			   default:
				   *payload_length_max = 11;
				   break;
		   }
		   break;
		case 6://DR_6
		   switch(*region)
		   {
			   case LORAMAC_REGION_EU868:
				   *payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_AU915:
				   *payload_length_max = 222;
				   break;
			   default:
				   *payload_length_max = 11;
				   break;
		   }
		   break;
	}

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
  while (1)
  {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
