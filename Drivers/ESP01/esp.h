
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

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct
{
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
void ESP_ESPCOMMSHandler(uint8_t *data, uint32_t length);
void ESP_PostWeatherUpdate(ESP_td *esp, uint32_t pressure, int32_t temperature, uint32_t humidity);

#endif /* ESP_H_ */
