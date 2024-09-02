/**
  ******************************************************************************
  * @file     	espPost.h
  * @author		beede
  * @version	1V0
  * @date		Sep 2, 2024
  * @brief
  */


#ifndef SRC_ESPPOST_H_
#define SRC_ESPPOST_H_

/* Includes ------------------------------------------------------------------*/
#include "espCore.h"
#include "stdint.h"
#include "stdbool.h"
#include "espCommunications.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct
{
	ESP_Communications_td comms;		//Communications control
	uint8_t state;						//Control state
	uint8_t tmr;						//Control timer

	//Update setting
	uint32_t pressure;
	int32_t temperature;
	uint32_t humidity;

	//Flags
	bool hasWeatherUpdate;				//Set when the pressure, temperature, and humidity need to be posted
}ESP_td;

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ESP_Init(void);
void ESP_tick(void);
void ESP_ESPCOMMSHandler(uint8_t *data, uint32_t length);
void ESP_PostWeatherUpdate(ESP_td *esp, uint32_t pressure, int32_t temperature, uint32_t humidity);
#endif /* SRC_ESPPOST_H_ */
