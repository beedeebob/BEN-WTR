
/*
 * comms.c
 *
 *  Created on: Feb 22, 2024
 *      Author: ben
 */


/* Includes ------------------------------------------------------------------*/
#include "espCommunications.h"
#include "espQueue.h"
#include "espUsartEncoding.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void ESP_PacketReceivedHandler(ESP_td *esp, ESPPKT_RxPacket_TD *packet);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Initialize structure
  * @param	commsStruct: pointer to structure
  * @retval	None
  */
void ESPCOMMS_structInit(ESP_Communications_td *commsStruct)
{
	commsStruct->toEspBuffer;
	commsStruct->toEspQ.pBuff = &commsStruct->toEspBuffer;
	commsStruct->toEspQ.size =  ESPCOMMS_BUFFERSIZE;
	commsStruct->toEspQ.in =  0;
	commsStruct->toEspQ.out =  0;

	commsStruct->fromEspBuffer;
	commsStruct->fromExpQ.pBuff = &commsStruct->fromEspBuffer;
	commsStruct->fromExpQ.size = ESPCOMMS_BUFFERSIZE;
	commsStruct->fromExpQ.in = 0;
	commsStruct->fromExpQ.out = 0;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Task tick routine
  * @param	commsStruct: pointer to structure
  * @retval	None
  */
void ESPCOMMS_tick(ESP_td *esp)
{
	ESP_Communications_td *commsStruct = &esp->comms;

	//Parse received data for packets
	ESPPKT_RxPacket_TD receivedPacket = {0};
	while(ESPQ_COUNT(&commsStruct->fromEspBuffer) > 0)
	{
		ESPPKT_DECODEEnum result = ESPPKT_Decode(&commsStruct->fromEspBuffer, &receivedPacket);
		if(result == BPKT_OK)
		{
			ESP_PacketReceivedHandler(commsStruct, &receivedPacket);
			ESPQ_Remove(&commsStruct->fromEspBuffer, ESPPKT_PACKETSIZE(receivedPacket.length));
		}
		else if(result == BPKT_NOTENOUGHDATA)
			break;
		else
			ESPQ_Remove(&commsStruct->fromEspBuffer, 1);
	}

	//Transfer out to USART
	if(ESPQ_COUNT(&commsStruct->toEspBuffer) > 0)
	{
		USART_Transmit(esp, &commsStruct->toEspBuffer);
	}
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	USART received callback handler
  * @param	esp: pointer to the esp system
  * @param	data: pointer to the data to transit
  * @param	length: amount of data to transmit
  * @retval	None
  */
void ESPCOMMS_PacketDataReceive(ESP_td *esp, uint8_t *pData, uint32_t length)
{
	ESPQ_AddArray(&esp->comms.fromEspBuffer, pData, length);
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Get space in the transmission system
  * @param	esp: pointer to the esp system
  * @retval	space
  */
uint32_t ESPCOMMS_GetTransmitSpace(ESP_td *esp)
{
	return ESPQ_SPACE(&esp->comms.toEspBuffer);
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Command
  * @param	esp: pointer to the esp system
  * @param	command: command ID from @ref espPkt_commands
  * @param	parameters: pointer to the parameter list
  * @param	parameterSize: number of bytes in the parameter list
  * @retval	ESP_Result
  */
ESP_Result ESPCOMMS_Command(ESP_td *esp, uint16_t command, uint8_t *parameters, uint16_t parameterSize)
{
	uint8_t data[2];
	data[0] = (uint8_t)command;
	data[1] = (uint8_t)(command >> 8);
	uint16_t crc = 0;
	ESP_Result result = ESPPKT_EncodeStart(&esp->comms.toEspBuffer, 2 + parameterSize, data, 2, &crc);
	if(result != ESP_OK)
		return result;
	result = ESPPKT_EncodeEnd(&esp->comms.toEspBuffer, parameters, parameterSize, &crc);
	if(result != ESP_OK)
		return result;
	return ESP_OK;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Transmit a USART packet
  * @param	esp: pointer to the esp system
  * @param	data: pointer to the data to transmit
  * @param	size: amount of data to transmit
  * @retval	ESP_Result
  */
__attribute__((weak)) ESP_Result USART_Transmit(ESP_td *esp, uint8_t *data, uint32_t size)
{
	return ESP_OK;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Packet received callback handler
  * @param	esp: pointer to the esp system
  * @param	packet: pointer to the received packet
  * @retval	None
  */
__attribute__((weak)) void ESP_PacketReceivedHandler(ESP_td *esp, ESPPKT_RxPacket_TD *packet)
{
	return;
}
