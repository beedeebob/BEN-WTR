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
#include "stdint.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ESPCOMMS_structInit(ESP_Communications_td *commsStruct);
void ESPCOMMS_tick(ESP_td *esp);
uint32_t ESPCOMMS_GetTransmitSpace(ESP_td *esp);
ESP_Result ESPCOMMS_Command(ESP_td *esp, uint16_t command, uint8_t *parameters, uint16_t parameterSize);
void ESPCOMMS_PacketDataReceive(ESP_td *esp, uint8_t *pData, uint32_t length);
void ESPCOMMS_PacketByteReceive(ESP_td *esp, uint8_t data);

#endif /* ESPCOMMS_H_ */
