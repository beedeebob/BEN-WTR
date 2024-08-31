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
void USART_ReceiveHandler(uint8_t *data, uint32_t size);

#endif /* ESP01_ESPDRIVER_H_ */
