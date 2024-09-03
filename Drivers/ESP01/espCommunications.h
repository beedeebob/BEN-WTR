/*
 * comms.h
 *
 *  Created on: Feb 22, 2024
 *      Author: ben
 */

#ifndef ESPCOMMS_H_
#define ESPCOMMS_H_

/* Includes ------------------------------------------------------------------*/
#include "espCore.h"
#include "espDefs.h"
#include "stdint.h"
#include "esp.h"
#include "espQueue.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
	uint8_t toEspBuffer[ESPCOMMS_BUFFERSIZE];
	ESPQ_Typedef toEspQ;
	uint8_t fromEspBuffer[ESPCOMMS_BUFFERSIZE];
	ESPQ_Typedef fromExpQ;
}ESP_Communications_td;

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ESPCOMMS_structInit(ESP_Communications_td *commsStruct);
void ESPCOMMS_tick(ESP_td *esp);
uint32_t ESPCOMMS_GetTransmitSpace(ESP_td *esp);
ESP_Result ESPCOMMS_Command(ESP_td *esp, uint16_t command, uint8_t *parameters, uint16_t parameterSize);
void ESPCOMMS_PacketDataReceive(ESP_td *esp, uint8_t *pData, uint32_t length);

#endif /* ESPCOMMS_H_ */
