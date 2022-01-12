/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lora_app.c
  * @author  MCD Application Team
  * @brief   Application of the LRWAN Middleware
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
#include "platform.h"
#include "Region.h" /* Needed for LORAWAN_DEFAULT_DATA_RATE */
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "utilities_def.h"
#include "lora_app_version.h"
#include "lorawan_version.h"
#include "subghz_phy_version.h"
#include "lora_info.h"
#include "LmHandler.h"
#include "lora_command.h"
#include "lora_at.h"
#include "flash_if.h"
#include "secure-element.h"

/* USER CODE BEGIN Includes */
//bool hopping_done =false;
bool join_hopping = false;
uint8_t num_retry = 0;
uint8_t num_retry_send = 0;
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
static void Get_key(void);
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  join event callback function
  * @param  joinParams status of join
  */
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);

/**
  * @brief  tx event callback function
  * @param  params status of last Tx
  */
static void OnTxData(LmHandlerTxParams_t *params);

/**
  * @brief callback when LoRa application has received a frame
  * @param appData data received in the last Rx
  * @param params status of last Rx
  */
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

/*!
 * Will be called each time a Radio IRQ is handled by the MAC layer
 *
 */
static void OnMacProcessNotify(void);

/**
  * @brief  call back when LoRaWan Stack needs update
  */
static void CmdProcessNotify(void);

/* USER CODE BEGIN PFP */

static void QuitTimeRequest(void *context);
/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
/**
  * @brief LoRaWAN handler Callbacks
  */
static LmHandlerCallbacks_t LmHandlerCallbacks =
{
  .GetBatteryLevel =           GetBatteryLevel,
  .GetTemperature =            GetTemperatureLevel,
  .GetUniqueId =               GetUniqueId,
  .GetDevAddr =                GetDevAddr,
  .OnMacProcess =              OnMacProcessNotify,
  .OnJoinRequest =             OnJoinRequest,
  .OnTxData =                  OnTxData,
  .OnRxData =                  OnRxData
};

/**
  * @brief LoRaWAN handler parameters
  */
static LmHandlerParams_t LmHandlerParams =
{
  .ActiveRegion =             ACTIVE_REGION,
  .DefaultClass =             LORAWAN_DEFAULT_CLASS,
  .AdrEnable =                LORAWAN_ADR_STATE,
  .TxDatarate =               LORAWAN_DEFAULT_DATA_RATE,
  .PingPeriodicity =          LORAWAN_DEFAULT_PING_SLOT_PERIODICITY
};

/* USER CODE BEGIN PV */

static UTIL_TIMER_Object_t JoinRetryTimer;

/* USER CODE BEGIN PV */
static ActivationType_t ActivationType = ACTIVATION_TYPE_OTAA;
/* USER CODE END PV */

/* Exported functions ---------------------------------------------------------*/
/* USER CODE BEGIN EF */

/* USER CODE END EF */

void LoRaWAN_Init(void)
{
  /* USER CODE BEGIN LoRaWAN_Init_1 */

  /* USER CODE END LoRaWAN_Init_1 */

  CMD_Init(CmdProcessNotify);

  APP_LOG(TS_OFF, VLEVEL_ALWAYS,"start\r\n");
  /* USER CODE BEGIN LoRaWAN_Init_1 */

  UTIL_TIMER_Create(&TimeRequestTimer,0xFFFFFFFFU, UTIL_TIMER_ONESHOT, QuitTimeRequest, NULL);

  UTIL_TIMER_SetPeriod(&TimeRequestTimer, 4000);

  /* USER CODE END LoRaWAN_Init_1 */

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_Vcom), UTIL_SEQ_RFU, CMD_Process);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_QuitTimeRequest), UTIL_SEQ_RFU, QuitTimeRequest_Process);

  Get_region();

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  LmHandlerConfigure(&LmHandlerParams);

  Get_key();

  /* USER CODE BEGIN LoRaWAN_Init_Last */

  // APP_PPRINTF("ATtention command interface\r\n");
  // APP_PPRINTF("AT? to list all available functions\r\n");

  /* USER CODE END LoRaWAN_Init_Last */
}

/* USER CODE BEGIN PB_Callbacks */

/* USER CODE END PB_Callbacks */

/* Private functions ---------------------------------------------------------*/
/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  /* USER CODE BEGIN OnRxData_1 */

  /* USER CODE END OnRxData_1 */
  if ((appData != NULL) && (params != NULL))
  {
    AT_event_receive(appData, params);
  }

  /* USER CODE BEGIN OnRxData_2 */

  /* USER CODE END OnRxData_2 */
}

static void OnTxData(LmHandlerTxParams_t *params)
{
  /* USER CODE BEGIN OnTxData_1 */

  /* USER CODE END OnTxData_1 */
  if ((params != NULL) && (params->IsMcpsConfirm != 0))
  {
	 if (Send_test == true && ACK_value == 1)
	 {
		  if(params->AckReceived)
		  {
			  num_retry_send = 0;
			  if (total_number != 0 && send_already_done == true)
			  {
				  UTIL_TIMER_SetPeriod(&Tx_Data, send_timer);
				  UTIL_TIMER_Start(&Tx_Data);
			  }
			  else
			  {
				number_send_ok = 0;
				total_number = 0;
				send_already_done = false;
				if(LmHandlerSetAdrEnable(LORAMAC_HANDLER_ADR_ON)!=LORAMAC_HANDLER_SUCCESS)
				{
//					#ifndef PRINT_RELEASE
						APP_LOG(TS_OFF, VLEVEL_H, "wrong_enable_ADR\r\n");
//						AT_PRINTF("Wrong enable ADR\r\n");
//					#endif
				}
				else
				{
//					#ifndef PRINT_RELEASE
						APP_LOG(TS_OFF, VLEVEL_H, "ok_enable_ADR\r\n");
//						AT_PRINTF("OK enable ADR\r\n");
//					#endif
				}
				#ifndef PRINT_RELEASE
					AT_PRINTF("Finish or stop send\r\n");
				#else
					AT_PRINTF("s0\r\n");
				#endif
			  }

			  Send_test = false;
		  }
		  else
		  {

			  if(num_retry_send < MAX_SEND_RETRY)
			  {
				  #ifndef PRINT_RELEASE
				  	  AT_PRINTF("RETRY ACK SEND\r\n");
				  #else
				  	  AT_PRINTF("ack1\r\n");
				  #endif
				  UTIL_TIMER_SetPeriod(&Tx_Data, 10000);
				  UTIL_TIMER_Start(&Tx_Data);
				  num_retry_send++;
			  }
			  else
			  {

				  number_send_ok = 0;
				  total_number = 0;
				  Send_test = false;
                  #ifndef PRINT_RELEASE
				  	  AT_PRINTF("EXPIRED RETRY\r\n");
                  #else
				  	  AT_PRINTF("s1\r\n");
                  #endif
			  }
		  }
	 }
  }
  AT_event_confirm(params);

  /* USER CODE BEGIN OnTxData_2 */

  /* USER CODE END OnTxData_2 */
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  /* USER CODE BEGIN OnJoinRequest_1 */

  /* USER CODE END OnJoinRequest_1 */
  if (joinParams != NULL)
  {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
    {
    	join_hopping = true;
    	num_retry = 0;
    	if (UTIL_TIMER_IsRunning(&JoinRetryTimer) == 1)
    	{
    		UTIL_TIMER_Stop(&JoinRetryTimer);
    	}
    }
}

  AT_event_join(joinParams);

  /* USER CODE BEGIN OnJoinRequest_2 */

  /* USER CODE END OnJoinRequest_2 */
}

static void CmdProcessNotify(void)
{
  /* USER CODE BEGIN CmdProcessNotify_1 */

  /* USER CODE END CmdProcessNotify_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Vcom), 0);
  /* USER CODE BEGIN CmdProcessNotify_2 */

  /* USER CODE END CmdProcessNotify_2 */
}

static void OnMacProcessNotify(void)
{
  /* USER CODE BEGIN OnMacProcessNotify_1 */

  /* USER CODE END OnMacProcessNotify_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);

  /* USER CODE BEGIN OnMacProcessNotify_2 */

  /* USER CODE END OnMacProcessNotify_2 */
}

//void Clear_flag_hopping(void)
//{
//	uint8_t buff [KEY_PLUS_REGION_DIM];
//	FLASH_Read((void*)buff,(const void*)ADDR_FLASH_PAGE_63, KEY_PLUS_REGION_DIM);
//	FLASH_Erase((void *)ADDR_FLASH_PAGE_63, FLASH_PAGE_SIZE);
//	FLASH_Write(ADDR_FLASH_PAGE_63, buff,  KEY_PLUS_REGION_DIM);
//}

static void QuitTimeRequest(void *context)
{
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_QuitTimeRequest), CFG_SEQ_Prio_0);
}

//void Store_flag_hopping(uint8_t flag)
//{
//	uint8_t buff [KEY_PLUS_REGION_DIM];
//	FLASH_Read((void*)buff,(const void*)ADDR_FLASH_PAGE_63, KEY_PLUS_REGION_DIM);
//    FLASH_Erase((void *)ADDR_FLASH_PAGE_63, FLASH_PAGE_SIZE);
//    FLASH_Write(ADDR_FLASH_PAGE_63, buff,  KEY_PLUS_REGION_DIM);
//}

void QuitTimeRequest_Process(void)
{
//	#ifndef PRINT_RELEASE
		APP_LOG(TS_OFF, VLEVEL_H, "time_expired_retry\r\n");
//		APP_PPRINTF("Time expired, retry\r\n");
//	#endif
	TimeRequestTimerExp = 'f';
}

void Store_region_default(uint8_t default_region)
{
	  uint8_t buff [KEY_PLUS_REGION_DIM];
	  FLASH_Read((void*)buff,(const void*)ADDR_FLASH_PAGE_63, KEY_PLUS_REGION_DIM);
	  FLASH_Erase((void *)ADDR_FLASH_PAGE_63, FLASH_PAGE_SIZE);
	  buff[48] = default_region;
	  FLASH_Write(ADDR_FLASH_PAGE_63, buff, KEY_PLUS_REGION_DIM);
}

void Store_region_new(uint8_t new_region)
{
	uint8_t buff [KEY_PLUS_REGION_DIM];
	FLASH_Read((void*)buff,(const void*)ADDR_FLASH_PAGE_63, KEY_PLUS_REGION_DIM);
	FLASH_Erase((void *)ADDR_FLASH_PAGE_63, FLASH_PAGE_SIZE);
	buff[49] = new_region;
	FLASH_Write(ADDR_FLASH_PAGE_63, buff, KEY_PLUS_REGION_DIM);
}


void Store_key (uint8_t*keys,uint8_t index,uint8_t dim)
{
	uint8_t buff_temp[KEY_DIM];
	FLASH_Read((void*)buff_temp,(const void*)ADDR_FLASH_PAGE_63, KEY_DIM);
	FLASH_Erase((void *)ADDR_FLASH_PAGE_63, FLASH_PAGE_SIZE);
	memcpy(&buff_temp[index],&keys[0],dim);
	FLASH_Write(ADDR_FLASH_PAGE_63, buff_temp, KEY_DIM);
}

static void Get_key(void)
{
	uint8_t keys_get[KEY_DIM];
	uint8_t buff_16_FF[16];
	uint8_t buff_8_FF[8];

	FLASH_Read((void*)keys_get,(const void*)ADDR_FLASH_PAGE_63, KEY_DIM);

	if (memcmp(&keys_get[0],&buff_8_FF[0],8)==0 || memcmp(&keys_get[8],&buff_8_FF[0],8)==0 ||
			memcmp(&keys_get[16],&buff_16_FF[0],16)==0 || memcmp(&keys_get[32],&buff_16_FF[0],16)==0)
	{
//		#ifndef PRINT_RELEASE
			APP_LOG(TS_OFF, VLEVEL_H, "time_expired_retry\r\n");
//			APP_LOG(TS_OFF, VLEVEL_ALWAYS, "### YOU MUST SET THE KEYS !\r\n")
//		#endif
		return;
	}

	 SecureElementSetDevEui(&keys_get[0]);
	 SecureElementSetJoinEui(&keys_get[8]);
	 SecureElementSetKey(APP_KEY, &keys_get[16]);
	 SecureElementSetKey(NWK_KEY, &keys_get[32]);

	 #ifndef PRINT_RELEASE
		 APP_LOG(TS_OFF, VLEVEL_ALWAYS, "### DEV_EUI = %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X \r\n",
				 keys_get[0],keys_get[1],keys_get[2],keys_get[3],keys_get[4],keys_get[5],keys_get[6],keys_get[7]);

		 APP_LOG(TS_OFF, VLEVEL_ALWAYS, "### APP_EUI = %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X \r\n",
					 keys_get[8],keys_get[9],keys_get[10],keys_get[11],keys_get[12],keys_get[13],keys_get[14],keys_get[15]);

		 APP_LOG(TS_OFF, VLEVEL_ALWAYS, "### APP_KEY = %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X \r\n",
					 keys_get[16],keys_get[17],keys_get[18],keys_get[19],keys_get[20],keys_get[21],keys_get[22],keys_get[23],
					 keys_get[24],keys_get[25],keys_get[26],keys_get[27],keys_get[28],keys_get[29],keys_get[30],keys_get[31]);

		 APP_LOG(TS_OFF, VLEVEL_ALWAYS, "### NET_KEY = %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X \r\n",
						 keys_get[32],keys_get[33],keys_get[34],keys_get[35],keys_get[36],keys_get[37],keys_get[38],keys_get[39],
						 keys_get[40],keys_get[41],keys_get[42],keys_get[43],keys_get[44],keys_get[45],keys_get[46],keys_get[47])
	 #endif
	 uint8_t uid[8];

	 GetUniqueId(uid);
	 uint32_t seed_rand=0;
	 seed_rand = ( uint32_t) ( uid[7] << 24 | uid[6] << 16 | uid[5] << 8 | uid[4] );
	 srand1(seed_rand);

}

void Get_region(void)
{
	#ifndef PRINT_RELEASE
		const char *regionStrings[] = { "AS923", "AU915", "CN470", "CN779", "EU433", "EU868", "KR920", "IN865", "US915", "RU864" };
	#endif
	uint8_t buff[KEY_PLUS_REGION_DIM];
	FLASH_Read((void*)buff,(const void*)ADDR_FLASH_PAGE_63, KEY_PLUS_REGION_DIM);
	if (buff[49] != 0xFF && (buff[49] == LORAMAC_REGION_EU868 || buff[49] == LORAMAC_REGION_US915 || buff[49] == LORAMAC_REGION_AU915))
	{
		LmHandlerParams.ActiveRegion = buff[49];
		#ifndef PRINT_RELEASE
			APP_LOG(TS_OFF, VLEVEL_ALWAYS, "### REGION = %s (value: %d)\r\n",regionStrings[buff[49]],LmHandlerParams.ActiveRegion);
		#endif
		return;
	}
	if ((buff[48] == LORAMAC_REGION_EU868 || buff[48] == LORAMAC_REGION_US915 || buff[48] == LORAMAC_REGION_AU915))
	{
		LmHandlerParams.ActiveRegion = buff[48];
		#ifndef PRINT_RELEASE
			APP_LOG(TS_OFF, VLEVEL_ALWAYS, "### REGION = %s (value: %d)\r\n",regionStrings[buff[48]],LmHandlerParams.ActiveRegion);
		#endif
		return;
	}
//	hopping_done = false;
	LmHandlerParams.ActiveRegion = LORAMAC_REGION_EU868;
	#ifndef PRINT_RELEASE
		APP_LOG(TS_OFF, VLEVEL_ALWAYS, "### REGION = %s (value: %d)\r\n",regionStrings[5],LmHandlerParams.ActiveRegion);
	#endif

}

void JoinTry(void*context)
{
  LmHandlerJoin(ActivationType);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
