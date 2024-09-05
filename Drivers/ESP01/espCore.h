/**
  ******************************************************************************
  * @file     	espCore.h
  * @author		ben
  * @version	1V0
  * @date		Aug 31, 2024
  * @brief
  */


#ifndef ESP01_ESPCORE_H_
#define ESP01_ESPCORE_H_

/* Includes ------------------------------------------------------------------*/
#include "espQueue.h"
#include "stdint.h"
#include "espDefs.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum
{
	ESP_OK = 0,
	ESP_BUSY = 1,
	ESP_ALREADYEXISTS = 2,
	ESP_NOSPACE = 3,
	ESP_LIMITSEXCEEDED = 3
}ESP_Result;

typedef struct
{
	uint8_t toEspBuffer[ESPCOMMS_BUFFERSIZE];
	ESPQ_Typedef toEspQ;
	uint8_t fromEspBuffer[ESPCOMMS_BUFFERSIZE];
	ESPQ_Typedef fromEspQ;
}ESP_Communications_td;

typedef struct ESP_td
{
	struct ESP_td *next;				//Pointer for chaining
	ESP_Communications_td comms;		//Communications control
}ESP_td;

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* ESP01_ESPCORE_H_ */
