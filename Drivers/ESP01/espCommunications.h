/*
 * comms.h
 *
 *  Created on: Feb 22, 2024
 *      Author: ben
 */

#ifndef COMMS_H_
#define COMMS_H_

/* Includes ------------------------------------------------------------------*/
#include "espCore.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
	static uint8_t toUsart1Buffer[COMMS_BUFFERSIZE];
	static QUEUE_Typedef toUSARTESP = {toUsart1Buffer, COMMS_BUFFERSIZE, 0, 0};
	static uint8_t toUSBBuffer[COMMS_BUFFERSIZE];
	static QUEUE_Typedef toUSARTPC = {toUSBBuffer, COMMS_BUFFERSIZE, 0, 0};
	static uint8_t bpktBuffer[COMMS_BUFFERSIZE];
	static QUEUE_Typedef bpktQueue = {bpktBuffer, COMMS_BUFFERSIZE, 0, 0};
};

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void COMMS_milli(void);
ESP_Result ESP_SendPacket(uint8_t *data, uint32_t length);
void ESP_PacketDataReceive(ESP_td *esp, uint8_t *pData, uint32_t length);

#endif /* COMMS_H_ */
