
/*
 * comms.c
 *
 *  Created on: Feb 22, 2024
 *      Author: ben
 */


/* Includes ------------------------------------------------------------------*/
#include "comms.h"
#include "benQueue.h"
#include "usart.h"
#include "benPacket.h"
#include "led.h"

/* Private define ------------------------------------------------------------*/
#define COMMS_BUFFERSIZE				128

//Event FLAGS
enum
{
	COMMS_FLAG_TASKRUN = 0x01,
};

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void COMMS_ESPPacketReceived(BPKT_Packet_TD *packet);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Task milli routine
  * @param	None
  * @retval	None
  */
void COMMS_milli(void)
{
	//Parse received data for packets
	BPKT_Packet_TD receivedPacket = {0};
	while(QUEUE_COUNT(&bpktQueue) > 0)
	{
		BPKT_STATUS_ENUM result = PKT_Decode(&bpktQueue, &receivedPacket);
		if(result == BPKT_OK)
		{
			COMMS_ESPPacketReceived(&receivedPacket);
			QUEUE_Remove(&bpktQueue, BPKT_PACKETSIZE(receivedPacket.length));
		}
		else if(result == BPKT_NOTENOUGHDATA)
			break;
		else
			QUEUE_Remove(&bpktQueue, 1);
	}

	//Transfer out to USART
	if(QUEUE_COUNT(&toUSARTESP) > 0)
	{
		USART_Transmit(&usartESP, &toUSARTESP);
	}

	//Transfer out to USB
	if(QUEUE_COUNT(&toUSARTPC) > 0)
	{
		USART_Transmit(&usartPC, &toUSARTPC);
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
void ESP_PacketDataReceive(ESP_td *esp, uint8_t *pData, uint32_t length)
{
		QUEUE_AddArray(&toUSARTPC, pData, length);
		if(QUEUE_AddArray(&bpktQueue, pData, length) != QUEUE_OK)
			return HAL_ERROR;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Transmit to ESP device
  * @param	esp: pointer to the esp system
  * @param	data: pointer to the data to transit
  * @param	length: amount of data to transmit
  * @retval	HAL_StatusTypeDef
  */
ESP_Result ESP_SendPacket(ESP_td *esp, uint8_t *data, uint32_t length)
{
	if(PKT_Encode(data, length, &toUSARTESP) != BPKT_OK)
		return HAL_ERROR;
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
