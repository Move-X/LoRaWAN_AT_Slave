/*!
 * \file      LmhpClockSync.c
 *
 * \brief     Implements the LoRa-Alliance clock synchronization package
 *            Specification: https://lora-alliance.org/sites/default/files/2018-09/application_layer_clock_synchronization_v1.0.0.pdf
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */
/**
  ******************************************************************************
  *
  *          Portions COPYRIGHT 2020 STMicroelectronics
  *
  * @file    LmhpClockSync.c
  * @author  MCD Application Team
  * @brief   Clock Synchronisation Package definition
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "LmHandler.h"
#include "Synchronization_Driver.h"
#include "utilities.h"
#include "utilities_def.h"
#include "stm32_seq.h"

/* Private typedef -----------------------------------------------------------*/
/*!
 * Package current context
 */
typedef struct LmhpClockSyncState_s
{
  bool Initialized;
  bool IsRunning;
  uint8_t DataBufferMaxSize;
  uint8_t *DataBuffer;
  union
  {
    uint8_t Value;
    struct
    {
      uint8_t TokenReq:    4;
      uint8_t AnsRequired: 1;
      uint8_t RFU:         3;
    }Fields;
  }TimeReqParam;
  bool AppTimeReqPending;
  bool AdrEnabledPrev;
  uint8_t NbTransPrev;
  uint8_t DataratePrev;
  uint8_t NbTransmissions;
}LmhpClockSyncState_t;

typedef enum LmhpClockSyncMoteCmd_e
{
  CLOCK_SYNC_PKG_VERSION_ANS       = 0x00,
  CLOCK_SYNC_APP_TIME_REQ          = 0x01,
  CLOCK_SYNC_APP_TIME_PERIOD_ANS   = 0x02,
  CLOCK_SYNC_FORCE_RESYNC_ANS      = 0x03,
}LmhpClockSyncMoteCmd_t;

//typedef enum LmhpClockSyncSrvCmd_e
//{
//  CLOCK_SYNC_PKG_VERSION_REQ       = 0x00,
//  CLOCK_SYNC_APP_TIME_ANS          = 0x01,
//  CLOCK_SYNC_APP_TIME_PERIOD_REQ   = 0x02,
//  CLOCK_SYNC_FORCE_RESYNC_REQ      = 0x03,
//}LmhpClockSyncSrvCmd_t;

/* Private define ------------------------------------------------------------*/
/*!
 * LoRaWAN Application Layer Clock Synchronization Specification
 */
#define CLOCK_SYNC_PORT                             202 //For Lora Alliance Protocoll
#define CLOCK_SYNC_ID                               1
#define CLOCK_SYNC_VERSION                          1

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*!
 * Initializes the package with provided parameters
 *
 * \param [IN] params            Pointer to the package parameters
 * \param [IN] dataBuffer        Pointer to main application buffer
 * \param [IN] dataBufferMaxSize Main application buffer maximum size
 */
static void LmhpClockSyncInit( void *params, uint8_t *dataBuffer, uint8_t dataBufferMaxSize );

/*!
 * Returns the current package initialization status.
 *
 * \retval status Package initialization status
 *                [true: Initialized, false: Not initialized]
 */
static bool LmhpClockSyncIsInitialized( void );

/*!
 * Returns the package operation status.
 *
 * \retval status Package operation status
 *                [true: Running, false: Not running]
 */
static bool LmhpClockSyncIsRunning( void );

/*!
 * Processes the internal package events.
 */
static void LmhpClockSyncProcess( void );

/*!
 * Processes the MCSP Confirm
 *
 * \param [IN] mcpsConfirm MCPS confirmation primitive data
 */
static void LmhpClockSyncOnMcpsConfirm( McpsConfirm_t *mcpsConfirm );

/*!
 * Processes the MCPS Indication
 *
 * \param [IN] mcpsIndication     MCPS indication primitive data
 */
static void LmhpClockSyncOnMcpsIndication( McpsIndication_t *mcpsIndication );

static void OnPeriodicTimeStartTimer( void *context );

/* Private variables ---------------------------------------------------------*/
static LmhpClockSyncState_t LmhpClockSyncState =
{
  .Initialized =        false,
  .IsRunning =          false,
  .TimeReqParam.Value = 0,
  .AppTimeReqPending =  false,
  .AdrEnabledPrev =     false,
  .NbTransPrev =        0,
  .NbTransmissions =    0
};

static LmhPackage_t LmhpClockSyncPackage =
{
  .Port =                       CLOCK_SYNC_PORT,
  .Init =                       LmhpClockSyncInit,//LmHandlerPackageRegister in LmHandler.c riga 781
  .IsInitialized =              LmhpClockSyncIsInitialized,//LmHandlerPackageIsInitialized in LmHandler.c riga 1709 & LmHandlerPackagesProcess in LmHandler.c riga 816
  .IsRunning =                  LmhpClockSyncIsRunning,// Non sembra usato
  .Process =                    LmhpClockSyncProcess,// LmHandlerPackagesProcess in LmHandler.c riga 816
  .OnMcpsConfirmProcess =       LmhpClockSyncOnMcpsConfirm,// LmHandlerPackagesNotify riga 1721 LmHandler.c
  .OnMcpsIndicationProcess =    LmhpClockSyncOnMcpsIndication,// LmHandlerPackagesNotify riga 1721 LmHandler.c
  .OnMlmeConfirmProcess =       NULL,                          // Not used in this package
  .OnJoinRequest =              NULL,                          // To be initialized by LmHandler LmHandlerPackageRegister in LmHandler.c riga 781
  .OnSendRequest =              NULL,                          // To be initialized by LmHandler LmHandlerPackageRegister in LmHandler.c riga 781
  .OnDeviceTimeRequest =        NULL,                          // To be initialized by LmHandler LmHandlerPackageRegister in LmHandler.c riga 781
};

/*!
 * Periodic Time start timer
 */
TimerEvent_t PeriodicTimeStartTimer;//MODIFICA

/* Exported functions ---------------------------------------------------------*/
LmhPackage_t *LmphClockSyncPackageFactory( void )
{
  return &LmhpClockSyncPackage;
}

// You can't remove this function it's essential for sinchronization
LmHandlerErrorStatus_t LmhpClockSyncAppTimeReq( void )
{
  if( LmHandlerIsBusy( ) == true )
  {
    return LORAMAC_HANDLER_ERROR;
  }

  if( LmhpClockSyncState.AppTimeReqPending == false )
  {
    MibRequestConfirm_t mibReq;

    // Disable ADR
    mibReq.Type = MIB_ADR;
    LoRaMacMibGetRequestConfirm( &mibReq );
    LmhpClockSyncState.AdrEnabledPrev = mibReq.Param.AdrEnable;
    mibReq.Param.AdrEnable = false;
    LoRaMacMibSetRequestConfirm( &mibReq );

    // Set NbTrans = 1
    mibReq.Type = MIB_CHANNELS_NB_TRANS;
    LoRaMacMibGetRequestConfirm( &mibReq );
    LmhpClockSyncState.NbTransPrev = mibReq.Param.ChannelsNbTrans;
    mibReq.Param.ChannelsNbTrans = 1;
    LoRaMacMibSetRequestConfirm( &mibReq );

    // Store data rate
    mibReq.Type = MIB_CHANNELS_DATARATE;
    LoRaMacMibGetRequestConfirm( &mibReq );
    LmhpClockSyncState.DataratePrev = mibReq.Param.ChannelsDatarate;

    // Add DeviceTimeReq MAC command.
    // In case the network server supports this more precise command
    // this package will use DeviceTimeAns answer as clock synchronization
    // mechanism.
    LmhpClockSyncPackage.OnDeviceTimeRequest( );
  }

  SysTime_t curTime = SysTimeGet( );
  uint8_t dataBufferIndex = 0;

  // Substract Unix to Gps epcoh offset. The system time is based on Unix time.
  curTime.Seconds -= UNIX_GPS_EPOCH_OFFSET;

  LmhpClockSyncState.DataBuffer[dataBufferIndex++] = CLOCK_SYNC_APP_TIME_REQ;
  LmhpClockSyncState.DataBuffer[dataBufferIndex++] = ( curTime.Seconds >> 0  ) & 0xFF;
  LmhpClockSyncState.DataBuffer[dataBufferIndex++] = ( curTime.Seconds >> 8  ) & 0xFF;
  LmhpClockSyncState.DataBuffer[dataBufferIndex++] = ( curTime.Seconds >> 16 ) & 0xFF;
  LmhpClockSyncState.DataBuffer[dataBufferIndex++] = ( curTime.Seconds >> 24 ) & 0xFF;
  LmhpClockSyncState.TimeReqParam.Fields.AnsRequired = 0;
  LmhpClockSyncState.DataBuffer[dataBufferIndex++] = LmhpClockSyncState.TimeReqParam.Value;
//  APP_LOG(TS_ON, VLEVEL_ALWAYS, "\r\n Cur_Time: Seconds%i SubSeconds:%i\r\n", curTime.Seconds, curTime.SubSeconds);
  LmHandlerAppData_t appData =
  {
    .Buffer = LmhpClockSyncState.DataBuffer,
    .BufferSize = dataBufferIndex,
    .Port = CLOCK_SYNC_PORT
  };
  LmhpClockSyncState.AppTimeReqPending = true;

  bool current_dutycycle;
  LmHandlerGetDutyCycleEnable(&current_dutycycle);

  // force Duty Cycle OFF to this Send
  LmHandlerSetDutyCycleEnable(false);
  LmHandlerErrorStatus_t status = LmhpClockSyncPackage.OnSendRequest( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, true );
//  APP_LOG(TS_ON, VLEVEL_ALWAYS, "\r\n LmhpClockSyncAppTimeReq(). Send_data_with_Command:%02X PayloadofCurTime:%02X-%02X-%02X-%02X Answer_required:%i Time_Req_Param_Value:%02X On_port:%i Length:%i\r\n",appData.Buffer[0],appData.Buffer[1],appData.Buffer[2],appData.Buffer[3], appData.Buffer[4], LmhpClockSyncState.TimeReqParam.Fields.AnsRequired,  appData.Buffer[5], appData.Port, appData.BufferSize);
  // restore initial Duty Cycle
  LmHandlerSetDutyCycleEnable(current_dutycycle);

  return status;
}

/* Private  functions ---------------------------------------------------------*/
static void LmhpClockSyncInit( void * params, uint8_t *dataBuffer, uint8_t dataBufferMaxSize )
{
  if( dataBuffer != NULL )
  {
    LmhpClockSyncState.DataBuffer = dataBuffer;
    LmhpClockSyncState.DataBufferMaxSize = dataBufferMaxSize;
    LmhpClockSyncState.Initialized = true;
    LmhpClockSyncState.IsRunning = true;
    TimerInit( &PeriodicTimeStartTimer, OnPeriodicTimeStartTimer );
  }
  else
  {
    LmhpClockSyncState.IsRunning = false;
    LmhpClockSyncState.Initialized = false;
  }
}

static bool LmhpClockSyncIsInitialized( void )
{
  return LmhpClockSyncState.Initialized;
}

static bool LmhpClockSyncIsRunning( void )
{
  if( LmhpClockSyncState.Initialized == false )
  {
    return false;
  }

  return LmhpClockSyncState.IsRunning;
}

static void LmhpClockSyncProcess( void )
{

   //APP_LOG(TS_ON, VLEVEL_ALWAYS, "\r\n LmhpClockSyncProcess() NbTransmissions= %i\r\n", LmhpClockSyncState.NbTransmissions);
  if( LmhpClockSyncState.NbTransmissions > 0 )
  {
    if( LmhpClockSyncAppTimeReq( ) == LORAMAC_HANDLER_SUCCESS )
    {
      LmhpClockSyncState.NbTransmissions--;
    }
  }
}

static void LmhpClockSyncOnMcpsConfirm( McpsConfirm_t *mcpsConfirm )
{
  MibRequestConfirm_t mibReq;

  if( LmhpClockSyncState.AppTimeReqPending == true )
  {
    // Revert ADR setting
    mibReq.Type = MIB_ADR;
    mibReq.Param.AdrEnable = LmhpClockSyncState.AdrEnabledPrev;
    LoRaMacMibSetRequestConfirm( &mibReq );

    // Revert NbTrans setting
    mibReq.Type = MIB_CHANNELS_NB_TRANS;
    mibReq.Param.ChannelsNbTrans = LmhpClockSyncState.NbTransPrev;
    LoRaMacMibSetRequestConfirm( &mibReq );

    // Revert data rate setting
    mibReq.Type = MIB_CHANNELS_DATARATE;
    mibReq.Param.ChannelsDatarate = LmhpClockSyncState.DataratePrev;
    LoRaMacMibSetRequestConfirm( &mibReq );

    LmhpClockSyncState.AppTimeReqPending = false;
  }
}

static void LmhpClockSyncOnMcpsIndication( McpsIndication_t *mcpsIndication )
{
  // If you want you can ADD LoRa Alliance Protocoll  here.
}

static void OnPeriodicTimeStartTimer( void *context )
{
  LmhpClockSyncState.NbTransmissions = 1;
//  fuota_start = true;
 // TimerStart( &PeriodicTimeStartTimer);
//  APP_LOG(TS_ON, VLEVEL_ALWAYS, "\r\n ON PERIODIC TIME START TIMER: SCHEDULED.  NbTransmissions = %i\r\n", LmhpClockSyncState.NbTransmissions);
}


void PeriodicityTimeRequest(uint32_t PeriodTime) //128*2^(periodTime) MODIFICA
{
     PeriodTime = PeriodTime + randr(0, 30);//(128 << PeriodTime) + randr(0, 30);
     // Start Periodic timer

//     TimerSetValue( &PeriodicTimeStartTimer, PeriodTime * 1000 );
//     TimerStart( &PeriodicTimeStartTimer );
//     APP_LOG(TS_ON, VLEVEL_ALWAYS, "### CLOCK SYNC APP TIME PERIOD REQ IN : %i sec \r\n", PeriodTime);
}

void TimeRequestSynchNotPeriodic(void)
{
	LmhpClockSyncState.NbTransmissions = 1;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);
}
