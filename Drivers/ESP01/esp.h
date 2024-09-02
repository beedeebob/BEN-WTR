
/**
  ******************************************************************************
  * @file     	esp.h
  * @author		ben
  * @version	1V0
  * @date		Mar 5, 2024
  * @brief
  */

#ifndef ESP_H_
#define ESP_H_

/* Includes ------------------------------------------------------------------*/
#include "espCore.h"
#include "stdint.h"
#include "stdbool.h"
#include "espCommunications.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct ESP_td
{
	struct ESP_td *next;				//Pointer for chaining
	ESP_Communications_td comms;		//Communications control
}ESP_td;

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ESP_Initialize(ESP_td *esp);
ESP_Result ESP_Start(ESP_td *esp);
ESP_Result ESP_Close(ESP_td *esp);
void ESP_tick(void);

ESP_Result ESP_Command(ESP_td *esp, uint16_t command, uint8_t *parameters, uint16_t parameterSize);

#endif /* ESP_H_ */
