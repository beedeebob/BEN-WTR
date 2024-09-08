/**
  ******************************************************************************
  * @file     	espDriver.h
  * @author		beede
  * @version	1V0
  * @date		Aug 31, 2024
  * @brief
  */


#ifndef ESP01_ESPDRIVER_H_
#define ESP01_ESPDRIVER_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ESPDRV_Init(void);
void ESPDRV_milli(void);
void ESPDRV_DMATransferCompleteHandler(DMA_TypeDef * dma);

#endif /* ESP01_ESPDRIVER_H_ */
