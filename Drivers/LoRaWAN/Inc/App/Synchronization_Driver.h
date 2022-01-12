/*!
 * \file      LmhpClockSync.h
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
  * @file    LmhpClockSync.h
  * @author  MCD Application Team
  * @brief   Header for Clock Synchronisation Package module
  ******************************************************************************
  */
#ifndef __LMHP_CLOCK_SYNC_H__
#define __LMHP_CLOCK_SYNC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "LmhPackage.h"

/* Exported defines ----------------------------------------------------------*/
/*!
 * Clock sync package identifier.
 *
 * \remark This value must be unique amongst the packages
 */
#define PACKAGE_ID_CLOCK_SYNC    1
#define START_PERIODIC_REQ_TIME  30//60*60*3 //MODIFICA 128*2^8 circa 10 minuti

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

LmhPackage_t *LmphClockSyncPackageFactory( void );

LmHandlerErrorStatus_t LmhpClockSyncAppTimeReq( void );

void PeriodicityTimeRequest(uint32_t PeriodTime); //MODIFICA
void TimeRequestSynchNotPeriodic(void);
extern TimerEvent_t PeriodicTimeStartTimer;
#ifdef __cplusplus
}
#endif

#endif // __LMHP_CLOCK_SYNC_H__
