
/*
 * comms.c
 *
 *  Created on: Feb 22, 2024
 *      Author: ben
 */


/* Includes ------------------------------------------------------------------*/
#include "espCommunications.h"
#include "espUsartEncoding.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void ESP_PacketReceivedHandler(ESP_td *esp, uint8_t *data, uint32_t length);
ESP_Result USART_Transmit(ESP_td *esp, uint8_t *data, uint32_t size);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Initialize structure
  * @param	commsStruct: pointer to structure
  * @retval	None
  */
void ESPCOMMS_structInit(ESP_Communications_td *commsStruct)
{
	commsStruct->toEspQ.pBuff = commsStruct->toEspBuffer;
	commsStruct->toEspQ.size =  ESPCOMMS_BUFFERSIZE;
	commsStruct->toEspQ.in =  0;
	commsStruct->toEspQ.out =  0;

	commsStruct->fromEspQ.pBuff = commsStruct->fromEspBuffer;
	commsStruct->fromEspQ.size = ESPCOMMS_BUFFERSIZE;
	commsStruct->fromEspQ.in = 0;
	commsStruct->fromEspQ.out = 0;
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
	while(ESPQ_COUNT(&commsStruct->fromEspQ) > 0)
	{
		ESPPKT_DECODEEnum result = ESPPKT_Decode(&commsStruct->fromEspQ, &receivedPacket);
		if(result == ESPPKT_OK)
		{
			ESP_PacketReceivedHandler(esp, receivedPacket.data, receivedPacket.length);
			ESPQ_Remove(&commsStruct->fromEspQ, ESPPKT_PACKETSIZE(receivedPacket.length));
		}
		else if(result == ESPPKT_NOTENOUGHDATA)
			break;
		else
			ESPQ_Remove(&commsStruct->fromEspQ, 1);
	}

	//Transfer out to USART
	while(ESPQ_COUNT(&commsStruct->toEspQ) > 0)
	{
		uint32_t toSend = commsStruct->toEspQ.size - commsStruct->toEspQ.out;
		if(toSend > ESPQ_COUNT(&commsStruct->toEspQ))
			toSend = ESPQ_COUNT(&commsStruct->toEspQ);

		if(USART_Transmit(esp, &commsStruct->toEspQ.pBuff[commsStruct->toEspQ.out], toSend) == ESP_OK)
			ESPQ_Remove(&commsStruct->toEspQ, toSend);
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
	ESPQ_AddArray(&esp->comms.fromEspQ, pData, length);
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Get space in the transmission system
  * @param	esp: pointer to the esp system
  * @retval	space
  */
uint32_t ESPCOMMS_GetTransmitSpace(ESP_td *esp)
{
	return ESPQ_SPACE(&esp->comms.toEspQ);
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
	uint32_t crc = 0;
	ESP_Result result = ESPPKT_EncodeStart(&esp->comms.toEspQ, 2 + parameterSize, data, 2, &crc);
	if(result != ESP_OK)
		return result;
	result = ESPPKT_EncodeEnd(&esp->comms.toEspQ, parameters, parameterSize, &crc);
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
__attribute__((weak)) void ESP_PacketReceivedHandler(ESP_td *esp, uint8_t *data, uint32_t length)
{
	return;
}
