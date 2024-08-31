/**
  ******************************************************************************
  * @file     	usbHandler.h
  * @author		beede
  * @version	1V0
  * @date		Jul 17, 2024
  * @brief
  */


#ifndef INC_USBHANDLER_H_
#define INC_USBHANDLER_H_

/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "stdint.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void USBHND_milli(void);
bool USBHND_sendPacket(uint8_t *data, uint32_t length);

#endif /* INC_USBHANDLER_H_ */
