/**
  ******************************************************************************
  * @file     	usbHandler.c
  * @author		beede
  * @version	1V0
  * @date		Jul 17, 2024
  * @brief
  */


/* Includes ------------------------------------------------------------------*/
#include "usbHandler.h"
#include "usbd_cdc_if.h"
#include "bQueue.h"
#include "bPacket.h"
#include "usbPacketIDs.h"
#include "utils.h"
#include "usbStreamReader.h"
#include "fileStreamWriter.h"

/* Private define ------------------------------------------------------------*/
#define USBHND_RXQUEUESIZE					1024
#define USBHND_TXQUEUESIZE					1024

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t rxBuffer[USBHND_RXQUEUESIZE];
static QUEUE_Typedef rxQueue = {rxBuffer, USBHND_RXQUEUESIZE, 0, 0 };
static uint8_t txBuffer[USBHND_TXQUEUESIZE];
static QUEUE_Typedef txQueue = {txBuffer, USBHND_TXQUEUESIZE, 0, 0 };

static volatile uint32_t debugRxDataLost;

static USR_StreamReader_td usbFileStream;

/* Private function prototypes -----------------------------------------------*/
static void USBHND_PacketHandler(BPKT_Packet_TD *packet);
static void USBHND_PacketACKHandler(BPKT_Packet_TD *packet);
static void USBHND_sendData(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Handle received USB data
  * @param	data: pointer to the received data
  * @param	length: amount of dta received
  * @retval	None
  */
void CDC_ReceivedHandler(uint8_t *data, uint32_t length)
{
	if(QUEUE_AddArray(&rxQueue, data, length) == QUEUE_NOSPACE)
		debugRxDataLost += length;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Tick handler
  * @param	NOne
  * @retval	None
  */
void USBHND_milli(void)
{
	BPKT_Packet_TD packet;
	while(1)
	{
		BPKT_STATUS_ENUM result = PKT_Decode(&rxQueue, &packet);
		if(result == BPKT_OK)
		{
			USBHND_PacketHandler(&packet);
			QUEUE_Remove(&rxQueue, BPKT_PACKETSIZE(packet.length));
			continue;
		}
		else if (result == BPKT_NOTENOUGHDATA)
		{
			break;
		}

		QUEUE_Remove(&rxQueue, 1);
	}

	USBHND_sendData();
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Handler for parsed packets
  * @param	NOne
  * @retval	None
  */
static void USBHND_PacketHandler(BPKT_Packet_TD *packet)
{
	uint8_t data[10];

	switch(packet->data[0])
	{

	case pktACK:
		USBHND_PacketACKHandler(packet);
		break;

	case pktFileHeader:
	{
		uint32_t length = BYTESTOUINT32(packet->data, 1);
		uint32_t crc = BYTESTOUINT32(packet->data, 5);
		BSTREAM_Enum result = USR_Start(&usbFileStream, length, crc);
		if(result != BSTREAM_OK)
		{
			data[0] = pktNACK;
			data[1] = pktFileHeader;
			data[2] = result;
			USBHND_sendPacket(data, 3);
			break;
		}

		result = FSW_WriteFile((BSTREAM_Reader_td*)&usbFileStream);
		if(result != BSTREAM_OK)
		{
			USR_Cancel(&usbFileStream);

			data[0] = pktNACK;
			data[1] = pktFileHeader;
			data[2] = result;
			USBHND_sendPacket(data, 3);
			break;
		}

		data[0] = pktACK;
		data[1] = pktFileHeader;
		data[2] = usbFileStream.streamID;
		USBHND_sendPacket(data, 3);
		break;
	}

	case pktUSRData:
	{
		uint32_t offset = BYTESTOUINT32(packet->data, 2);
		USR_DataReceivedHandler(USR_GetStreamByID(packet->data[1]), offset, &packet->data[6], (packet->length - 6));
		break;
	}
	}
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Handler for parsed packets
  * @param	NOne
  * @retval	None
  */
static void USBHND_PacketACKHandler(BPKT_Packet_TD *packet)
{
	switch(packet->data[1])
	{
	case pktUSRAlive:
	{
		USR_AliveHandler(USR_GetStreamByID(packet->data[2]));
		break;
	}
	}
}


/*----------------------------------------------------------------------------*/
/**
  * @brief	Send a packet over USB
  * @param	data: pointer to the data to be encoded
  * @param	length: amount of data to be transmitted
  * @retval	Send successful
  */
bool USBHND_sendPacket(uint8_t *data, uint32_t length)
{
	assert(length < BPKT_MAXDATALENGTH);
	if(PKT_Encode(data, length, &txQueue) != BPKT_OK)
		return false;

	//Force send;
	USBHND_sendData();
	return true;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Send a packet over USB
  * @param	data: pointer to the data to be encoded
  * @param	length: amount of data to be transmitted
  * @retval	Send successful
  */
static void USBHND_sendData(void)
{
	//TODO: USB transmit limit?
	uint32_t length = txQueue.size - txQueue.out;
	if(length > QUEUE_COUNT(&txQueue))
		length = QUEUE_COUNT(&txQueue);

	if(CDC_Transmit_FS(&txQueue.pBuff[txQueue.out], length) == USBD_OK)
		QUEUE_Remove(&txQueue, length);
}

