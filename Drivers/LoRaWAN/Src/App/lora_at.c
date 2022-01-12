/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lora_at.c
  * @author  MCD Application Team
  * @brief   AT command API
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
#include "lora_at.h"
#include "sys_app.h"
#include "stm32_tiny_sscanf.h"
#include "lora_app_version.h"
#include "lorawan_version.h"
#include "subghz_phy_version.h"
#include "test_rf.h"
#include "adc_if.h"
#include "stm32_seq.h"
#include "utilities_def.h"
#include "radio.h"
#include "lora_info.h"
#include "Synchronization_Driver.h"
//#include "tiny_sscanf.h"

/* USER CODE BEGIN Includes */
#include "secure-element.h"
#include "lora_app.h"
#include "Region.h"
#include "LoRaMac.h"
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
//uint8_t flag_join = 'f';
uint8_t TimeRequestTimerExp = 'f';
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
UTIL_TIMER_Object_t TimeRequestTimer;
UTIL_TIMER_Object_t Tx_Data;
bool Send_test = false;
bool mac_send = false;
bool is_rx2_elapsed = false;
bool send_already_done = false;
//INITIAL CSEND
uint32_t number_send_ok = 0;
uint32_t port_tx = 10;
uint32_t number_of_send = 10;
uint32_t ACK_value = 0;
uint32_t send_timer = 10000;
int8_t datarate = 4;
uint32_t txPower = 0;
LoRaMacRegion_t region;

uint32_t total_number = 0;
uint8_t flag_channel_AU = 0;
uint8_t flag_channel_US = 0;
uint32_t payload_length_max = 222;
LmHandlerMsgTypes_t isTxConfirmed = LORAMAC_HANDLER_UNCONFIRMED_MSG;
uint32_t TimeStampSetting=0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static bool ClassBEnableRequest = false;

/*!
 * User application buffer
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * User application data structure
 */
static LmHandlerAppData_t AppData = { 0, 0, AppDataBuffer };

/* Dummy data sent periodically to let the tester respond with start test command*/
static UTIL_TIMER_Object_t TxCertifTimer;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  check if the key is in hex format
  * @param  address of 8 (param_type = 0) or 16 bytes (param_type = 1)
  * @retval 1 if OK 0 otherwise
  */
static uint8_t format_check(const char *param, uint8_t param_length);

/**
  * @brief  Get 4 bytes values in hex
  * @param  from string containing the 16 bytes, something like ab:cd:01:21
  * @param  value buffer that will contain the bytes read
  * @retval The number of bytes read
  */
static int32_t sscanf_uint32_as_hhx(const char *from, uint32_t *value);

/**
  * @brief  Get 16 bytes values in hex
  * @param  from string containing the 16 bytes, something like ab:cd:01:...
  * @param  pt buffer that will contain the bytes read
  * @retval The number of bytes read
  */
static int sscanf_16_hhx(const char *from, uint8_t *pt);

/**
  * @brief  Print 4 bytes as %02x
  * @param  value containing the 4 bytes to print
  */
static void print_uint32_as_02x(uint32_t value);

/**
  * @brief  Print 16 bytes as %02X
  * @param  pt pointer containing the 16 bytes to print
  */
static void print_16_02x(uint8_t *pt);

/**
  * @brief  Print 8 bytes as %02X
  * @param  pt pointer containing the 8 bytes to print
  */
static void print_8_02x(uint8_t *pt);

/**
  * @brief  Print an int
  * @param  value to print
  */
static void print_d(int32_t value);

/**
  * @brief  Print an unsigned int
  * @param  value to print
  */
static void print_u(uint32_t value);

/**
  * @brief  Certif Rejoin timer callback function
  * @param  context ptr of Certif Rejoin context
  */
static void OnCertifTimer(void *context);

/**
  * @brief  Certif send function
  */
static void CertifSend(void);

/**
  * @brief  Check if character in parameter is alphanumeric
  * @param  Char for the alphanumeric check
  */
static int32_t isHex(char Char);

/**
  * @brief  Converts hex string to a nibble ( 0x0X )
  * @param  Char hex string
  * @retval the nibble. Returns 0xF0 in case input was not an hex string (a..f; A..F or 0..9)
  */
static uint8_t Char2Nibble(char Char);

/**
  * @brief  Convert a string into a buffer of data
  * @param  str string to convert
  * @param  data output buffer
  * @param  Size of input string
  */
static int32_t stringToData(const char *str, uint8_t *data, uint32_t Size);

void Tx_Data_Cmd(void *context);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
ATEerror_t AT_return_ok(const char *param)
{
  return AT_OK;
}

ATEerror_t AT_return_error(const char *param)
{
  return AT_ERROR;
}

/* --------------- Application events --------------- */
void AT_event_join(LmHandlerJoinParams_t *params)
{
  /* USER CODE BEGIN AT_event_join_1 */

  /* USER CODE END AT_event_join_1 */
  if ((params != NULL) && (params->Status == LORAMAC_HANDLER_SUCCESS))
  {
	#ifndef PRINT_RELEASE
	  AT_PRINTF("+EVT:JOINED\r\n");
	#else
	  AT_PRINTF("j0\r\n");
	#endif
  }
  else
  {
	#ifndef PRINT_RELEASE
	  AT_PRINTF("+EVT:JOIN FAILED\r\n");
	#else
	  AT_PRINTF("j1\r\n");
    #endif
  }
  /* USER CODE BEGIN AT_event_join_2 */

  /* USER CODE END AT_event_join_2 */
}

void AT_event_receive(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
	/* USER CODE BEGIN AT_event_receive_1 */

	/* USER CODE END AT_event_receive_1 */
	const char *slotStrings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };
	uint8_t ReceivedDataSize = 0;

	if ((appData != NULL) && (appData->BufferSize > 0))
	{
		/* Received data to be copied*/
		if (LORAWAN_APP_DATA_BUFFER_MAX_SIZE <= appData->BufferSize)
		{
			ReceivedDataSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE;
		}
		else
		{
			ReceivedDataSize = appData->BufferSize;
		}
		#ifndef PRINT_RELEASE
			/*asynchronous notification to the host*/
			AT_PRINTF("port=%d:length=%02X:payload=", appData->Port, ReceivedDataSize);
			for (uint8_t i = 0; i < ReceivedDataSize-1; i++)
			{
				AT_PRINTF("%02x:", appData->Buffer[i]);
			}
			AT_PRINTF("%02x\r\n", appData->Buffer[ReceivedDataSize-1]);
		#else
			/*asynchronous notification to the host*/
			AT_PRINTF("rx:%d:%02X:", appData->Port, ReceivedDataSize);
			for (uint8_t i = 0; i < ReceivedDataSize-1; i++)
			{
				AT_PRINTF("%02x:", appData->Buffer[i]);
			}
			AT_PRINTF("%02x\r\n", appData->Buffer[ReceivedDataSize-1]);
    	#endif

	}
	if (params != NULL)
	{
		if (params->LinkCheck == true)
		{
			#ifndef PRINT_RELEASE
				AT_PRINTF("+EVT:RX_%s, DR %d, RSSI %d, SNR %d, DMODM %d, GWN %d\r\n",
						slotStrings[params->RxSlot], params->Datarate, params->Rssi, params->Snr,
						params->DemodMargin, params->NbGateways);
			#else
				AT_PRINTF("%s:%d:%d:%d:%d:%d\r\n",
						slotStrings[params->RxSlot], params->Datarate, params->Rssi, params->Snr,
						params->DemodMargin, params->NbGateways);
			#endif
		}
		else
		{
			#ifndef PRINT_RELEASE
				AT_PRINTF("+EVT:RX_%s, DR %d, RSSI %d, SNR %d\r\n", slotStrings[params->RxSlot],
						params->Datarate, params->Rssi,	params->Snr);
			#else
				AT_PRINTF("%s:%d:%d:%d\r\n", slotStrings[params->RxSlot],
						params->Datarate, params->Rssi,	params->Snr);
			#endif
		}
	}

  /* USER CODE BEGIN AT_event_receive_2 */

  /* USER CODE END AT_event_receive_2 */
}

void AT_event_confirm(LmHandlerTxParams_t *params)
{
	/* USER CODE BEGIN AT_event_confirm_1 */

	/* USER CODE END AT_event_confirm_1 */
	if ((params != NULL) && (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG) && (params->AckReceived != 0))
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("+EVT:SEND_CONFIRMED\r\n");
		#endif
	}
	/* USER CODE BEGIN AT_event_confirm_2 */

	/* USER CODE END AT_event_confirm_2 */
}

/* --------------- General commands --------------- */
ATEerror_t AT_reset(const char *param)
{
	/* USER CODE BEGIN AT_reset_1 */

	/* USER CODE END AT_reset_1 */
	NVIC_SystemReset();
	/* USER CODE BEGIN AT_reset_2 */

	/* USER CODE END AT_reset_2 */
}

ATEerror_t AT_verbose_get(const char *param)
{
	/* USER CODE BEGIN AT_verbose_get_1 */

	/* USER CODE END AT_verbose_get_1 */
	AT_PRINTF("%u:%u\r\n",UTIL_ADV_TRACE_GetVerboseLevel(),TimeStampSetting);
	return AT_OK;
	/* USER CODE BEGIN AT_verbose_get_2 */

	/* USER CODE END AT_verbose_get_2 */
}

ATEerror_t AT_verbose_set(const char *param)
{
	/* USER CODE BEGIN AT_verbose_set_1 */

	/* USER CODE END AT_verbose_set_1 */
	const char *buf = param;
	int32_t lvl_nb;
	int32_t TimeStampVar;

	/* read and set the verbose level */
	if (2 != tiny_sscanf(buf, "%u:%u", &lvl_nb,&TimeStampVar))
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("AT+VL: verbose level is not well set\r\n");
		#endif
		return AT_ERROR;
	}
	if ((lvl_nb > VLEVEL_H) || (lvl_nb < VLEVEL_OFF))
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("AT+VL: verbose level out of range => 0(VLEVEL_OFF) to 3(VLEVEL_H)\r\n");
		#endif
		return AT_ERROR;
	}

	UTIL_ADV_TRACE_SetVerboseLevel(lvl_nb);
	TimeStampSetting = TimeStampVar;

	return AT_OK;
	/* USER CODE BEGIN AT_verbose_set_2 */

	/* USER CODE END AT_verbose_set_2 */
}

ATEerror_t AT_LocalTime_get(const char *param)
{
  /* USER CODE BEGIN AT_LocalTime_get_1 */

  /* USER CODE END AT_LocalTime_get_1 */
  struct tm localtime;
  SysTime_t UnixEpoch = SysTimeGet();
//  UnixEpoch.Seconds -= 18; /*removing leap seconds*/

  SysTimeLocalTime(UnixEpoch.Seconds,  & localtime);

  AT_PRINTF("%02d/%02d/%04d %02d:%02d:%02d\r\n",
		  localtime.tm_mday, localtime.tm_mon + 1, localtime.tm_year + 1900,
		  localtime.tm_hour, localtime.tm_min, localtime.tm_sec
  	  	  );

  return AT_OK;
  /* USER CODE BEGIN AT_LocalTime_get_2 */

  /* USER CODE END AT_LocalTime_get_2 */
}

ATEerror_t AT_TimeRequest_run(const char *param)
{
	if (LmHandlerJoinStatus() != LORAMAC_HANDLER_SET)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("You have to join before\r\n");
		#else
			AT_PRINTF("rj1\r\n");
		#endif
		return AT_OK;
	}
	TimeRequestSynchNotPeriodic();
	TimeRequestTimerExp = 't';
	UTIL_TIMER_Start(&TimeRequestTimer);
	return AT_OK;
}

/* --------------- Keys, IDs and EUIs management commands --------------- */
ATEerror_t AT_JoinEUI_get(const char *param)
{
	/* USER CODE BEGIN AT_JoinEUI_get_1 */

	/* USER CODE END AT_JoinEUI_get_1 */
	uint8_t appEUI[8];
	if (LmHandlerGetAppEUI(appEUI) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_8_02x(appEUI);
	return AT_OK;
	/* USER CODE BEGIN AT_JoinEUI_get_2 */

	/* USER CODE END AT_JoinEUI_get_2 */
}

ATEerror_t AT_JoinEUI_set(const char *param)
{
	/* USER CODE BEGIN AT_JoinEUI_set_1 */

	/* USER CODE END AT_JoinEUI_set_1 */
	uint8_t JoinEui[8];
	if (tiny_sscanf(param, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				  &JoinEui[0], &JoinEui[1], &JoinEui[2], &JoinEui[3],
				  &JoinEui[4], &JoinEui[5], &JoinEui[6], &JoinEui[7]) != 8)
	{
		return AT_ERROR;
	}

	if(!format_check(param,(3*sizeof(JoinEui)-1)))
	{
		return AT_ERROR;
	}

	if (LORAMAC_HANDLER_SUCCESS != LmHandlerSetAppEUI(JoinEui)) //MODIFICA
	{
		return AT_ERROR;
	}
	Store_key(&JoinEui[0],8,8);
	return AT_OK;
	/* USER CODE BEGIN AT_JoinEUI_set_2 */

	/* USER CODE END AT_JoinEUI_set_2 */
}

ATEerror_t AT_NwkKey_get(const char *param)
{
	/* USER CODE BEGIN AT_NwkKey_get_1 */

	/* USER CODE END AT_NwkKey_get_1 */
	uint8_t nwkKey[16];
	if (LORAMAC_HANDLER_SUCCESS != LmHandlerGetNwkKey(nwkKey))
	{
		return AT_ERROR;
	}
	print_16_02x(nwkKey);

	return AT_OK;
	/* USER CODE BEGIN AT_NwkKey_get_2 */

	/* USER CODE END AT_NwkKey_get_2 */
}

ATEerror_t AT_NwkKey_set(const char *param)
{
	/* USER CODE BEGIN AT_NwkKey_set_1 */

	/* USER CODE END AT_NwkKey_set_1 */
	uint8_t nwk_key[16];

	if (tiny_sscanf(param, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				  &nwk_key[0], &nwk_key[1], &nwk_key[2], &nwk_key[3],
				  &nwk_key[4], &nwk_key[5], &nwk_key[6], &nwk_key[7],
				  &nwk_key[8], &nwk_key[9], &nwk_key[10], &nwk_key[11],
				  &nwk_key[12], &nwk_key[13], &nwk_key[14], &nwk_key[15]) != 16)
	{
		return AT_ERROR;
	}

	if(!format_check(param,(3*sizeof(nwk_key))-1))
	{
		return AT_ERROR;
	}

	if (SECURE_ELEMENT_SUCCESS != SecureElementSetKey(NWK_KEY, &nwk_key[0]))
	{
		return AT_ERROR;
	}

	Store_key(&nwk_key[0],32,16);
	memcpy(&nwk_key[0],&param[0],16);

	return AT_OK;
	/* USER CODE BEGIN AT_NwkKey_set_2 */

	/* USER CODE END AT_NwkKey_set_2 */
}

ATEerror_t AT_AppKey_get(const char *param)
{
	/* USER CODE BEGIN AT_AppKey_get_1 */

	/* USER CODE END AT_AppKey_get_1 */
	uint8_t appKey[16];
	if (LORAMAC_HANDLER_SUCCESS != LmHandlerGetAppKey(appKey))
	{
		return AT_ERROR;
	}
	print_16_02x(appKey);

	return AT_OK;
	/* USER CODE BEGIN AT_AppKey_get_2 */

	/* USER CODE END AT_AppKey_get_2 */
}

ATEerror_t AT_AppKey_set(const char *param)
{
	/* USER CODE BEGIN AT_AppKey_set_1 */

	/* USER CODE END AT_AppKey_set_1 */
	uint8_t appKey[16];
	if (tiny_sscanf(param, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				  &appKey[0], &appKey[1], &appKey[2], &appKey[3],
				  &appKey[4], &appKey[5], &appKey[6], &appKey[7],
				  &appKey[8], &appKey[9], &appKey[10], &appKey[11],
				  &appKey[12], &appKey[13], &appKey[14], &appKey[15]) != 16)
	{
		return AT_ERROR;
	}

	if(!format_check(param,(3*sizeof(appKey))-1))
	{
		return AT_ERROR;
	}

	if (SECURE_ELEMENT_SUCCESS != SecureElementSetKey(APP_KEY, &appKey[0]))
	{
		return AT_ERROR;
	}

	Store_key(&appKey[0],16,16);
	memcpy(&appKey[0],&param[0],16);

	return AT_OK;
	/* USER CODE BEGIN AT_AppKey_set_2 */

	/* USER CODE END AT_AppKey_set_2 */
}

ATEerror_t AT_NwkSKey_get(const char *param)
{
	/* USER CODE BEGIN AT_NwkSKey_get_1 */

	/* USER CODE END AT_NwkSKey_get_1 */
	uint8_t nwkSKey[16];
	if (LORAMAC_HANDLER_SUCCESS != LmHandlerGetNwkSKey(nwkSKey))
	{
		return AT_ERROR;
	}
	print_16_02x(nwkSKey);

	return AT_OK;
	/* USER CODE BEGIN AT_NwkSKey_get_2 */

	/* USER CODE END AT_NwkSKey_get_2 */
}

ATEerror_t AT_NwkSKey_set(const char *param)
{
	/* USER CODE BEGIN AT_NwkSKey_set_1 */

	/* USER CODE END AT_NwkSKey_set_1 */
	uint8_t nwkSKey[16];

	if (sscanf_16_hhx(param, nwkSKey) != 16)
	{
		return AT_ERROR;
	}

	if(!format_check(param,(3*sizeof(nwkSKey))-1))
	{
		return AT_ERROR;
	}

	if (LORAMAC_HANDLER_SUCCESS != LmHandlerSetNwkSKey(nwkSKey))
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_NwkSKey_set_2 */

	/* USER CODE END AT_NwkSKey_set_2 */
}

ATEerror_t AT_AppSKey_get(const char *param)
{
	/* USER CODE BEGIN AT_AppSKey_get_1 */

	/* USER CODE END AT_AppSKey_get_1 */
	uint8_t appSKey[16];
	if (LORAMAC_HANDLER_SUCCESS != LmHandlerGetAppSKey(appSKey))
	{
		return AT_ERROR;
	}
	print_16_02x(appSKey);

	return AT_OK;
	/* USER CODE BEGIN AT_AppSKey_get_2 */

	/* USER CODE END AT_AppSKey_get_2 */
}

ATEerror_t AT_AppSKey_set(const char *param)
{
	/* USER CODE BEGIN AT_AppSKey_set_1 */

	/* USER CODE END AT_AppSKey_set_1 */
	uint8_t appSKey[16];

	if (sscanf_16_hhx(param, appSKey) != 16)
	{
		return AT_ERROR;
	}

	if(!format_check(param,(3*sizeof(appSKey))-1))
	{
		return AT_ERROR;
	}

	if (LORAMAC_HANDLER_SUCCESS != LmHandlerSetAppSKey(appSKey))
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_AppSKey_set_2 */

	/* USER CODE END AT_AppSKey_set_2 */
}

ATEerror_t AT_DevAddr_get(const char *param)
{
	/* USER CODE BEGIN AT_DevAddr_get_1 */

	/* USER CODE END AT_DevAddr_get_1 */
	uint32_t devAddr;
	if (LmHandlerGetDevAddr(&devAddr) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_uint32_as_02x(devAddr);
	return AT_OK;
	/* USER CODE BEGIN AT_DevAddr_get_2 */

	/* USER CODE END AT_DevAddr_get_2 */
}

ATEerror_t AT_DevAddr_set(const char *param)
{
	/* USER CODE BEGIN AT_DevAddr_set_1 */

	/* USER CODE END AT_DevAddr_set_1 */
	uint32_t devAddr;

	if (sscanf_uint32_as_hhx(param, &devAddr) != 4)
	{
		return AT_ERROR;
	}

	if(!format_check(param,sizeof(devAddr)*3-1))
	{
		return AT_ERROR;
	}

	if (LORAMAC_HANDLER_SUCCESS != LmHandlerSetDevAddr(devAddr))
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_DevAddr_set_2 */

	/* USER CODE END AT_DevAddr_set_2 */
}

ATEerror_t AT_DevEUI_get(const char *param)
{
	/* USER CODE BEGIN AT_DevEUI_get_1 */

	/* USER CODE END AT_DevEUI_get_1 */
	uint8_t devEUI[8];
	if (LmHandlerGetDevEUI(devEUI) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_8_02x(devEUI);
	return AT_OK;
	/* USER CODE BEGIN AT_DevEUI_get_2 */

	/* USER CODE END AT_DevEUI_get_2 */
}

ATEerror_t AT_DevEUI_set(const char *param)
{
	/* USER CODE BEGIN AT_DevEUI_set_1 */

	/* USER CODE END AT_DevEUI_set_1 */
	uint8_t devEui[8];

	if (tiny_sscanf(param, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				  &devEui[0], &devEui[1], &devEui[2], &devEui[3],
				  &devEui[4], &devEui[5], &devEui[6], &devEui[7]) != 8)
	{
		return AT_ERROR;
	}

	if(!format_check(param,(3*sizeof(devEui))-1))
	{
		return AT_ERROR;
	}

	if (LORAMAC_HANDLER_SUCCESS != LmHandlerSetDevEUI(devEui))
	{
		return AT_ERROR;
	}

	Store_key(&devEui[0],0,8);
	return AT_OK;
	/* USER CODE BEGIN AT_DevEUI_set_2 */

	/* USER CODE END AT_DevEUI_set_2 */
}

ATEerror_t AT_NetworkID_get(const char *param)
{
	/* USER CODE BEGIN AT_NetworkID_get_1 */

	/* USER CODE END AT_NetworkID_get_1 */
	uint32_t networkId;
	if (LmHandlerGetNetworkID(&networkId) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_d(networkId); //MODIFICA
	return AT_OK;
	/* USER CODE BEGIN AT_NetworkID_get_2 */

	/* USER CODE END AT_NetworkID_get_2 */
}

ATEerror_t AT_NetworkID_set(const char *param)
{
	/* USER CODE BEGIN AT_NetworkID_set_1 */

	/* USER CODE END AT_NetworkID_set_1 */
	uint32_t networkId;
	if (tiny_sscanf(param, "%u", &networkId) != 1) //modifica
	{
		return AT_ERROR;
	}

	if (networkId > 127)
	{
		return AT_ERROR;
	}

	LmHandlerSetNetworkID(networkId);
	return AT_OK;
	/* USER CODE BEGIN AT_NetworkID_set_2 */

	/* USER CODE END AT_NetworkID_set_2 */
}

/* --------------- LoRaWAN join and send data commands --------------- */
ATEerror_t AT_Join(const char *param)
{
	/* USER CODE BEGIN AT_Join_1 */

	/* USER CODE END AT_Join_1 */
	if (LmHandlerJoinStatus() == LORAMAC_HANDLER_SET)
	{
		#ifndef PRINT_RELEASE
				AT_PRINTF("You have already joined\r\n");
		#else
				AT_PRINTF("j2\r\n");
		#endif
		return AT_OK;
	}
	switch (param[0])
	{
	case '0':
		LmHandlerJoin(ACTIVATION_TYPE_ABP);
		break;
	case '1':
		LmHandlerJoin(ACTIVATION_TYPE_OTAA);
		break;
	default:
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_Join_2 */

	/* USER CODE END AT_Join_2 */
}

ATEerror_t AT_Link_Check(const char *param)
{
	/* USER CODE BEGIN AT_Link_Check_1 */

	/* USER CODE END AT_Link_Check_1 */
	if (LmHandlerJoinStatus() != LORAMAC_HANDLER_SET)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("You have to join before\r\n");
		#else
			AT_PRINTF("rj1\r\n");
		#endif
		return AT_OK;
	}
	if (LmHandlerLinkCheckReq() != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_Link_Check_2 */

	/* USER CODE END AT_Link_Check_2 */
}

ATEerror_t AT_Get_Channel(const char *param)
{
//	LoRaMacRegion_t region;
	uint16_t channel_mask_app[6];
	uint16_t channel_mask[6];
	uint8_t i, j, id, k;
	uint16_t app;
	if (LmHandlerGetActiveRegion(&region) != LORAMAC_HANDLER_SUCCESS)
	{
		#ifndef PRINT_RELEASE
	    	AT_PRINTF("Error get region. Make sure you have joined\r\n");
		#endif
		return AT_ERROR;
	}
	switch (region)
	{
		case LORAMAC_REGION_EU868:
			for (id=0;id<16;id++)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Channel id %d Dr max-min %d-%d Band %d Freq %d\r\n",
							id,Nvm.RegionGroup2.Channels[id].DrRange.Fields.Max,
							Nvm.RegionGroup2.Channels[id].DrRange.Fields.Min,
							Nvm.RegionGroup2.Channels[id].Band,
							Nvm.RegionGroup2.Channels[id].Frequency);
				#else
					AT_PRINTF("%d:%d_%d:%d:%d\r\n",
							id,Nvm.RegionGroup2.Channels[id].DrRange.Fields.Max,
							Nvm.RegionGroup2.Channels[id].DrRange.Fields.Min,
							Nvm.RegionGroup2.Channels[id].Band,
							Nvm.RegionGroup2.Channels[id].Frequency);
				#endif

			}

			channel_mask_app[0] = Nvm.RegionGroup2.ChannelsMask[0];

			channel_mask[0] = 0;

			j=16;
			while (j!=0)
			{
				app = channel_mask_app[0] & 0x0001;
				channel_mask[0] = channel_mask[0] | (app<<(j-1));
				j = j-1;
				channel_mask_app[0] = channel_mask_app[0] >> 1;
			}

			#ifndef PRINT_RELEASE
				AT_PRINTF("Mask %04X \r\n",channel_mask[0]);
			#else
				AT_PRINTF("%04X\r\n",channel_mask[0]);
			#endif

			break;

		case LORAMAC_REGION_US915:
			for (id=0;id<72;id++)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Channel id %d Dr max-min %d-%d Band %d Freq %d\r\n",
							id,Nvm.RegionGroup2.Channels[id].DrRange.Fields.Max,
							Nvm.RegionGroup2.Channels[id].DrRange.Fields.Min,
							Nvm.RegionGroup2.Channels[id].Band,
							Nvm.RegionGroup2.Channels[id].Frequency);
				#else
					AT_PRINTF("%d:%d_%d:%d:%d\r\n",
							id,Nvm.RegionGroup2.Channels[id].DrRange.Fields.Max,
							Nvm.RegionGroup2.Channels[id].DrRange.Fields.Min,
							Nvm.RegionGroup2.Channels[id].Band,
							Nvm.RegionGroup2.Channels[id].Frequency);
				#endif
				HAL_Delay(100);
			}

			channel_mask_app[0] = Nvm.RegionGroup2.ChannelsMask[0];
			channel_mask_app[1] = Nvm.RegionGroup2.ChannelsMask[1];
			channel_mask_app[2] = Nvm.RegionGroup2.ChannelsMask[2];
			channel_mask_app[3] = Nvm.RegionGroup2.ChannelsMask[3];
			channel_mask_app[4] = Nvm.RegionGroup2.ChannelsMask[4];
			channel_mask_app[5] = Nvm.RegionGroup2.ChannelsMask[5];

			channel_mask[0] = 0;
			channel_mask[1] = 0;
			channel_mask[2] = 0;
			channel_mask[3] = 0;
			channel_mask[4] = 0;
			channel_mask[5] = 0;

			for(i=0;i<6;i++)
			{
				j=16;
				while (j!=0)
				{
					app = channel_mask_app[i] & 0x0001;
					channel_mask[i] = channel_mask[i] | (app<<(j-1));
					j = j-1;
					channel_mask_app[i] = channel_mask_app[i] >> 1;
				}
			}

			#ifndef PRINT_RELEASE
				AT_PRINTF("Mask:");
			#endif
			for (k=0;k<5;k++)
			{
				AT_PRINTF("%04X:",channel_mask[k]);
				HAL_Delay(100);
			}
			AT_PRINTF("%04X\r\n",channel_mask[k]);

			break;

		case LORAMAC_REGION_AU915:
			for (id=0;id<72;id++)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Channel id %d Dr max-min %d-%d Band %d Freq %d\r\n",
							id,Nvm.RegionGroup2.Channels[id].DrRange.Fields.Max,
							Nvm.RegionGroup2.Channels[id].DrRange.Fields.Min,
							Nvm.RegionGroup2.Channels[id].Band,
							Nvm.RegionGroup2.Channels[id].Frequency);
				#else
					AT_PRINTF("%d:%d_%d:%d:%d\r\n",
							id,Nvm.RegionGroup2.Channels[id].DrRange.Fields.Max,
							Nvm.RegionGroup2.Channels[id].DrRange.Fields.Min,
							Nvm.RegionGroup2.Channels[id].Band,
							Nvm.RegionGroup2.Channels[id].Frequency);
				#endif
				HAL_Delay(100);
			}

			channel_mask_app[0] = Nvm.RegionGroup2.ChannelsMask[0];
			channel_mask_app[1] = Nvm.RegionGroup2.ChannelsMask[1];
			channel_mask_app[2] = Nvm.RegionGroup2.ChannelsMask[2];
			channel_mask_app[3] = Nvm.RegionGroup2.ChannelsMask[3];
			channel_mask_app[4] = Nvm.RegionGroup2.ChannelsMask[4];
			channel_mask_app[5] = Nvm.RegionGroup2.ChannelsMask[5];

			channel_mask[0] = 0;
			channel_mask[1] = 0;
			channel_mask[2] = 0;
			channel_mask[3] = 0;
			channel_mask[4] = 0;
			channel_mask[5] = 0;

			for(i=0;i<6;i++)
			{
				j=16;
				while (j!=0)
				{
					app = channel_mask_app[i] & 0x0001;
					channel_mask[i] = channel_mask[i] | (app<<(j-1));
					j = j-1;
					channel_mask_app[i] = channel_mask_app[i] >> 1;
				}
			}

			#ifndef PRINT_RELEASE
				AT_PRINTF("Mask:");
			#endif
			for (k=0;k<5;k++)
			{
				AT_PRINTF("%04X:",channel_mask[k]);
				HAL_Delay(100);
			}
			AT_PRINTF("%04X\r\n",channel_mask[k]);

			break;

		default:
			return AT_ERROR;
	}

   return AT_OK;
}

ATEerror_t AT_Add_Remove_Channel(const char *param)
{
	/* USER CODE BEGIN AT_Add_Remove_Channel */

	/* USER CODE END AT_Add_Remove_Channel */

//	LoRaMacRegion_t region;
	ChanMaskSetParams_t chanMaskSet;
	uint16_t channel_mask_app[6];
	uint16_t channel_mask[6];
	uint16_t app;
	uint8_t channel_mask_app_8[12];
	uint8_t ones_counter=0, ones_counter_last=0;
	uint8_t i,j;

	if (LmHandlerJoinStatus() != LORAMAC_HANDLER_SET)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("You have to join before\r\n");
		#else
			AT_PRINTF("rj1\r\n");
		#endif
		return AT_OK;
	}

	if (LmHandlerGetActiveRegion(&region) != LORAMAC_HANDLER_SUCCESS)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Error get region. Make sure you have joined\r\n");
		#endif
		return AT_ERROR;
	}

    switch(region)
    {
    	case LORAMAC_REGION_AU915:
    		// Check if the command format is right
			if (12 != tiny_sscanf(param, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
					&channel_mask_app_8[0],
					&channel_mask_app_8[1],
					&channel_mask_app_8[2],
					&channel_mask_app_8[3],
					&channel_mask_app_8[4],
					&channel_mask_app_8[5],
					&channel_mask_app_8[6],
					&channel_mask_app_8[7],
					&channel_mask_app_8[8],
					&channel_mask_app_8[9],
					&channel_mask_app_8[10],
					&channel_mask_app_8[11]))
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Param error\r\n");
				#endif
				return AT_ERROR;
			}

			channel_mask_app[0] = ((channel_mask_app_8[0]<<8)&0xFF00)|(channel_mask_app_8[1]&0x00FF);
			channel_mask_app[1] = ((channel_mask_app_8[2]<<8)&0xFF00)|(channel_mask_app_8[3]&0x00FF);
			channel_mask_app[2] = ((channel_mask_app_8[4]<<8)&0xFF00)|(channel_mask_app_8[5]&0x00FF);
			channel_mask_app[3] = ((channel_mask_app_8[6]<<8)&0xFF00)|(channel_mask_app_8[7]&0x00FF);
			channel_mask_app[4] = ((channel_mask_app_8[8]<<8)&0xFF00)|(channel_mask_app_8[9]&0x00FF);
			channel_mask_app[5] = ((channel_mask_app_8[11]<<8)&0xFF00)|(channel_mask_app_8[10]&0x00FF);

			channel_mask[0] = 0;
			channel_mask[1] = 0;
			channel_mask[2] = 0;
			channel_mask[3] = 0;
			channel_mask[4] = 0;
			channel_mask[5] = 0;

			for(i=0;i<6;i++)
			{
				j=16;
				while (j!=0)
				{
					app = channel_mask_app[i] & 0x0001;
					channel_mask[i] = channel_mask[i] | (app<<(j-1));
					j = j-1;
					channel_mask_app[i] = channel_mask_app[i] >> 1;
				}
			}

			chanMaskSet.ChannelsMaskIn=&channel_mask[0];
			chanMaskSet.ChannelsMaskType=0;

			// ones check
			for (i=0;i<4;i++)
			{
				for (j=0;j<8;j++)
				{
					if ( ((channel_mask[i]>>j)&0x0001) == 1)
					{
						ones_counter++;
					}
				}
			}
			for (j=0;j<8;j++)
			{
				if ( ((channel_mask[i]>>j)&0x0001) == 1)
				{
					ones_counter_last++;
				}
			}
			if (ones_counter <= 2)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Less than 3 channel chosen, error\r\n");
				#endif
				return AT_ERROR;
			}
			if (ones_counter_last > 2)
			{
				flag_channel_AU = 1;
			}
			else
			{
				flag_channel_AU = 0;
			}

			if(RegionChanMaskSet(region,&chanMaskSet) == false)
			{
				return AT_ERROR;
			}
			return AT_OK;
    		break;

    	case LORAMAC_REGION_US915:
    		// Check if the command format is right
			if (12 != tiny_sscanf(param, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
					&channel_mask_app_8[0],
					&channel_mask_app_8[1],
					&channel_mask_app_8[2],
					&channel_mask_app_8[3],
					&channel_mask_app_8[4],
					&channel_mask_app_8[5],
					&channel_mask_app_8[6],
					&channel_mask_app_8[7],
					&channel_mask_app_8[8],
					&channel_mask_app_8[9],
					&channel_mask_app_8[10],
					&channel_mask_app_8[11]))
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Param error\r\n");
				#endif
				return AT_ERROR;
			}

			channel_mask_app[0] = ((channel_mask_app_8[0]<<8)&0xFF00)|(channel_mask_app_8[1]&0x00FF);
			channel_mask_app[1] = ((channel_mask_app_8[2]<<8)&0xFF00)|(channel_mask_app_8[3]&0x00FF);
			channel_mask_app[2] = ((channel_mask_app_8[4]<<8)&0xFF00)|(channel_mask_app_8[5]&0x00FF);
			channel_mask_app[3] = ((channel_mask_app_8[6]<<8)&0xFF00)|(channel_mask_app_8[7]&0x00FF);
			channel_mask_app[4] = ((channel_mask_app_8[8]<<8)&0xFF00)|(channel_mask_app_8[9]&0x00FF);
			channel_mask_app[5] = ((channel_mask_app_8[11]<<8)&0xFF00)|(channel_mask_app_8[10]&0x00FF);

			channel_mask[0] = 0;
			channel_mask[1] = 0;
			channel_mask[2] = 0;
			channel_mask[3] = 0;
			channel_mask[4] = 0;
			channel_mask[5] = 0;

			for(i=0;i<6;i++)
			{
				j=16;
				while (j!=0)
				{
					app = channel_mask_app[i] & 0x0001;
					channel_mask[i] = channel_mask[i] | (app<<(j-1));
					j = j-1;
					channel_mask_app[i] = channel_mask_app[i] >> 1;
				}
			}

			chanMaskSet.ChannelsMaskIn=&channel_mask[0];
			chanMaskSet.ChannelsMaskType=0;

			// ones check
			for (i=0;i<4;i++)
			{
				for (j=0;j<16;j++)
				{
					if ( ((channel_mask[i]>>j)&0x0001) == 1)
					{
						ones_counter++;
					}
				}
			}
			for (j=0;j<8;j++)
			{
				if ( ((channel_mask[i]>>j)&0x0001) == 1)
				{
					ones_counter_last++;
				}
			}
			if (ones_counter <= 2)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Less than 3 channel chosen, error\r\n");
				#endif
				return AT_ERROR;
			}
			if (ones_counter_last > 2)
			{
				flag_channel_US = 1;
			}
			else
			{
				flag_channel_US = 0;
			}

			if(RegionChanMaskSet(region,&chanMaskSet) == false)
			{
				return AT_ERROR;
			}
			return AT_OK;
    		break;

    	default:
			#ifndef PRINT_RELEASE
				AT_PRINTF("Wrong region\r\n");
			#endif
			return AT_ERROR;
    		break;
    }
}

/* --------------- LoRaWAN network management commands --------------- */
ATEerror_t AT_version_get(const char *param)
{
	/* USER CODE BEGIN AT_version_get_1 */

	/* USER CODE END AT_version_get_1 */

	#ifndef PRINT_RELEASE
		AT_PRINTF("FW_VERSION:1.0\r\n");
	#else
		AT_PRINTF("VER:1.0\r\n");
	#endif

	/* Get LoRa APP version*/
//	AT_PRINTF("APP_VERSION:        V%X.%X.%X\r\n",
//			(uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_MAIN_SHIFT),
//			(uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB1_SHIFT),
//			(uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB2_SHIFT));

//	/* Get MW LoraWAN info */
//	AT_PRINTF("MW_LORAWAN_VERSION: V%X.%X.%X\r\n",
//			(uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_MAIN_SHIFT),
//			(uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB1_SHIFT),
//			(uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB2_SHIFT));
//
//	/* Get MW SubGhz_Phy info */
//	AT_PRINTF("MW_RADIO_VERSION:   V%X.%X.%X\r\n",
//			(uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_MAIN_SHIFT),
//			(uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB1_SHIFT),
//			(uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB2_SHIFT));

	return AT_OK;
	/* USER CODE BEGIN AT_version_get_2 */

	/* USER CODE END AT_version_get_2 */
}

ATEerror_t AT_ADR_get(const char *param)
{
	/* USER CODE BEGIN AT_ADR_get_1 */

	/* USER CODE END AT_ADR_get_1 */
	bool adrEnable;
	if (LmHandlerGetAdrEnable(&adrEnable) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_d(adrEnable);
	return AT_OK;
	/* USER CODE BEGIN AT_ADR_get_2 */

	/* USER CODE END AT_ADR_get_2 */
}

ATEerror_t AT_ADR_set(const char *param)
{
	/* USER CODE BEGIN AT_ADR_set_1 */

	/* USER CODE END AT_ADR_set_1 */
	switch (param[0])
	{
	case '0':
	case '1':
		LmHandlerSetAdrEnable(param[0] - '0');
		break;
	default:
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_ADR_set_2 */

	/* USER CODE END AT_ADR_set_2 */
}

ATEerror_t AT_DataRate_get(const char *param)
{
	/* USER CODE BEGIN AT_DataRate_get_1 */

	/* USER CODE END AT_DataRate_get_1 */
	int8_t datarate;
	if (LmHandlerGetTxDatarate(&datarate) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_d(datarate);
	return AT_OK;
	/* USER CODE BEGIN AT_DataRate_get_2 */

	/* USER CODE END AT_DataRate_get_2 */
}

ATEerror_t AT_DataRate_set(const char *param)
{
	/* USER CODE BEGIN AT_DataRate_set_1 */

	/* USER CODE END AT_DataRate_set_1 */
//	LoRaMacRegion_t region;
	if (tiny_sscanf(param, "%hhu", &datarate) != 1)
	{
		return AT_ERROR;
	}

	if (LmHandlerGetActiveRegion(&region) != LORAMAC_HANDLER_SUCCESS)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Error get region. Make sure you have joined\r\n");
		#endif
		return AT_ERROR;
	}

	switch(region)
	{
		case LORAMAC_REGION_EU868:
			if (!(datarate >= 0 && datarate <= 6))
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong EU datarate value\r\n");
				#endif
				return AT_ERROR;
			}
			break;
		case LORAMAC_REGION_US915:
			if (!(datarate >= 0 && datarate <= 4))
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong US datarate value\r\n");
				#endif
				return AT_ERROR;
			}
			break;
		case LORAMAC_REGION_AU915:
			if (!(datarate >= 0 && datarate <= 6))
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong AU datarate value\r\n");
				#endif
				return AT_ERROR;
			}
			break;
		default:
			#ifndef PRINT_RELEASE
				AT_PRINTF("Wrong datarate value\r\n");
			#endif
			return AT_ERROR;
	}

	if (LmHandlerSetTxDatarate(datarate) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	switch(datarate)
	{
	   case 0: //DR_0
		   switch(region)
		   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 11;
				   break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 51;
				   break;
			   default:
				   payload_length_max = 11;
				   break;
		   }
		   break;
	   case 1://DR_1
		   switch(region)
		   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 53;
				   break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 51;
			   default:
				   payload_length_max = 11;
				   break;
		   }
		   break;
	   case 2://DR_2
		   switch(region)
		   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 125;
					break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 51;
					break;
			   default:
				   payload_length_max = 11;
				   break;
		   }
		   break;
	   case 3://DR_3
		   switch(region)
		   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 115;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 242;
					break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 115;
					break;
			   default:
				   payload_length_max = 11;
				   break;
		   }
		   break;
	   case 4://DR_4
		   switch(region)
		   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 242;
					break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 222;
					break;
			   default:
				   payload_length_max = 11;
				   break;
		   }
		   break;
	   case 5://DR_5
		   switch(region)
		   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_US915:
				   #ifndef PRINT_RELEASE
					   AT_PRINTF("Error DR for selected region\r\n");
				   #endif
				   return AT_ERROR;
				   break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 222;
				   break;
			   default:
				   payload_length_max = 11;
				   break;
		   }
		   break;
		case 6://DR_6
		   switch(region)
		   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_US915:
				   #ifndef PRINT_RELEASE
					   AT_PRINTF("Error DR for selected region\r\n");
				   #endif
				   return AT_ERROR;
				   break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 222;
				   break;
			   default:
				   payload_length_max = 11;
				   break;
		   }
		   break;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_DataRate_set_2 */

	/* USER CODE END AT_DataRate_set_2 */
}

ATEerror_t AT_Region_get(const char *param)
{
	/* USER CODE BEGIN AT_Region_get_1 */

	/* USER CODE END AT_Region_get_1 */
	#ifndef PRINT_RELEASE
		const char *regionStrings[] = { "AS923", "AU915", "CN470", "CN779", "EU433", "EU868", "KR920", "IN865", "US915", "RU864" };
	#endif
//	LoRaMacRegion_t region;
	if (LmHandlerGetActiveRegion(&region) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	if (region > LORAMAC_REGION_RU864)
	{
		return AT_ERROR;
	}
	#ifndef PRINT_RELEASE
		AT_PRINTF("%d:%s\r\n", region, regionStrings[region]);
	#else
		AT_PRINTF("%d\r\n", region);
	#endif
	return AT_OK;
	/* USER CODE BEGIN AT_Region_get_2 */

	/* USER CODE END AT_Region_get_2 */
}

ATEerror_t AT_Region_set(const char *param)
{
	/* USER CODE BEGIN AT_Region_set_1 */

	/* USER CODE END AT_Region_set_1 */
//	LoRaMacRegion_t region;
	if (tiny_sscanf(param, "%hhu", &region) != 1)
	{
		return AT_ERROR;
	}
	if (region > LORAMAC_REGION_RU864)
	{
		return AT_ERROR;
	}

	if (region != LORAMAC_REGION_EU868 && region != LORAMAC_REGION_US915 && region != LORAMAC_REGION_AU915)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("REGION IS NOT ACTIVATED!\r\n");
			AT_PRINTF("Try AT+BAND=1 (AU915), AT+BAND=8 (US915) or AT+BAND=5 (EU868)");
		#endif
		return AT_ERROR;
	}

	if (LmHandlerSetActiveRegion(region) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	//  AT_PRINTF("\r\n OK\r\n");
	Store_region_new(region);
	HAL_Delay(500);
	HAL_NVIC_SystemReset();

	return AT_OK;
	/* USER CODE BEGIN AT_Region_set_2 */

	/* USER CODE END AT_Region_set_2 */
}

ATEerror_t AT_DeviceClass_get(const char *param)
{
	/* USER CODE BEGIN AT_DeviceClass_get_1 */

	/* USER CODE END AT_DeviceClass_get_1 */
	DeviceClass_t currentClass;
	LoraInfo_t *loraInfo = LoraInfo_GetPtr();
	if (loraInfo == NULL)
	{
		return AT_ERROR;
	}

	if (LmHandlerGetCurrentClass(&currentClass) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	if ((loraInfo->ClassB == 1) && (ClassBEnableRequest == true) && (currentClass == CLASS_A))
	{
		BeaconState_t beaconState;

		if (LmHandlerGetBeaconState(&beaconState) != LORAMAC_HANDLER_SUCCESS)
		{
			return AT_ERROR;
		}
		if ((beaconState == BEACON_STATE_ACQUISITION) ||
			(beaconState == BEACON_STATE_ACQUISITION_BY_TIME) ||
			(beaconState == BEACON_STATE_REACQUISITION)) /*Beacon_Searching on Class B request*/
		{
			AT_PRINTF("B,S0\r\n");
		}
		else if ((beaconState == BEACON_STATE_LOCKED) || /*Beacon locked on Gateway*/
				 (beaconState == BEACON_STATE_IDLE)   ||
				 (beaconState == BEACON_STATE_GUARD)  ||
				 (beaconState == BEACON_STATE_RX))
		{
			AT_PRINTF("B,S1\r\n");
		}
		else
		{
			AT_PRINTF("B,S2\r\n");
		}
	}
	else /* we are now either in Class B enable or Class C enable*/
	{
		AT_PRINTF("%c\r\n", 'A' + currentClass);
	}

	return AT_OK;
	/* USER CODE BEGIN AT_DeviceClass_get_2 */

	/* USER CODE END AT_DeviceClass_get_2 */
}

ATEerror_t AT_DeviceClass_set(const char *param)
{
	/* USER CODE BEGIN AT_DeviceClass_set_1 */

	/* USER CODE END AT_DeviceClass_set_1 */
	LmHandlerErrorStatus_t errorStatus = LORAMAC_HANDLER_SUCCESS;
	LoraInfo_t *loraInfo = LoraInfo_GetPtr();
	if (loraInfo == NULL)
	{
		return AT_ERROR;
	}

	switch (param[0])
	{
	case 'A':
		if (loraInfo->ClassB == 1)
		{
			ClassBEnableRequest = false;
		}
		errorStatus = LmHandlerRequestClass(CLASS_A);
		break;
	case 'B':
		if (loraInfo->ClassB == 1)
		{
			ClassBEnableRequest = true;
			errorStatus = LmHandlerRequestClass(CLASS_B);  /*Class B AT cmd switch Class B not supported cf.[UM2073]*/
		}
		else
		{
			return AT_NO_CLASS_B_ENABLE;
		}
		break;
	case 'C':
		errorStatus = LmHandlerRequestClass(CLASS_C);
		break;
	default:
		return AT_ERROR;
	}

	if (errorStatus == LORAMAC_HANDLER_NO_NETWORK_JOINED)
	{
		#ifdef PRINT_RELEASE
			AT_PRINTF("rj1\r\n");
		#endif
		return AT_OK;
	}
	else if (errorStatus != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_DeviceClass_set_2 */

	/* USER CODE END AT_DeviceClass_set_2 */
}

ATEerror_t AT_DutyCycle_get(const char *param)
{
	/* USER CODE BEGIN AT_DutyCycle_get_1 */

	/* USER CODE END AT_DutyCycle_get_1 */
	bool dutyCycleEnable;
	if (LmHandlerGetDutyCycleEnable(&dutyCycleEnable) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_d(dutyCycleEnable);
	return AT_OK;
	/* USER CODE BEGIN AT_DutyCycle_get_2 */

	/* USER CODE END AT_DutyCycle_get_2 */
}

ATEerror_t AT_DutyCycle_set(const char *param)
{
	/* USER CODE BEGIN AT_DutyCycle_set_1 */

	/* USER CODE END AT_DutyCycle_set_1 */
	switch (param[0])
	{
	case '0':
	case '1':
		LmHandlerSetDutyCycleEnable(param[0] - '0');
		break;
	default:
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_DutyCycle_set_2 */

	/* USER CODE END AT_DutyCycle_set_2 */
}

ATEerror_t AT_JoinAcceptDelay1_get(const char *param)
{
	/* USER CODE BEGIN AT_JoinAcceptDelay1_get_1 */

	/* USER CODE END AT_JoinAcceptDelay1_get_1 */
	uint32_t rxDelay;
	if (LmHandlerGetJoinRx1Delay(&rxDelay) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_u(rxDelay);
	return AT_OK;
	/* USER CODE BEGIN AT_JoinAcceptDelay1_get_2 */

	/* USER CODE END AT_JoinAcceptDelay1_get_2 */
}

ATEerror_t AT_JoinAcceptDelay1_set(const char *param)
{
	/* USER CODE BEGIN AT_JoinAcceptDelay1_set_1 */

	/* USER CODE END AT_JoinAcceptDelay1_set_1 */
	uint32_t rxDelay;
	if (tiny_sscanf(param, "%lu", &rxDelay) != 1)
	{
		return AT_ERROR;
	}
	if (rxDelay >= (1<<31))
	{
		return AT_ERROR;
	}
	else if (LmHandlerSetJoinRx1Delay(rxDelay) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_JoinAcceptDelay1_set_2 */

	/* USER CODE END AT_JoinAcceptDelay1_set_2 */
}

ATEerror_t AT_JoinAcceptDelay2_get(const char *param)
{
	/* USER CODE BEGIN AT_JoinAcceptDelay2_get_1 */

	/* USER CODE END AT_JoinAcceptDelay2_get_1 */
	uint32_t rxDelay;
	if (LmHandlerGetJoinRx2Delay(&rxDelay) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_u(rxDelay);
	return AT_OK;
	/* USER CODE BEGIN AT_JoinAcceptDelay2_get_2 */

	/* USER CODE END AT_JoinAcceptDelay2_get_2 */
}

ATEerror_t AT_JoinAcceptDelay2_set(const char *param)
{
	/* USER CODE BEGIN AT_JoinAcceptDelay2_set_1 */

	/* USER CODE END AT_JoinAcceptDelay2_set_1 */
	uint32_t rxDelay;
	if (tiny_sscanf(param, "%lu", &rxDelay) != 1)
	{
		return AT_ERROR;
	}
	if (rxDelay >= (1<<31))
	{
		return AT_ERROR;
	}
	else if (LmHandlerSetJoinRx2Delay(rxDelay) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_JoinAcceptDelay2_set_2 */

	/* USER CODE END AT_JoinAcceptDelay2_set_2 */
}

ATEerror_t AT_Rx1Delay_get(const char *param)
{
	/* USER CODE BEGIN AT_Rx1Delay_get_1 */

	/* USER CODE END AT_Rx1Delay_get_1 */
	uint32_t rxDelay;
	if (LmHandlerGetRx1Delay(&rxDelay) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_u(rxDelay);
	return AT_OK;
	/* USER CODE BEGIN AT_Rx1Delay_get_2 */

	/* USER CODE END AT_Rx1Delay_get_2 */
}

ATEerror_t AT_Rx1Delay_set(const char *param)
{
	/* USER CODE BEGIN AT_Rx1Delay_set_1 */

	/* USER CODE END AT_Rx1Delay_set_1 */
	uint32_t rxDelay;
	if (tiny_sscanf(param, "%lu", &rxDelay) != 1)
	{
		return AT_ERROR;
	}

	if (rxDelay >= (1<<31))
	{
		return AT_ERROR;
	}
	else if (LmHandlerSetRx1Delay(rxDelay) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_Rx1Delay_set_2 */

	/* USER CODE END AT_Rx1Delay_set_2 */
}

ATEerror_t AT_Rx2Delay_get(const char *param)
{
	/* USER CODE BEGIN AT_Rx2Delay_get_1 */

	/* USER CODE END AT_Rx2Delay_get_1 */
	uint32_t rxDelay;
	if (LmHandlerGetRx2Delay(&rxDelay) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_u(rxDelay);
	return AT_OK;
	/* USER CODE BEGIN AT_Rx2Delay_get_2 */

	/* USER CODE END AT_Rx2Delay_get_2 */
}

ATEerror_t AT_Rx2Delay_set(const char *param)
{
	/* USER CODE BEGIN AT_Rx2Delay_set_1 */

	/* USER CODE END AT_Rx2Delay_set_1 */
	uint32_t rxDelay;
	if (tiny_sscanf(param, "%lu", &rxDelay) != 1)
	{
		return AT_ERROR;
	}
	if (rxDelay >= (1<<31))
	{
		return AT_ERROR;
	}
	else if (LmHandlerSetRx2Delay(rxDelay) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_Rx2Delay_set_2 */

	/* USER CODE END AT_Rx2Delay_set_2 */
}

ATEerror_t AT_Rx2DataRate_get(const char *param)
{
	/* USER CODE BEGIN AT_Rx2DataRate_get_1 */

	/* USER CODE END AT_Rx2DataRate_get_1 */
	RxChannelParams_t rx2Params;
	LmHandlerGetRX2Params(&rx2Params);
	print_d(rx2Params.Datarate);
	return AT_OK;
	/* USER CODE BEGIN AT_Rx2DataRate_get_2 */

	/* USER CODE END AT_Rx2DataRate_get_2 */
}

ATEerror_t AT_Rx2DataRate_set(const char *param)
{
	/* USER CODE BEGIN AT_Rx2DataRate_set_1 */

	/* USER CODE END AT_Rx2DataRate_set_1 */
	RxChannelParams_t rx2Params;

	/* Get the current configuration of RX2 */
	LmHandlerGetRX2Params(&rx2Params);

	/* Update the Datarate with scanf */
	if (tiny_sscanf(param, "%hhu", &(rx2Params.Datarate)) != 1)
	{
		return AT_ERROR;
	}
	else if ((rx2Params.Datarate < 0) || (rx2Params.Datarate > 15))
	{
		return AT_ERROR;
	}
	else if (LmHandlerSetRX2Params(&rx2Params) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_Rx2DataRate_set_2 */

	/* USER CODE END AT_Rx2DataRate_set_2 */
}

ATEerror_t AT_Rx2Frequency_get(const char *param)
{
	/* USER CODE BEGIN AT_Rx2Frequency_get_1 */

	/* USER CODE END AT_Rx2Frequency_get_1 */
	RxChannelParams_t rx2Params;
	LmHandlerGetRX2Params(&rx2Params);
	print_d(rx2Params.Frequency);
	return AT_OK;
	/* USER CODE BEGIN AT_Rx2Frequency_get_2 */

	/* USER CODE END AT_Rx2Frequency_get_2 */
}

ATEerror_t AT_Rx2Frequency_set(const char *param)
{
	/* USER CODE BEGIN AT_Rx2Frequency_set_1 */

	/* USER CODE END AT_Rx2Frequency_set_1 */
	RxChannelParams_t rx2Params;

	/* Get the current configuration of RX2 */
	LmHandlerGetRX2Params(&rx2Params);

	/* Update the frequency with scanf */
	if (tiny_sscanf(param, "%lu", &(rx2Params.Frequency)) != 1)
	{
		return AT_ERROR;
	}
	else if (LmHandlerSetRX2Params(&rx2Params) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_Rx2Frequency_set_2 */

	/* USER CODE END AT_Rx2Frequency_set_2 */
}

ATEerror_t AT_TransmitPower_get(const char *param)
{
	/* USER CODE BEGIN AT_TransmitPower_get_1 */

	/* USER CODE END AT_TransmitPower_get_1 */
	int8_t txPower;
	if (LmHandlerGetTxPower(&txPower) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	print_d(txPower);
	return AT_OK;
	/* USER CODE BEGIN AT_TransmitPower_get_2 */

	/* USER CODE END AT_TransmitPower_get_2 */
}

ATEerror_t AT_TransmitPower_set(const char *param)
{
	/* USER CODE BEGIN AT_TransmitPower_set_1 */

	/* USER CODE END AT_TransmitPower_set_1 */
	int8_t txPower;
	if (tiny_sscanf(param, "%hhu", &txPower) != 1)
	{
		return AT_ERROR;
	}

	if (LmHandlerSetTxPower(txPower) != LORAMAC_HANDLER_SUCCESS)
	{
		return AT_ERROR;
	}

	return AT_OK;
	/* USER CODE BEGIN AT_TransmitPower_set_2 */

	/* USER CODE END AT_TransmitPower_set_2 */
}

//ATEerror_t AT_PingSlot_get(const char *param)
//{
//	/* USER CODE BEGIN AT_PingSlot_get_1 */
//
//	/* USER CODE END AT_PingSlot_get_1 */
//	uint8_t periodicity;
//
//	if (LmHandlerGetPingPeriodicity(&periodicity) != LORAMAC_HANDLER_SUCCESS)
//	{
//		return AT_ERROR;
//	}
//
//	print_d(periodicity);
//	return AT_OK;
//	/* USER CODE BEGIN AT_PingSlot_get_2 */
//
//	/* USER CODE END AT_PingSlot_get_2 */
//}
//
//ATEerror_t AT_PingSlot_set(const char *param)
//{
//	/* USER CODE BEGIN AT_PingSlot_set_1 */
//
//	/* USER CODE END AT_PingSlot_set_1 */
//	int8_t periodicity;
//
//	if (tiny_sscanf(param, "%hhu", &periodicity) != 1)
//	{
//		return AT_ERROR;
//	}
//	else if ((periodicity > 7) || (periodicity < 0))
//	{
//		return AT_ERROR;
//	}
//	else if (LmHandlerSetPingPeriodicity(periodicity) != LORAMAC_HANDLER_SUCCESS)
//	{
//		return AT_ERROR;
//	}
//
//	return AT_OK;
//	/* USER CODE BEGIN AT_PingSlot_set_2 */
//
//	/* USER CODE END AT_PingSlot_set_2 */
//}

/* --------------- Radio tests commands --------------- */
ATEerror_t AT_test_txTone(const char *param)
{
	/* USER CODE BEGIN AT_test_txTone_1 */

	/* USER CODE END AT_test_txTone_1 */
	if (0U == TST_TxTone())
	{
		return AT_OK;
	}
	else
	{
		return AT_BUSY_ERROR;
	}
	/* USER CODE BEGIN AT_test_txTone_2 */

	/* USER CODE END AT_test_txTone_2 */
}

ATEerror_t AT_test_rxRssi(const char *param)
{
	/* USER CODE BEGIN AT_test_rxRssi_1 */

	/* USER CODE END AT_test_rxRssi_1 */
	if (0U == TST_RxRssi())
	{
		return AT_OK;
	}
	else
	{
		return AT_BUSY_ERROR;
	}
	/* USER CODE BEGIN AT_test_rxRssi_2 */

	/* USER CODE END AT_test_rxRssi_2 */
}

ATEerror_t AT_UID_get(const char *param)
{
	uint8_t Uid[8];
	GetUniqueId(&Uid[0]);
	print_8_02x(&Uid[0]);
	return AT_OK;
}

ATEerror_t AT_Send(const char *param)
{
	/* USER CODE BEGIN AT_Send_1 */

	/* USER CODE END AT_Send_1 */

	if (total_number != 0)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Wait to finish last send\r\n");
		#endif
		return AT_ERROR;
	}
	const char *buf = param;
	uint16_t bufSize = strlen(param);
	unsigned size = 0;
	char hex[3] = {0, 0, 0};
	uint32_t payload_length = 0;
	number_send_ok = 0;

	if (LmHandlerJoinStatus() != LORAMAC_HANDLER_SET)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("You have to join before\r\n");
		#else
			AT_PRINTF("rj1\r\n");
		#endif
		return AT_OK;
	}

	/* read and set the payload length */
	if (1 != tiny_sscanf(buf, "%u:", &payload_length))
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("AT+SEND without the payload length\r\n");
		#endif
		return AT_ERROR;
	}

	if (payload_length > payload_length_max)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Payload length greater than maximum admitted\r\n");
		#endif
		return AT_ERROR;
	}

	/* skip the payload length */
	while (('0' <= buf[0]) && (buf[0] <= '9') && bufSize > 1)
	{
		buf ++;
		bufSize --;
	}

	if ((bufSize == 0) || (':' != buf[0]))
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("AT+SEND missing : character after payload length\r\n");
		#endif
		return AT_ERROR;
	}
	else
	{
		/* skip the char ':' */
		buf ++;
		bufSize --;
	}

	while ((size < payload_length) && (bufSize > 1))
	{
		hex[0] = buf[size * 2];
		hex[1] = buf[size * 2 + 1];
		if (tiny_sscanf(hex, "%hhx", &AppData.Buffer[size]) != 1)
		{
			return AT_ERROR;
		}
		if(!(((hex[0])>=48 && (hex[0]) <= 57) || ((hex[0])>=65 && hex[0] <= 70) || (hex[0]>=97 && hex[0] <= 102)))
		{
			return AT_ERROR;
		}
		if(!(((hex[1])>=48 && (hex[1]) <= 57) || ((hex[1])>=65 && hex[1] <= 70) || (hex[1]>=97 && hex[1] <= 102)))
		{
			return AT_ERROR;
		}
		size++;
		bufSize -= 2;
	}
	if (bufSize != 0)
	{
		return AT_ERROR;
	}

	AppData.Port = port_tx;
	AppData.BufferSize = payload_length;

	//Start timer for sending
	UTIL_TIMER_Create(&Tx_Data, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, Tx_Data_Cmd, NULL);
	UTIL_TIMER_SetPeriod(&Tx_Data, 200);
	UTIL_TIMER_Start(&Tx_Data);
	total_number = number_of_send;
	AppData.BufferSize = payload_length;
	send_already_done = true;
	Send_test = true;

	return AT_OK;
}

ATEerror_t AT_ConfigSend_get(const char *param)
{
	#ifndef PRINT_RELEASE
	AT_PRINTF("Settings: Port = %d | Number of packet send = %u | ACK = %d | "
				"Send timer = %u | Data rate = %d | Power level = %d"
				" | Maximum size of packages = %u Bytes\r\n",port_tx,number_of_send,ACK_value,send_timer,
				datarate,txPower,payload_length_max);
	AT_PRINTF("\r\ncan be copy/paste in set cmd: AT+CFGSEND=%d:%d:%d:%d:%d:%d\r\n",
				port_tx,number_of_send,ACK_value,send_timer,datarate,txPower);
	#else
	AT_PRINTF("%d:%u:%d:%u:%u:%d:%d\r\n",
			port_tx,number_of_send,ACK_value,send_timer,payload_length_max,datarate,txPower);
	#endif
	return AT_OK;
}

ATEerror_t AT_ConfigSend_set(const char *param)
{
//	LoRaMacRegion_t region;
	uint8_t j;
	uint16_t channel_mask;
	uint8_t ones_counter_last=0;
	uint8_t flag_send_timer=0;

	if (send_already_done == true)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("A send is already running\r\n");
		#endif
		return AT_ERROR;
	}

	// Check if the command format is right
	if (6 != tiny_sscanf(param, "%u:%u:%u:%u:%u:%u",
			&port_tx,
			&number_of_send,
			&ACK_value,
			&send_timer,
			&datarate,
			&txPower))
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("AT+SEND without the application port\r\n");
		#endif
		return AT_ERROR;
	}

	if(port_tx < 1 || port_tx > 199)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Wrong application port\r\n");
		#endif
		return AT_ERROR;
	}

    if (number_of_send <= 0 || number_of_send > 0x7FFFFFFF)
    {
		#ifndef PRINT_RELEASE
    		AT_PRINTF("Wrong number of send \r\n");
		#endif
    	return AT_ERROR;
    }

	/* read and set the ack */
	switch(ACK_value)
	{
	    case 0:
	    	isTxConfirmed = LORAMAC_HANDLER_UNCONFIRMED_MSG;
	    	break;
	    case 1:
	    	isTxConfirmed = LORAMAC_HANDLER_CONFIRMED_MSG;
	    	break;
	    default:
			#ifndef PRINT_RELEASE
	    		AT_PRINTF("AT+SEND without the acknowledge flag\r\n");
			#endif
	    	return AT_ERROR;
	}

    if (send_timer <= 0 || send_timer > 0x7FFFFFFF)
    {
		#ifndef PRINT_RELEASE
    		AT_PRINTF("Wrong send timer\r\n");
		#endif
    	return AT_ERROR;
    }

	if (LmHandlerGetActiveRegion(&region) != LORAMAC_HANDLER_SUCCESS)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Error get region. Make sure you have joined\r\n");
		#endif
		return AT_ERROR;
	}

    switch(region)
    {
    	case LORAMAC_REGION_EU868:
    		if (datarate < 0 || datarate > 6)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong EU datarate value\r\n");
				#endif
				return AT_ERROR;
			}

    		break;

    	case LORAMAC_REGION_US915:
    		if (!(datarate >= 0 && datarate <= 4))
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong US datarate value\r\n");
				#endif
				return AT_ERROR;
			}

    		if (datarate == 4)
			{
    			// ones check DR4(6) in USA(AU) band
				channel_mask = Nvm.RegionGroup2.ChannelsMask[5];
				for (j=0;j<8;j++)
				{
					if ( ((channel_mask>>j)&0x0001) == 1)
					{
						ones_counter_last++;
					}
				}
				if (ones_counter_last > 2)
				{
					flag_send_timer = 1;
				}
				else
				{
					flag_send_timer = 0;
				}
				if ((send_timer <= 10000) && (flag_send_timer == 0))
				{
					#ifndef PRINT_RELEASE
						AT_PRINTF("Send timer too short\r\n");
					#endif
					return AT_ERROR;
				}
			}
    		break;

    	case LORAMAC_REGION_AU915:
    		if (!(datarate >= 0 && datarate <= 6))
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong AU datarate value\r\n");
				#endif
				return AT_ERROR;
			}

    		if (datarate == 6)
			{
				channel_mask = Nvm.RegionGroup2.ChannelsMask[5];
				for (j=0;j<8;j++)
				{
					if ( ((channel_mask>>j)&0x0001) == 1)
					{
						ones_counter_last++;
					}
				}
				if (ones_counter_last > 2)
				{
					flag_send_timer = 1;
				}
				else
				{
					flag_send_timer = 0;
				}
				if ((send_timer <= 10000) && (flag_send_timer == 0))
				{
					#ifndef PRINT_RELEASE
						AT_PRINTF("Send timer too short\r\n");
					#endif
					return AT_ERROR;
				}
			}
    		break;

    	default:
			#ifndef PRINT_RELEASE
				AT_PRINTF("Wrong datarate value\r\n");
			#endif
			return AT_ERROR;
    }

    if(LmHandlerSetAdrEnable(LORAMAC_HANDLER_ADR_OFF)!=LORAMAC_HANDLER_SUCCESS)
	{
		#ifndef PRINT_RELEASE
    		AT_PRINTF("Wrong Disable ADR\r\n");
		#endif
    	return AT_ERROR;
	}

    if (LmHandlerSetTxPower(txPower) != LORAMAC_HANDLER_SUCCESS)
	{
		#ifndef PRINT_RELEASE
    		AT_PRINTF("Wrong Set Tx Power. Make sure you have joined\r\n");
		#endif
    	return AT_ERROR;
	}
	if (LmHandlerSetTxDatarate(datarate) != LORAMAC_HANDLER_SUCCESS)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Wrong Set Data Rate. Make sure you have joined\r\n");
		#endif
		return AT_ERROR;
	}

    switch(datarate)
    {
       case 0: //DR_0
    	   switch(region)
    	   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 11;
				   break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 51;
			 	   break;
			   default:
				   payload_length_max = 11;
				   break;
    	   }
    	   break;
       case 1://DR_1
    	   switch(region)
    	   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 53;
				   break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 51;
			   default:
				   payload_length_max = 11;
				   break;
    	   }
    	   break;
       case 2://DR_2
    	   switch(region)
    	   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 51;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 125;
			   		break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 51;
			   		break;
			   default:
				   payload_length_max = 11;
				   break;
    	   }
    	   break;
       case 3://DR_3
    	   switch(region)
    	   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 115;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 242;
			   		break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 115;
			   		break;
			   default:
				   payload_length_max = 11;
				   break;
    	   }
    	   break;
       case 4://DR_4
    	   switch(region)
    	   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_US915:
				   payload_length_max = 242;
			   		break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 222;
			   		break;
			   default:
				   payload_length_max = 11;
				   break;
    	   }
           break;
       case 5://DR_5
    	   switch(region)
    	   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_US915:
				   #ifndef PRINT_RELEASE
				   	   AT_PRINTF("Error DR for selected region\r\n");
				   #endif
				   return AT_ERROR;
				   break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 222;
				   break;
			   default:
				   payload_length_max = 11;
				   break;
    	   }
		   break;
        case 6://DR_6
		   switch(region)
		   {
			   case LORAMAC_REGION_EU868:
				   payload_length_max = 222;
				   break;
			   case LORAMAC_REGION_US915:
				   #ifndef PRINT_RELEASE
					   AT_PRINTF("Error DR for selected region\r\n");
				   #endif
				   return AT_ERROR;
				   break;
			   case LORAMAC_REGION_AU915:
				   payload_length_max = 222;
				   break;
			   default:
				   payload_length_max = 11;
				   break;
		   }
		   break;
    }

	return AT_OK;
}

ATEerror_t AT_StopSend(const char *param)
{
	number_send_ok = 0;
	total_number = 0;
	Send_test = false;
	UTIL_TIMER_Stop(&Tx_Data);

	if(LmHandlerSetAdrEnable(LORAMAC_HANDLER_ADR_ON)!=LORAMAC_HANDLER_SUCCESS)
	{
		//AT_PRINTF("Wrong enable ADR\r\n");
	}
	else
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("OK enable ADR\r\n");
		#endif
	}
	send_already_done = false;
	return AT_OK;
}

ATEerror_t AT_Cw_Stop(const char *param)
{
	if (UTIL_TIMER_IsRunning(&TxTimeoutTimer) == 1)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Set stop CW timer\r\n");
		#endif
		TimerStop(&TxTimeoutTimer);
		TimerSetValue(&TxTimeoutTimer, 500);
		TimerStart(&TxTimeoutTimer);

	}
	else
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("No continuous wave mode to stop\r\n");
		#endif
	}
	return AT_OK;
}

ATEerror_t AT_Cw_set(const char *param)
{
	char timeout_str[10]={'\0'};
	char frequency_str[10]={'\0'};
	char power_str[10]={'\0'};
	int32_t timeout,frequency;
	int8_t power;
	uint8_t i = 0;
    uint8_t j = 0;
//    LoRaMacRegion_t region;

    if (LmHandlerJoinStatus() != LORAMAC_HANDLER_SET)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("You have to join before\r\n");
		#else
			AT_PRINTF("rj1\r\n");
		#endif
		return AT_OK;
	}

    //Check if the command format is right
    for (i=0; param[i]!='\0';i++)
    {
    	// Is not a number or a ":"
		if ((param[i]<'0' && param[i]!= ':')  || (param[i]>'9' && param[i]!=':'))
		{
			#ifndef PRINT_RELEASE
				AT_PRINTF("Wrong character\r\n");
			#endif
			return AT_ERROR;
		}

		else if (param[i] == ':' && i == 0)
		{
			#ifndef PRINT_RELEASE
				AT_PRINTF("First character is ':' \r\n");
			#endif
			return AT_ERROR;
		}

		else if (param[i]==':' && param[i-1]==':')
		{
			#ifndef PRINT_RELEASE
				AT_PRINTF("Two consecutive ':' \r\n");
			#endif
			return AT_ERROR;
		}
 // Count number of correct ':'
		else if (param[i]==':')
		{
			j++;
		}
    }

    if (j != 2 || i > 30)
    {
		#ifndef PRINT_RELEASE
    		AT_PRINTF("Wrong number of ':' or size of string too big \r\n");
		#endif
        return AT_ERROR;
    }
// Parsing string
    j= 0;
    i= 0;

	while(param[i] != ':')
	{
		timeout_str[j]=param[i];
		i++;
		j++;
	}
	i++;
    j=0;
	while(param[i] != ':')
	{
		frequency_str[j]=param[i];
		i++;
		j++;
	}

	i++;
	j=0;
	while(param[i] != '\0')
	{
		power_str[j]=param[i];
		i++;
		j++;
	}

	if (j == 0)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Last argument after '+' is empty \r\n");
		#endif
		return AT_ERROR;
	}

	if (LmHandlerGetActiveRegion(&region) != LORAMAC_HANDLER_SUCCESS)
    {
		#ifndef PRINT_RELEASE
			AT_PRINTF("Error get region. Make sure you have joined\r\n");
		#endif
		return AT_ERROR;
    }

    j = 0;
    i = 0;
    timeout = atoi(timeout_str);
    if (timeout < 0 || timeout > 0x7FFFFFFF)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Wrong timeout setting \r\n");
		#endif
		return AT_ERROR;
	}

    /*if freq is set in MHz, convert to Hz*/
    if (150 < atoi(frequency_str) && atoi(frequency_str) < 960)
    {
    	/*given in MHz*/
    	frequency = (atoi(frequency_str)*1000000);
    }
    else
    {
    	if (150000000 < atoi(frequency_str) && atoi(frequency_str) < 960000000)
    	{
    		frequency = atoi(frequency_str);
    	}
    	else
    	{
			#ifndef PRINT_RELEASE
    			AT_PRINTF("Wrong frequency\r\n");
			#endif
    		return AT_ERROR;
    	}
    }

    switch(region)
	{
	  case LORAMAC_REGION_EU868:
			if (frequency < 0 || frequency < 863000000 || frequency > 870000000)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong frequency parameter \r\n");
				#endif
				return AT_ERROR;
			}
			break;

	  case LORAMAC_REGION_US915:
			if (frequency < 0 || frequency < 902000000 || frequency > 928000000)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong frequency parameter \r\n");
				#endif
				return AT_ERROR;
			}
			 break;
	  case LORAMAC_REGION_AU915:
			if (frequency < 0 || frequency < 915000000 || frequency > 928000000)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong frequency parameter \r\n");
				#endif
				return AT_ERROR;
			}
			 break;
	  default:
			if (frequency < 0 || frequency < 863000000 || frequency > 870000000)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong frequency parameter \r\n");
				#endif
				return AT_ERROR;
			}
			break;

	}

    power = atoi(power_str);

    switch(region)
	{
		case LORAMAC_REGION_EU868:
			if (power < 0 || power > 7)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong power level \r\n");
				#endif
				return AT_ERROR;
			}
			break;

		case LORAMAC_REGION_US915:
		    if (power < 0 || power > 14)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong power level \r\n");
				#endif
				return AT_ERROR;
			}
			break;

		case LORAMAC_REGION_AU915:
		    if (power < 0 || power > 14)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong power level \r\n");
				#endif
				return AT_ERROR;
			}
			break;

		default:
		    if (power < 0 || power > 7)
			{
				#ifndef PRINT_RELEASE
					AT_PRINTF("Wrong power level \r\n");
				#endif
				return AT_ERROR;
			}
			break;
	}

	MlmeReq_t mlmeReq;
	mlmeReq.Type = MLME_TXCW_1;
	mlmeReq.Req.TxCw.Timeout = timeout;
	mlmeReq.Req.TxCw.Frequency = frequency;
	mlmeReq.Req.TxCw.Power = power;
	LoRaMacStatus_t ret = LORAMAC_STATUS_OK;
	ret = LoRaMacMlmeRequest(&mlmeReq);
	if (ret != LORAMAC_STATUS_OK)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Wrong setting continuous wave mode . Make sure you have joined %d\r\n",ret);
		#endif
		return AT_ERROR;
	}

	#ifndef PRINT_RELEASE
		AT_PRINTF("Setting: Timeout = %i sec | Frequency = %i MHz | Power level = %i dBm\r\n",timeout,frequency,power);
	#endif

	return AT_OK;
	/* USER CODE BEGIN AT_bat_get_2 */

	/* USER CODE END AT_bat_get_2 */
}

ATEerror_t AT_test_get_config(const char *param)
{
	/* USER CODE BEGIN AT_test_get_config_1 */

	/* USER CODE END AT_test_get_config_1 */
	testParameter_t test_param;
	#ifndef PRINT_RELEASE
		uint32_t loraBW[7] = {7812, 15625, 31250, 62500, 125000, 250000, 500000};
	#endif

	TST_get_config(&test_param);
	#ifndef PRINT_RELEASE
		AT_PRINTF("1: Freq= %d Hz\r\n", test_param.freq);
		AT_PRINTF("2: Power= %d dBm\r\n", test_param.power);

		if (test_param.modulation == 0)
		{
			/*fsk*/
			AT_PRINTF("3: Bandwidth= %d Hz\r\n", test_param.bandwidth);
			AT_PRINTF("4: FSK datarate= %d bps\r\n", test_param.loraSf_datarate);
			AT_PRINTF("5: Coding Rate not applicable\r\n");
			AT_PRINTF("6: LNA State= %d  \r\n", test_param.lna);
			AT_PRINTF("7: PA Boost State= %d  \r\n", test_param.paBoost);
			AT_PRINTF("8: modulation FSK\r\n");
			AT_PRINTF("9: Payload len= %d Bytes\r\n", test_param.payloadLen);
			AT_PRINTF("10: LowDRopt not applicable\r\n");
		}
		else if (test_param.modulation == 1)
		{
			/*Lora*/
			AT_PRINTF("3: Bandwidth= %d (=%d Hz)\r\n", test_param.bandwidth, loraBW[test_param.bandwidth]);
			AT_PRINTF("4: SF= %d \r\n", test_param.loraSf_datarate);
			AT_PRINTF("5: CR= %d (=4/%d) \r\n", test_param.codingRate, test_param.codingRate + 4);
			AT_PRINTF("6: LNA State= %d  \r\n", test_param.lna);
			AT_PRINTF("7: PA Boost State= %d  \r\n", test_param.paBoost);
			AT_PRINTF("8: modulation LORA\r\n");
			AT_PRINTF("9: Payload len= %d Bytes\r\n", test_param.payloadLen);
			AT_PRINTF("10: LowDRopt[0 to 2]= %d \r\n", test_param.lowDrOpt);
		}
		else
		{
			AT_PRINTF("4: BPSK datarate= %d bps\r\n", test_param.loraSf_datarate);
		}

		AT_PRINTF("\r\ncan be copy/paste in set cmd: AT+TCONF=%d:%d:%d:%d:4/%d:%d:%d:%d:%d:%d\r\n", test_param.freq,
				test_param.power,
				test_param.bandwidth, test_param.loraSf_datarate, test_param.codingRate + 4, \
				test_param.lna, test_param.paBoost, test_param.modulation, test_param.payloadLen,
				test_param.lowDrOpt);
	#else
		AT_PRINTF("%d:%d:%d:%d:4/%d:%d:%d:%d:%d:%d:%d:%d\r\n", test_param.freq,
				test_param.power,
				test_param.bandwidth, test_param.loraSf_datarate, test_param.codingRate + 4, \
				test_param.lna, test_param.paBoost, test_param.modulation, test_param.payloadLen,
				test_param.lowDrOpt);
	#endif
	return AT_OK;
	/* USER CODE BEGIN AT_test_get_config_2 */

	/* USER CODE END AT_test_get_config_2 */
}

ATEerror_t AT_test_set_config(const char *param) //modifica
{
	/* USER CODE BEGIN AT_test_set_config_1 */

	/* USER CODE END AT_test_set_config_1 */
	testParameter_t test_param = {0};
	uint32_t freq;
	int32_t power;
	uint32_t bandwidth;
	uint32_t loraSf_datarate;
	uint32_t codingRate;
	uint32_t lna;
	uint32_t paBoost;
	uint32_t modulation;
	uint32_t payloadLen;
	uint32_t fskDeviation;
	uint32_t lowDrOpt;
	uint32_t BTproduct;
	uint32_t crNum;

	if (11 == tiny_sscanf(param, "%d:%d:%d:%d:%d/%d:%d:%d:%d:%d:%d",
						&freq,
						&power,
						&bandwidth,
						&loraSf_datarate,
						&crNum,
						&codingRate,
						&lna,
						&paBoost,
						&modulation,
						&payloadLen,
						&lowDrOpt))
	{
	/*extend to new format for extended*/
	}
	else
	{
		return AT_ERROR;
	}

	fskDeviation = 600;
	test_param.fskDev = fskDeviation;
	BTproduct = 0;
	test_param.BTproduct = BTproduct;

	/*get current config*/
	TST_get_config(&test_param);

	/* 8: modulation check and set */
	/* first check because required for others */
	if (modulation == 0)
	{
		test_param.modulation = TEST_FSK;
	}
	else if (modulation == 1)
	{
		test_param.modulation = TEST_LORA;
	}
	else if (modulation == 2)
	{
		test_param.modulation = TEST_BPSK;
	}
	else
	{
		return AT_ERROR;
	}

	/* 1: frequency check and set */
	if (150 < freq && freq < 960)
	{
		/*given in MHz*/
		test_param.freq = freq * 1000000;
	}
	else
	{
	if (150000000 < freq && freq < 960000000)
	{
		test_param.freq = freq;
	}
	else
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("wrong frequency\r\n");
		#endif
		return AT_ERROR;
	}
	}

	/* 2: power check and set */
//    switch(region)
//	{
//
//		case LORAMAC_REGION_EU868:
//			if (power < 0 || power > 7)
//			{
//				#ifndef PRINT_RELEASE
//					AT_PRINTF("Wrong power level \r\n");
//				#endif
//				return AT_ERROR;
//			}
//			else
//			{
//				test_param.power = power;
//			}
//			break;
//
//		case LORAMAC_REGION_US915:
//		    if (power < 0 || power > 14)
//			{
//				#ifndef PRINT_RELEASE
//					AT_PRINTF("Wrong power level \r\n");
//				#endif
//				return AT_ERROR;
//			}
//		    else
//		    {
//		    	test_param.power = power;
//		    }
//			break;
//
//		case LORAMAC_REGION_AU915:
//		    if (power < 0 || power > 14)
//			{
//				#ifndef PRINT_RELEASE
//					AT_PRINTF("Wrong power level \r\n");
//				#endif
//				return AT_ERROR;
//			}
//		    else
//		    {
//		    	test_param.power = power;
//		    }
//			break;
//
//		default:
//		    if (power < 0 || power > 7)
//			{
//				#ifndef PRINT_RELEASE
//					AT_PRINTF("Wrong power level \r\n");
//				#endif
//				return AT_ERROR;
//			}
//		    else
//		    {
//		    	test_param.power = power;
//		    }
//			break;
//	}

	if ((power >= -9) && (power <= 22))
	{
		test_param.power = power;
	}
	else
	{
		return AT_ERROR;
	}

	/* 3: bandwidth check and set */
	if ((test_param.modulation == TEST_FSK) && (bandwidth >= 4800) && (bandwidth <= 467000))
	{
		test_param.bandwidth = bandwidth;
	}
	else if ((test_param.modulation == TEST_LORA) && (bandwidth <= BW_500kHz))
	{
		test_param.bandwidth = bandwidth;
	}
	else if (test_param.modulation == TEST_BPSK)
	{
	/* Not used */
	}
	else
	{
		return AT_ERROR;
	}

	/* 4: datarate/spreading factor check and set */
	if ((test_param.modulation == TEST_FSK) && (loraSf_datarate >= 600) && (loraSf_datarate <= 300000))
	{
		test_param.loraSf_datarate = loraSf_datarate;
	}
	else if ((test_param.modulation == TEST_LORA) && (loraSf_datarate >= 5) && (loraSf_datarate <= 12))
	{
		test_param.loraSf_datarate = loraSf_datarate;
	}
	else if ((test_param.modulation == TEST_BPSK) && (loraSf_datarate <= 1000))
	{
		test_param.loraSf_datarate = loraSf_datarate;
	}
	else
	{
		return AT_ERROR;
	}

	/* 5: coding rate check and set */
	if ((test_param.modulation == TEST_FSK) || (test_param.modulation == TEST_BPSK))
	{
	/* Not used */
	}
	else if ((test_param.modulation == TEST_LORA) && ((codingRate >= 5) && (codingRate <= 8)))
	{
		test_param.codingRate = codingRate - 4;
	}
	else
	{
		return AT_ERROR;
	}

	/* 6: lna state check and set */
	if (lna <= 1)
	{
		test_param.lna = lna;
	}
	else
	{
		return AT_ERROR;
	}

	/* 7: pa boost check and set */
	if (paBoost <= 1)
	{
	/* Not used */
		test_param.paBoost = paBoost;
	}

	/* 9: payloadLen check and set */
	if ((payloadLen != 0) && (payloadLen < 256))
	{
		test_param.payloadLen = payloadLen;
	}
	else
	{
		return AT_ERROR;
	}

	/* 10: low datarate optimization check and set */
	if ((test_param.modulation == TEST_FSK) || (test_param.modulation == TEST_BPSK))
	{
	/* Not used */
	}
	else if ((test_param.modulation == TEST_LORA) && (lowDrOpt <= 2))
	{
		test_param.lowDrOpt = lowDrOpt;
	}
	else
	{
		return AT_ERROR;
	}

	TST_set_config(&test_param);

	return AT_OK;
	/* USER CODE BEGIN AT_test_set_config_2 */

	/* USER CODE END AT_test_set_config_2 */
}

ATEerror_t AT_test_tx(const char *param)
{
	/* USER CODE BEGIN AT_test_tx_1 */

	/* USER CODE END AT_test_tx_1 */
	const char *buf = param;
	uint32_t nb_packet;
	uint32_t freq_send;
	uint32_t loraSf;
	uint32_t bandwidth,bandwidth_rx;

	testParameter_t test_param;

	if (4 != tiny_sscanf(buf, "%u:%u:%u:%u", &nb_packet,&freq_send,&loraSf,&bandwidth_rx))
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("AT+TTX: Some parameter missing\r\n");
		#endif
		return AT_ERROR;
	}

	/*get current config*/
	TST_get_config(&test_param);
	/*if freq is set in MHz, convert to Hz*/
	if (150 < freq_send && freq_send < 960)
	{
	/*given in MHz*/
	test_param.freq = freq_send * 1000000;
	}
	else
	{
	if (150000000 < freq_send && freq_send < 960000000)
	{
		test_param.freq = freq_send;
	}
	else
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("wrong frequency\r\n");
		#endif
		return AT_ERROR;
	}
	}

	if (nb_packet == 0)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Error number of packet\r\n");
		#endif
		return AT_ERROR;
	}

	if (bandwidth_rx == 500)
	{
		bandwidth = BW_500kHz;
	}
	else if (bandwidth_rx == 250)
	{
		bandwidth = BW_250kHz;
	}
	else if (bandwidth_rx == 125)
	{
		bandwidth = BW_125kHz;
	}
	else
	{
		return AT_ERROR;
	}

	if ((loraSf >= 5) && (loraSf <= 12))
	{
		test_param.loraSf_datarate = loraSf;
	}
	else
	{
		return AT_ERROR;
	}

	test_param.bandwidth = bandwidth;
	/*increment frequency*/
	 /*Set new config*/
	TST_set_config(&test_param);
	#ifndef PRINT_RELEASE
		APP_TPRINTF("Tx at -- %dMz -- of %d packets\r\n", test_param.freq,nb_packet);
	#endif

	if (0U == TST_TX_Start(nb_packet))
	{
		return AT_OK;
	}
	else
	{
		#ifdef PRINT_RELEASE
			AT_PRINTF("b1\r\n");
		#endif
		return AT_BUSY_ERROR;
	}
	/* USER CODE BEGIN AT_test_tx_2 */

	/* USER CODE END AT_test_tx_2 */
}

ATEerror_t AT_test_rx(const char *param)
{
	/* USER CODE BEGIN AT_test_rx_1 */

	/* USER CODE END AT_test_rx_1 */
	const char *buf = param;
	uint32_t nb_packet;
	uint32_t freq_send;
	uint32_t loraSf;
	uint32_t bandwidth,bandwidth_rx;

	testParameter_t test_param;

	if (4 != tiny_sscanf(buf, "%u:%u:%u:%u", &nb_packet,&freq_send,&loraSf,&bandwidth_rx))
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("AT+TRX: Some parameter missing\r\n");
		#endif
		return AT_ERROR;
	}

	/*get current config*/
	TST_get_config(&test_param);
	/*if freq is set in MHz, convert to Hz*/
	if (150 < freq_send && freq_send < 960)
	{
		/*given in MHz*/
		test_param.freq = freq_send * 1000000;
	}
	else
	{
	if (150000000 < freq_send && freq_send < 960000000)
	{
		test_param.freq = freq_send;
	}
	else
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("wrong frequency\r\n");
		#endif
		return AT_ERROR;
	}
	}

	if (nb_packet == 0)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("Error number of packet\r\n");
		#endif
		return AT_ERROR;
	}

	if (bandwidth_rx == 500)
	{
		bandwidth = BW_500kHz;
	}
	else if (bandwidth_rx == 250)
	{
		bandwidth = BW_250kHz;
	}
	else if (bandwidth_rx == 125)
	{
		bandwidth = BW_125kHz;
	}
	else
	{
		return AT_ERROR;
	}

	if ((loraSf >= 5) && (loraSf <= 12))
	{
		test_param.loraSf_datarate = loraSf;
	}
	else
	{
		return AT_ERROR;
	}
	test_param.bandwidth = bandwidth;
	/*increment frequency*/

	//  /*Set new config*/
	TST_set_config(&test_param);

	#ifndef PRINT_RELEASE
		APP_TPRINTF("Rx at -- %dMz  -- of %d packets\r\n", test_param.freq,nb_packet);
	#endif

	if (0U == TST_RX_Start(nb_packet))
	{
		return AT_OK;
	}
	else
	{
		#ifdef PRINT_RELEASE
			AT_PRINTF("b1\r\n");
		#endif
		return AT_BUSY_ERROR;
	}
	/* USER CODE BEGIN AT_test_rx_2 */

	/* USER CODE END AT_test_rx_2 */
}

ATEerror_t AT_Certif(const char *param)
{
	/* USER CODE BEGIN AT_Certif_1 */

	/* USER CODE END AT_Certif_1 */
	switch (param[0])
	{
	case '0':
		LmHandlerJoin(ACTIVATION_TYPE_ABP);
	case '1':
		LmHandlerJoin(ACTIVATION_TYPE_OTAA);
		break;
	default:
		return AT_ERROR;
	}

	UTIL_TIMER_Create(&TxCertifTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnCertifTimer, NULL);  /* 8s */
	UTIL_TIMER_SetPeriod(&TxCertifTimer, 8000);  /* 8s */
	UTIL_TIMER_Start(&TxCertifTimer);
	UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaCertifTx), UTIL_SEQ_RFU, CertifSend);

	return AT_OK;
	/* USER CODE BEGIN AT_Certif_2 */

	/* USER CODE END AT_Certif_2 */
}

ATEerror_t AT_test_tx_hopping(const char *param)
{
	/* USER CODE BEGIN AT_test_tx_hopping_1 */

	/* USER CODE END AT_test_tx_hopping_1 */
	const char *buf = param;
	uint32_t freq_start;
	uint32_t freq_stop;
	uint32_t delta_f;
	uint32_t nb_tx;

	testParameter_t test_param;
	uint32_t hop_freq;

	if (4 != tiny_sscanf(buf, "%u:%u:%u:%u", &freq_start, &freq_stop, &delta_f, &nb_tx))
	{
		return AT_ERROR;
	}

	/*if freq_start is set in MHz, convert to Hz*/
	if (150 < freq_start && freq_start < 960)
	{
		/*given in MHz*/
		freq_start *= 1000000;
	}
	else
	{
		if (!(150000000 < freq_start && freq_start < 960000000))
		{
			#ifndef PRINT_RELEASE
				AT_PRINTF("wrong frequency\r\n");
			#endif
			return AT_ERROR;
		}
	}

	/*if freq_stop is set in MHz, convert to Hz*/
	if (150 < freq_stop && freq_stop < 960)
	{
		/*given in MHz*/
		freq_stop *= 1000000;
	}
	else
	{
		if (!(150000000 < freq_stop && freq_stop < 960000000))
		{
			#ifndef PRINT_RELEASE
				AT_PRINTF("wrong frequency\r\n");
			#endif
			return AT_ERROR;
		}
	}

	hop_freq = freq_start;

	for (int i = 0; i < nb_tx; i++)
	{
		/*get current config*/
		TST_get_config(&test_param);

		/*increment frequency*/
		test_param.freq = hop_freq;
		/*Set new config*/
		TST_set_config(&test_param);

		#ifndef PRINT_RELEASE
			APP_TPRINTF("Tx Hop at %dHz. %d of %d\r\n", hop_freq, i, nb_tx);
		#endif

		if (0U != TST_TX_Start(1))
		{
			return AT_BUSY_ERROR;
		}

		hop_freq += delta_f;

		if (hop_freq > freq_stop)
		{
			hop_freq = freq_start;
		}
	}

	return AT_OK;
	/* USER CODE BEGIN AT_test_tx_hopping_2 */

	/* USER CODE END AT_test_tx_hopping_2 */
}

ATEerror_t AT_write_register(const char *param)
{
	/* USER CODE BEGIN AT_write_register_1 */

	/* USER CODE END AT_write_register_1 */
	uint8_t add[2];
	uint16_t add16;
	uint8_t data;

	if (strlen(param) != 7)
	{
		return AT_ERROR;
	}

	if (stringToData(param, add, 2) != 0)
	{
		return AT_ERROR;
	}
	param += 5;
	if (stringToData(param, &data, 1) != 0)
	{
		return AT_ERROR;
	}
	add16 = (((uint16_t)add[0]) << 8) + (uint16_t)add[1];
	Radio.Write(add16, data);

	return AT_OK;
	/* USER CODE BEGIN AT_write_register_2 */

	/* USER CODE END AT_write_register_2 */
}

ATEerror_t AT_read_register(const char *param)
{
	/* USER CODE BEGIN AT_read_register_1 */

	/* USER CODE END AT_read_register_1 */
	uint8_t add[2];
	uint16_t add16;
	uint8_t data;


	if (strlen(param) != 4)
	{
		return AT_ERROR;
	}

	if (stringToData(param, add, 2) != 0)
	{
		return AT_ERROR;
	}

	add16 = (((uint16_t)add[0]) << 8) + (uint16_t)add[1];
	data = Radio.Read(add16);

		AT_PRINTF("REG 0x%04X=0x%02X", add16, data);

	return AT_OK;
	/* USER CODE BEGIN AT_read_register_2 */

	/* USER CODE END AT_read_register_2 */
}

/* --------------- Information command --------------- */
ATEerror_t AT_bat_get(const char *param)
{
	/* USER CODE BEGIN AT_bat_get_1 */

	/* USER CODE END AT_bat_get_1 */
	print_d(SYS_GetBatteryLevel());

	return AT_OK;
	/* USER CODE BEGIN AT_bat_get_2 */

	/* USER CODE END AT_bat_get_2 */
}


/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

static uint8_t format_check(const char *param, uint8_t param_length)
{
  uint8_t choose = 0;
  for (uint8_t i=0;i<param_length;i++)
  {

	  if((i+1)%3 == 0)
	  {
		  choose = 1;
	  }
	  else
	  {
		  choose = 2;
	  }

	  switch (choose)
	  {
	  case 1:
		  if(!(param[i]==58))
		  {
			  return 0;
		  }
		  break;
	  case 2:
		  if(!(((param[i])>=48 && (param[i]) <= 57) || ((param[i])>=65 && param[i] <= 70) || (param[i]>=97 && param[i] <= 102)))
		  {
			  return 0;
		  }
		  break;

	  default:
		  break;
	  }

  }

  return 1;
}

static int32_t sscanf_uint32_as_hhx(const char *from, uint32_t *value)
{
  /* USER CODE BEGIN sscanf_uint32_as_hhx_1 */

  /* USER CODE END sscanf_uint32_as_hhx_1 */
  return tiny_sscanf(from, "%hhx:%hhx:%hhx:%hhx",
                     &((unsigned char *)(value))[3],
                     &((unsigned char *)(value))[2],
                     &((unsigned char *)(value))[1],
                     &((unsigned char *)(value))[0]);
  /* USER CODE BEGIN sscanf_uint32_as_hhx_2 */

  /* USER CODE END sscanf_uint32_as_hhx_2 */
}

static int sscanf_16_hhx(const char *from, uint8_t *pt)
{
  /* USER CODE BEGIN sscanf_16_hhx_1 */

  /* USER CODE END sscanf_16_hhx_1 */
  return tiny_sscanf(from, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                     &pt[0], &pt[1], &pt[2], &pt[3], &pt[4], &pt[5], &pt[6],
                     &pt[7], &pt[8], &pt[9], &pt[10], &pt[11], &pt[12], &pt[13],
                     &pt[14], &pt[15]);
  /* USER CODE BEGIN sscanf_16_hhx_2 */

  /* USER CODE END sscanf_16_hhx_2 */
}

static void print_uint32_as_02x(uint32_t value)
{
  /* USER CODE BEGIN print_uint32_as_02x_1 */

  /* USER CODE END print_uint32_as_02x_1 */
  AT_PRINTF("%02X:%02X:%02X:%02X\r\n",
            (unsigned)((unsigned char *)(&value))[3],
            (unsigned)((unsigned char *)(&value))[2],
            (unsigned)((unsigned char *)(&value))[1],
            (unsigned)((unsigned char *)(&value))[0]);
  /* USER CODE BEGIN print_uint32_as_02x_2 */

  /* USER CODE END print_uint32_as_02x_2 */
}

static void print_16_02x(uint8_t *pt)
{
  /* USER CODE BEGIN print_16_02x_1 */

  /* USER CODE END print_16_02x_1 */
  AT_PRINTF("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
            pt[0], pt[1], pt[2], pt[3],
            pt[4], pt[5], pt[6], pt[7],
            pt[8], pt[9], pt[10], pt[11],
            pt[12], pt[13], pt[14], pt[15]);
  /* USER CODE BEGIN print_16_02x_2 */

  /* USER CODE END print_16_02x_2 */
}

static void print_8_02x(uint8_t *pt)
{
  /* USER CODE BEGIN print_8_02x_1 */

  /* USER CODE END print_8_02x_1 */
  AT_PRINTF("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
            pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], pt[6], pt[7]);
  /* USER CODE BEGIN print_8_02x_2 */

  /* USER CODE END print_8_02x_2 */
}

static void print_d(int32_t value)
{
  /* USER CODE BEGIN print_d_1 */

  /* USER CODE END print_d_1 */
  AT_PRINTF("%d\r\n", value);
  /* USER CODE BEGIN print_d_2 */

  /* USER CODE END print_d_2 */
}

static void print_u(uint32_t value)
{
  /* USER CODE BEGIN print_u_1 */

  /* USER CODE END print_u_1 */
  AT_PRINTF("%u\r\n", value);
  /* USER CODE BEGIN print_u_2 */

  /* USER CODE END print_u_2 */
}

static void OnCertifTimer(void *context)
{
  /* USER CODE BEGIN OnCertifTimer_1 */

  /* USER CODE END OnCertifTimer_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaCertifTx), CFG_SEQ_Prio_0);
  /* USER CODE BEGIN OnCertifTimer_2 */

  /* USER CODE END OnCertifTimer_2 */
}

static void CertifSend(void)
{
  /* USER CODE BEGIN CertifSend_1 */

  /* USER CODE END CertifSend_1 */
  AppData.Buffer[0] = 0x43;
  AppData.BufferSize = 1;
  AppData.Port = 99;

  /* Restart Tx to prevent a previous Join Failed */
  if (LmHandlerJoinStatus() != LORAMAC_HANDLER_SET)
  {
    UTIL_TIMER_Start(&TxCertifTimer);
  }
  LmHandlerSend(&AppData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, false);
}

static uint8_t Char2Nibble(char Char)
{
  if (((Char >= '0') && (Char <= '9')))
  {
    return Char - '0';
  }
  else if (((Char >= 'a') && (Char <= 'f')))
  {
    return Char - 'a' + 10;
  }
  else if ((Char >= 'A') && (Char <= 'F'))
  {
    return Char - 'A' + 10;
  }
  else
  {
    return 0xF0;
  }
  /* USER CODE BEGIN CertifSend_2 */

  /* USER CODE END CertifSend_2 */
}

static int32_t stringToData(const char *str, uint8_t *data, uint32_t Size)
{
  /* USER CODE BEGIN stringToData_1 */

  /* USER CODE END stringToData_1 */
  char hex[3];
  hex[2] = 0;
  int32_t ii = 0;
  while (Size-- > 0)
  {
    hex[0] = *str++;
    hex[1] = *str++;

    /*check if input is hex */
    if ((isHex(hex[0]) == -1) || (isHex(hex[1]) == -1))
    {
      return -1;
    }
    /*check if input is even nb of character*/
    if ((hex[1] == '\0') || (hex[1] == ','))
    {
      return -1;
    }
    data[ii] = (Char2Nibble(hex[0]) << 4) + Char2Nibble(hex[1]);

    ii++;
  }

  return 0;
  /* USER CODE BEGIN stringToData_2 */

  /* USER CODE END stringToData_2 */
}

static int32_t isHex(char Char)
{
  /* USER CODE BEGIN isHex_1 */

  /* USER CODE END isHex_1 */
  if (((Char >= '0') && (Char <= '9')) ||
      ((Char >= 'a') && (Char <= 'f')) ||
      ((Char >= 'A') && (Char <= 'F')))
  {
    return 0;
  }
  else
  {
    return -1;
  }
  /* USER CODE BEGIN isHex_2 */

  /* USER CODE END isHex_2 */
}

/* USER CODE BEGIN PrFD */
void Tx_Data_Cmd(void *context)
{
	UTIL_TIMER_Time_t nextTxIn = 0;
	LmHandlerErrorStatus_t lmhStatus;

	lmhStatus = LmHandlerSend(&AppData, isTxConfirmed, &nextTxIn, false);
	if ( lmhStatus == LORAMAC_HANDLER_SUCCESS )
	{
		number_send_ok++;
		#ifndef PRINT_RELEASE
			AT_PRINTF("SEND REQUEST: %i/%i\r\n",number_send_ok,number_of_send);
		#else
			AT_PRINTF("ns=%d\r\n",number_send_ok);
		#endif
		total_number--;
		Send_test = true;
	}
	else if (nextTxIn > 0)
	{
		#ifndef PRINT_RELEASE
			AT_PRINTF("AT_DUTYCYCLE_RESTRICTED %d, RETRY in %d msec\r\n",lmhStatus, (nextTxIn+2000));
		#else
			AT_PRINTF("d1\r\n");
		#endif
		UTIL_TIMER_SetPeriod(&Tx_Data, nextTxIn+2000);
		UTIL_TIMER_Start(&Tx_Data);
	}
	else
	{
		AT_PRINTF("r1\r\n");
		UTIL_TIMER_SetPeriod(&Tx_Data, send_timer);
		UTIL_TIMER_Start(&Tx_Data);
	}

}
/* USER CODE END PrFD */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
