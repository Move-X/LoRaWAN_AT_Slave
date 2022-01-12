/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lora_command.c
  * @author  MCD Application Team
  * @brief   Main command driver dedicated to command AT
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
#include <string.h>
#include "platform.h"
#include "lora_at.h"
#include "lora_command.h"
#include "lora_app.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* comment the following to have help message */
/* #define NO_HELP */
/* #define AT_RADIO_ACCESS */

/* Private typedef -----------------------------------------------------------*/
/**
  * @brief  Structure defining an AT Command
  */
struct ATCommand_s
{
  const char *string;                       /*< command string, after the "AT" */
  const int32_t size_string;                /*< size of the command string, not including the final \0 */
  ATEerror_t (*get)(const char *param);     /*< =? after the string to get the current value*/
  ATEerror_t (*set)(const char *param);     /*< = (but not =?\0) after the string to set a value */
  ATEerror_t (*run)(const char *param);     /*< \0 after the string - run the command */
#if !defined(NO_HELP)
  const char *help_string;                  /*< to be printed when ? after the string */
#endif /* !NO_HELP */
};

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
#define CMD_SIZE                        540
#define CIRC_BUFF_SIZE                  8
#define HELP_DISPLAY_FLUSH_DELAY        100

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/**
  * @brief  Array corresponding to the description of each possible AT Error
  */
#ifndef PRINT_RELEASE
	static const char *const ATError_description[] =
	{
	  "AT_OK\r\n",                     /* AT_OK */
	  "AT_ERROR\r\n",               /* AT_ERROR */
	  "AT_PARAM_ERROR\r\n",         /* AT_PARAM_ERROR */
	  "AT_BUSY_ERROR\r\n",          /* AT_BUSY_ERROR */
	  "AT_TEST_PARAM_OVERFLOW\r\n", /* AT_TEST_PARAM_OVERFLOW */
	  "AT_NO_NETWORK_JOINED\r\n",   /* AT_NO_NET_JOINED */
	  "AT_RX_ERROR\r\n",            /* AT_RX_ERROR */
	  "AT_NO_CLASS_B_ENABLE\r\n",   /* AT_NO_CLASS_B_ENABLE */
	  "AT_DUTYCYCLE_RESTRICTED\r\n", /* AT_DUTYCYCLE_RESTRICTED */
	  "AT_CRYPTO_ERROR\r\n",        /* AT_CRYPTO_ERROR */
	  "error unknown\r\n",          /* AT_MAX */
	};
#endif

/**
  * @brief  Array of all supported AT Commands
  */
static const struct ATCommand_s ATCommand[] =
{
  /* General commands */
  {
    .string = AT_RESET,
    .size_string = sizeof(AT_RESET) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RESET"<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_return_error,
    .run = AT_reset,
  },

  {
    .string = AT_VL,
    .size_string = sizeof(AT_VL) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_VL"=<Level>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_verbose_get,
    .set = AT_verbose_set,
    .run = AT_return_error,
  },

  {
    .string = AT_LTIME,
    .size_string = sizeof(AT_LTIME) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_LTIME"=?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_LocalTime_get,
    .set = AT_return_error,
    .run = AT_return_error,
  },

  {
      .string = AT_TREQ,
      .size_string = sizeof(AT_TREQ) - 1,
  #ifndef NO_HELP
      .help_string = "AT"AT_TREQ"<CR>\r\n",
  #endif /* !NO_HELP */
      .get = AT_return_error,
      .set = AT_return_error,
      .run = AT_TimeRequest_run,
    },

  /* Keys, IDs and EUIs management commands */
  {
    .string = AT_JOINEUI,
    .size_string = sizeof(AT_JOINEUI) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_JOINEUI"=<XX:XX:XX:XX:XX:XX:XX:XX>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_JoinEUI_get,
    .set = AT_JoinEUI_set,
    .run = AT_return_error,
  },

  {
    .string = AT_NWKKEY,
    .size_string = sizeof(AT_NWKKEY) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_NWKKEY"=<XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_NwkKey_get,
    .set = AT_NwkKey_set,
    .run = AT_return_error,
  },

  {
    .string = AT_APPKEY,
    .size_string = sizeof(AT_APPKEY) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_APPKEY"=<XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_AppKey_get,
    .set = AT_AppKey_set,
    .run = AT_return_error,
  },

  {
    .string = AT_NWKSKEY,
    .size_string = sizeof(AT_NWKSKEY) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_NWKSKEY"=<XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_NwkSKey_get,
    .set = AT_NwkSKey_set,
    .run = AT_return_error,
  },

  {
    .string = AT_APPSKEY,
    .size_string = sizeof(AT_APPSKEY) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_APPSKEY"=<XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_AppSKey_get,
    .set = AT_AppSKey_set,
    .run = AT_return_error,
  },

  {
    .string = AT_DADDR,
    .size_string = sizeof(AT_DADDR) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DADDR"=<XX:XX:XX:XX>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_DevAddr_get,
    .set = AT_DevAddr_set,
    .run = AT_return_error,
  },

  {
    .string = AT_DEUI,
    .size_string = sizeof(AT_DEUI) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DEUI"=<XX:XX:XX:XX:XX:XX:XX:XX>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_DevEUI_get,
    .set = AT_DevEUI_set,
    .run = AT_return_error,
  },

  {
    .string = AT_NWKID,
    .size_string = sizeof(AT_NWKID) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_NWKID"=<NwkID>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_NetworkID_get,
    .set = AT_NetworkID_set,
    .run = AT_return_error,
  },

  /* LoRaWAN join and send data commands */
  {
    .string = AT_JOIN,
    .size_string = sizeof(AT_JOIN) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_JOIN"=<Mode><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_Join,
    .run = AT_return_error,
  },

  {
    .string = AT_LINKC,
    .size_string = sizeof(AT_LINKC) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_LINKC"\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_return_error,
    .run = AT_Link_Check,
  },

  {
    .string = AT_CHANNEL,
    .size_string = sizeof(AT_CHANNEL) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_CHANNEL"=<Mask1>:<Mask2>:<Mask3>:<Mask4>:<Mask5>:<Mask6>:<Mask7>:"
    		"<Mask8>:<Mask9>:<Mask10>:<Mask11>:<Mask12>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_Get_Channel,
    .set = AT_Add_Remove_Channel,
    .run = AT_return_error,
  },

  {
    .string = AT_SEND,
    .size_string = sizeof(AT_SEND) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_SEND"=<Payload_length>:<Payload><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_Send,
    .run = AT_return_error,
  },

  /* LoRaWAN network management commands */
  {
    .string = AT_VER,
    .size_string = sizeof(AT_VER) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_VER"=?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_version_get,
    .set = AT_return_error,
    .run = AT_return_error,
  },

  {
    .string = AT_ADR,
    .size_string = sizeof(AT_ADR) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_ADR"=<ADR>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_ADR_get,
    .set = AT_ADR_set,
    .run = AT_return_error,
  },

  {
    .string = AT_DR,
    .size_string = sizeof(AT_DR) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DR"=<DataRate>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_DataRate_get,
    .set = AT_DataRate_set,
    .run = AT_return_error,
  },

  {
    .string = AT_BAND,
    .size_string = sizeof(AT_BAND) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_BAND"=<BandID>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_Region_get,
    .set = AT_Region_set,
    .run = AT_return_error,
  },

  {
    .string = AT_CLASS,
    .size_string = sizeof(AT_CLASS) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_CLASS"=<Class>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_DeviceClass_get,
    .set = AT_DeviceClass_set,
    .run = AT_return_error,
  },

  {
    .string = AT_DCS,
    .size_string = sizeof(AT_DCS) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DCS"=<DutyCycle>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_DutyCycle_get,
    .set = AT_DutyCycle_set,
    .run = AT_return_error,
  },

  {
    .string = AT_JN1DL,
    .size_string = sizeof(AT_JN1DL) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_JN1DL"=<Delay>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_JoinAcceptDelay1_get,
    .set = AT_JoinAcceptDelay1_set,
    .run = AT_return_error,
  },

  {
    .string = AT_JN2DL,
    .size_string = sizeof(AT_JN2DL) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_JN2DL"=<Delay>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_JoinAcceptDelay2_get,
    .set = AT_JoinAcceptDelay2_set,
    .run = AT_return_error,
  },

  {
    .string = AT_RX1DL,
    .size_string = sizeof(AT_RX1DL) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RX1DL"=<Delay>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_Rx1Delay_get,
    .set = AT_Rx1Delay_set,
    .run = AT_return_error,
  },

  {
    .string = AT_RX2DL,
    .size_string = sizeof(AT_RX2DL) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RX2DL"=<Delay>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_Rx2Delay_get,
    .set = AT_Rx2Delay_set,
    .run = AT_return_error,
  },

  {
    .string = AT_RX2DR,
    .size_string = sizeof(AT_RX2DR) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RX2DR"=<DataRate>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_Rx2DataRate_get,
    .set = AT_Rx2DataRate_set,
    .run = AT_return_error,
  },

  {
    .string = AT_RX2FQ,
    .size_string = sizeof(AT_RX2FQ) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RX2FQ"=<Freq>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_Rx2Frequency_get,
    .set = AT_Rx2Frequency_set,
    .run = AT_return_error,
  },

  {
    .string = AT_TXP,
    .size_string = sizeof(AT_TXP) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TXP"=<Power>,?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_TransmitPower_get,
    .set = AT_TransmitPower_set,
    .run = AT_return_error,
  },

//  {
//    .string = AT_PGSLOT,
//    .size_string = sizeof(AT_PGSLOT) - 1,
//#ifndef NO_HELP
//    .help_string = "AT"AT_PGSLOT"=<Period>,?<CR>\r\n",
//#endif /* !NO_HELP */
//    .get = AT_PingSlot_get,
//    .set = AT_PingSlot_set,
//    .run = AT_return_error,
//  },

  /* Radio tests commands */

#ifdef FSK_COMMAND
  {
    .string = AT_TTONE,
    .size_string = sizeof(AT_TTONE) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TTONE"<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_return_error,
    .run = AT_test_txTone,
  },

  {
    .string = AT_TRSSI,
    .size_string = sizeof(AT_TRSSI) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TRSSI"<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_return_error,
    .run = AT_test_rxRssi,
  },
#endif

  {
    .string = AT_TCONF,
    .size_string = sizeof(AT_TCONF) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TCONF"=<Freq_Hz_MHz>:<Power_dBm>:<Lora_Bandwidth>:<SF>:<Coding_Rate>:"
    		"<Lna>:<PA_Boost>:<Modulation>:<Payload_Length>:<LowDrOpt>:<BTproduct><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_test_get_config,
    .set = AT_test_set_config,
    .run = AT_return_error,
  },

  {
    .string = AT_TTX,
    .size_string = sizeof(AT_TTX) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TTX"=<PacketNb><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_test_tx,
    .run = AT_return_error,
  },

  {
    .string = AT_TRX,
    .size_string = sizeof(AT_TRX) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TRX"=<PacketNb><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_test_rx,
    .run = AT_return_error,
  },

  {
    .string = AT_CERTIF,
    .size_string = sizeof(AT_CERTIF) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_CERTIF"=<Mode><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_Certif,
    .run = AT_return_error,
  },

  {
    .string = AT_TTH,
    .size_string = sizeof(AT_TTH) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TTH"=<Fstart_Hz_MHz>:<Fstop_Hz_MHz>:<Fdelta_Hz_MHz>:<PacketNb><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_test_tx_hopping,
    .run = AT_return_error,
  },

#ifdef AT_RADIO_ACCESS
  {
    .string = AT_REGW,
    .size_string = sizeof(AT_REGW) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_REGW"=<Addr>:<Data><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_write_register,
    .run = AT_return_error,
  },

  {
    .string = AT_REGR,
    .size_string = sizeof(AT_REGR) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_REGR"=<Addr><CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_return_error,
    .set = AT_read_register,
    .run = AT_return_error,
  },
#endif /*AT_RADIO_ACCESS*/

  /* Information command */
  {
    .string = AT_BAT,
    .size_string = sizeof(AT_BAT) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_BAT"=?<CR>\r\n",
#endif /* !NO_HELP */
    .get = AT_bat_get,
    .set = AT_return_error,
    .run = AT_return_error,
  },
  /* USER CODE BEGIN ATCommand */
  {
	.string = AT_UID,
	.size_string = sizeof(AT_UID) - 1,
#ifndef NO_HELP
	.help_string = "AT"AT_UID"=?<CR>\r\n",
#endif /* !NO_HELP */
	.get = AT_UID_get,
	.set = AT_return_error,
	.run = AT_return_error,
  },

  {
	.string = AT_CW,
	.size_string = sizeof(AT_CW) - 1,
#ifndef NO_HELP
	.help_string = "AT"AT_CW"=<Timeout_Value>:<Frequency_Setting>:<Tx_Power_Level><CR>\r\n",
#endif /* !NO_HELP */
	.get = AT_return_error,
	.set = AT_Cw_set,
	.run = AT_return_error,
  },

  {
	.string = AT_STOPCW,
	.size_string = sizeof(AT_STOPCW) - 1,
#ifndef NO_HELP
	.help_string = "AT"AT_STOPCW"<CR>\r\n",
#endif /* !NO_HELP */
	.get = AT_return_error,
	.set = AT_return_error,
	.run = AT_Cw_Stop,
  },


  {
	.string = AT_CFGSEND,
	.size_string = sizeof(AT_CFGSEND) - 1,
#ifndef NO_HELP
	.help_string = "AT"AT_CFGSEND"=<Port>:<Number_Of_Send>:<ACK>:<Send_Timer>:<Data_Rate>:<Tx_Power_Level><CR>\r\n",
#endif /* !NO_HELP */
	.get = AT_ConfigSend_get,
	.set = AT_ConfigSend_set,
	.run = AT_return_error,
  },

  {
	.string = AT_STOPSEND,
	.size_string = sizeof(AT_STOPSEND) - 1,
#ifndef NO_HELP
	.help_string = "AT"AT_STOPSEND"<CR>\r\n",
#endif /* !NO_HELP */
	.get = AT_return_error,
	.set = AT_return_error,
	.run = AT_StopSend,
  },
  /* USER CODE END ATCommand */
};

static char circBuffer[CIRC_BUFF_SIZE];
static char command[CMD_SIZE];
static unsigned i = 0;
static uint32_t widx = 0;
static uint32_t ridx = 0;
static uint32_t charCount = 0;
static uint32_t circBuffOverflow = 0;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Parse a command and process it
  * @param  cmd The command
  */
static void parse_cmd(const char *cmd);

/**
  * @brief  Print a string corresponding to an ATEerror_t
  * @param  error_type The AT error code
  */
static void com_error(ATEerror_t error_type);

/**
  * @brief  CMD_GetChar callback from ADV_TRACE
  * @param  rxChar th char received
  * @param  size
  * @param  error
  */
static void CMD_GetChar(uint8_t *rxChar, uint16_t size, uint8_t error);

/**
  * @brief  CNotifies the upper layer that a character has been received
  */
static void (*NotifyCb)(void);

/**
  * @brief  Remove backspace and its preceding character in the Command string
  * @param  cmd string to process
  * @retval 0 when OK, otherwise error
  */
static int32_t CMD_ProcessBackSpace(char *cmd);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void CMD_Init(void (*CmdProcessNotify)(void))
{
  /* USER CODE BEGIN CMD_Init_1 */

  /* USER CODE END CMD_Init_1 */
  UTIL_ADV_TRACE_StartRxProcess(CMD_GetChar);
  /* register call back*/
  if (CmdProcessNotify != NULL)
  {
    NotifyCb = CmdProcessNotify;
  }
  widx = 0;
  ridx = 0;
  charCount = 0;
  i = 0;
  circBuffOverflow = 0;
  /* USER CODE BEGIN CMD_Init_2 */

  /* USER CODE END CMD_Init_2 */
}

void CMD_Process(void)
{
  /* USER CODE BEGIN CMD_Process_1 */

  /* USER CODE END CMD_Process_1 */
  /* Process all commands */
  if (circBuffOverflow == 1)
  {
    com_error(AT_TEST_PARAM_OVERFLOW);
    /*Full flush in case of overflow */
    UTILS_ENTER_CRITICAL_SECTION();
    ridx = widx;
    charCount = 0;
    circBuffOverflow = 0;
    UTILS_EXIT_CRITICAL_SECTION();
    i = 0;
  }

  while (charCount != 0)
  {
#if 0 /* echo On    */
    AT_PPRINTF("%c", circBuffer[ridx]);
#endif /* 0 */

    if (circBuffer[ridx] == AT_ERROR_RX_CHAR)
    {
      ridx++;
      if (ridx == CIRC_BUFF_SIZE)
      {
        ridx = 0;
      }
      UTILS_ENTER_CRITICAL_SECTION();
      charCount--;
      UTILS_EXIT_CRITICAL_SECTION();
      com_error(AT_RX_ERROR);
      i = 0;
    }
    else if ((circBuffer[ridx] == '\r') || (circBuffer[ridx] == '\n'))
    {
      ridx++;
      if (ridx == CIRC_BUFF_SIZE)
      {
        ridx = 0;
      }
      UTILS_ENTER_CRITICAL_SECTION();
      charCount--;
      UTILS_EXIT_CRITICAL_SECTION();

      if (i != 0)
      {
        command[i] = '\0';
        UTILS_ENTER_CRITICAL_SECTION();
        CMD_ProcessBackSpace(command);
        UTILS_EXIT_CRITICAL_SECTION();
        parse_cmd(command);
        i = 0;
      }
    }
    else if (i == (CMD_SIZE - 1))
    {
      i = 0;
      com_error(AT_TEST_PARAM_OVERFLOW);
    }
    else
    {
      command[i++] = circBuffer[ridx++];
      if (ridx == CIRC_BUFF_SIZE)
      {
        ridx = 0;
      }
      UTILS_ENTER_CRITICAL_SECTION();
      charCount--;
      UTILS_EXIT_CRITICAL_SECTION();
    }
  }
  /* USER CODE BEGIN CMD_Process_2 */

  /* USER CODE END CMD_Process_2 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/
static int32_t CMD_ProcessBackSpace(char *cmd)
{
  /* USER CODE BEGIN CMD_ProcessBackSpace_1 */

  /* USER CODE END CMD_ProcessBackSpace_1 */
  uint32_t i = 0;
  uint32_t bs_cnt = 0;
  uint32_t cmd_len = 0;
  /*get command length and number of backspace*/
  while (cmd[cmd_len] != '\0')
  {
    if (cmd[cmd_len] == '\b')
    {
      bs_cnt++;
    }
    cmd_len++;
  }
  /*for every backspace, remove backspace and its preceding character*/
  for (i = 0; i < bs_cnt; i++)
  {
    int curs = 0;
    int j = 0;

    /*set cursor to backspace*/
    while (cmd[curs] != '\b')
    {
      curs++;
    }
    if (curs > 0)
    {
      for (j = curs - 1; j < cmd_len - 2; j++)
      {
        cmd[j] = cmd[j + 2];
      }
      cmd[j++] = '\0';
      cmd[j++] = '\0';
      cmd_len -= 2;
    }
    else
    {
      return -1;
    }
  }
  return 0;
  /* USER CODE BEGIN CMD_ProcessBackSpace_2 */

  /* USER CODE END CMD_ProcessBackSpace_2 */
}

static void CMD_GetChar(uint8_t *rxChar, uint16_t size, uint8_t error)
{
  /* USER CODE BEGIN CMD_GetChar_1 */

  /* USER CODE END CMD_GetChar_1 */
  charCount++;
  if (charCount == (CIRC_BUFF_SIZE + 1))
  {
    circBuffOverflow = 1;
    charCount--;
  }
  else
  {
    circBuffer[widx++] = *rxChar;
    if (widx == CIRC_BUFF_SIZE)
    {
      widx = 0;
    }
  }

  if (NotifyCb != NULL)
  {
    NotifyCb();
  }
  /* USER CODE BEGIN CMD_GetChar_2 */

  /* USER CODE END CMD_GetChar_2 */
}

static void parse_cmd(const char *cmd)
{
  /* USER CODE BEGIN parse_cmd_1 */

  /* USER CODE END parse_cmd_1 */
  ATEerror_t status = AT_OK;
  const struct ATCommand_s *Current_ATCommand;
  int32_t i;

  //if this flag is active then the time request is running, so the user cannot insert any command
  if(TimeRequestTimerExp == 't')
  {
      status = AT_ERROR;
  }

  else if ((cmd[0] != 'A') || (cmd[1] != 'T'))
  {
    status = AT_ERROR;
  }
  else if (cmd[2] == '\0')
  {
    /* status = AT_OK; */
  }
  else if (cmd[2] == '?')
  {
	#ifndef PRINT_RELEASE
	  	#ifndef NO_HELP
			  AT_PPRINTF("AT+<CMD>?        : Help on <CMD>\r\n"
					  "AT+<CMD>         : Run <CMD>\r\n"
					  "AT+<CMD>=<value> : Set the value\r\n"
					  "AT+<CMD>=?       : Get the value\r\n\r\n");
			  for (i = 0; i < (sizeof(ATCommand) / sizeof(struct ATCommand_s)); i++)
			  {
				  AT_PPRINTF(ATCommand[i].help_string);
			  }

			  while (1 != UTIL_ADV_TRACE_IsBufferEmpty())
			  {
				  /* Wait that all printfs are completed*/
			  }
			HAL_Delay(HELP_DISPLAY_FLUSH_DELAY); /* IL NUOVO FIRMWARE NON INCLUDE QUESTO COMANDO, C'E' ANCHE UN CICLO WHILE*/
		#endif /* !NO_HELP */
	#endif
  }
  else
  {
    /* point to the start of the command, excluding AT */
    status = AT_ERROR;
    cmd += 2;
    for (i = 0; i < (sizeof(ATCommand) / sizeof(struct ATCommand_s)); i++)
    {
      if (strncmp(cmd, ATCommand[i].string, ATCommand[i].size_string) == 0)
      {
        Current_ATCommand = &(ATCommand[i]);
        /* point to the string after the command to parse it */
        cmd += Current_ATCommand->size_string;

        /* parse after the command */
        switch (cmd[0])
        {
          case '\0':    /* nothing after the command */
            status = Current_ATCommand->run(cmd);
            break;
          case '=':
            if ((cmd[1] == '?') && (cmd[2] == '\0'))
            {
              status = Current_ATCommand->get(cmd + 1);
            }
            else
            {
              status = Current_ATCommand->set(cmd + 1);
            }
            break;
          case '?':
#ifndef NO_HELP
            AT_PPRINTF(Current_ATCommand->help_string);
#endif /* !NO_HELP */
            status = AT_OK;
            break;
          default:
            /* not recognized */
            break;
        }

        /* we end the loop as the command was found */
        break;
      }
    }
  }

  com_error(status);
  /* USER CODE BEGIN parse_cmd_2 */

  /* USER CODE END parse_cmd_2 */
}

static void com_error(ATEerror_t error_type)
{
	/* USER CODE BEGIN com_error_1 */

	/* USER CODE END com_error_1 */
	if (error_type > AT_MAX)
	{
		error_type = AT_MAX;
	}
	#ifndef PRINT_RELEASE
		AT_PPRINTF(ATError_description[error_type]);
	#else
		if (error_type>=AT_ERROR)
		{
			error_type = AT_ERROR;
		}
		AT_PRINTF("c%d\r\n",error_type);
	#endif
	/* USER CODE BEGIN com_error_2 */

	/* USER CODE END com_error_2 */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
