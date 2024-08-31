/**
  ******************************************************************************
  * @file     	benPacket.c
  * @author		beede
  * @version	1V0
  * @date		Feb 26, 2024
  * @brief
  */


/* Information ---------------------------------------------------------------*/
/*
PACKET
o STX           1 byte          0x02
o Length        2 bytes
o CRC           4 bytes
o Data
o CRC           4 bytes
o ETX                           0x03
*/

/* Includes ------------------------------------------------------------------*/
#include "benPacket.h"
#include "benQueue.h"
#include "stdint.h"

/* Private define ------------------------------------------------------------*/
/* CRC-32C (iSCSI) polynomial in reversed bit order. */
#define POLY 0x82f63b78

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Accumulate CRC value
  * @param	crc: the current crc value
  * @param	buf: pointer to the buffer array to encode
  * @param	len: number of bytes to encode
  * @retval	None
  */
uint32_t crc32_accumulate(uint32_t crc, QUEUE_Typedef *queue, uint32_t offset, uint32_t len)
{
    int k;

    crc = ~crc;
    while (len--) {
        crc ^= queue->pBuff[QUEUE_PTRLOOP(queue, offset++)];
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return ~crc;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Parse for packet
  * @param	queue: Queue from which to remove the packet
  * @param[out]	packet: pointer to the returned packet when valid
  * @retval	BPKT_STATUS_ENUM
  */
BPKT_STATUS_ENUM PKT_Decode(QUEUE_Typedef *queue, BPKT_Packet_TD *packet)
{
    if(QUEUE_COUNT(queue) < BPKT_OVERHEAD)
        return BPKT_NOTENOUGHDATA;

    if(QUEUE_ElementAt(queue, 0) != 0x02)
        return BPKT_STX;

    uint32_t calccrc = 0;
    calccrc = crc32_accumulate(calccrc, queue, queue->out, 3);
    uint32_t lclcrc = QUEUE_TOU32(queue, queue->out + 3);
    if(lclcrc != calccrc)
        return BPKT_HCRC;

    uint16_t length = QUEUE_ElementAt(queue, 1);
    length += (QUEUE_ElementAt(queue, 2) << 8);
    if((length > BPKT_MAXDATALENGTH) || (length == 0))
        return BPKT_LENGTH;

    if(QUEUE_COUNT(queue) < BPKT_PACKETSIZE(length))
        return BPKT_NOTENOUGHDATA;

    if(QUEUE_ElementAt(queue, BPKT_PACKETSIZE(length) - 1) !=  0x03)
        return BPKT_ETX;

    calccrc = 0;
    calccrc = crc32_accumulate(calccrc, queue, queue->out + 7, length);
    lclcrc = QUEUE_TOU32(queue, queue->out + BPKT_PACKETSIZE(length) - 5);
    if(lclcrc != calccrc)
        return BPKT_DCRC;

    //All good now
    QUEUE_ReadToArray(queue, 7, packet->data, length);
    packet->length = length;
    return BPKT_OK;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Parse for packet
  * @param	queue: Queue from which to remove the packet
  * @param[out]	packet: pointer to the returned packet when valid
  * @retval	BPKT_STATUS_ENUM
  */
BPKT_STATUS_ENUM PKT_Encode(uint8_t *data, uint16_t length, QUEUE_Typedef *queue)
{
    if(QUEUE_SPACE(queue) < BPKT_PACKETSIZE(length))
        return BPKT_NOTENOUGHSPACE;
    if(length > BPKT_MAXDATALENGTH)
    	return BPKT_EXCEEDSMAXSIZE;

    uint32_t strt = queue->in;
    QUEUE_Add(queue, 0x02);

    QUEUE_Add(queue, (uint8_t)length);
    QUEUE_Add(queue, (uint8_t)(length >> 8));

    uint32_t calccrc = crc32_accumulate(0, queue, strt, 3);
    QUEUE_Add(queue, (uint8_t)calccrc);
    QUEUE_Add(queue, (uint8_t)(calccrc >> 8));
    QUEUE_Add(queue, (uint8_t)(calccrc >> 16));
    QUEUE_Add(queue, (uint8_t)(calccrc >> 24));

    strt = queue->in;
    QUEUE_AddArray(queue, data, length);

    calccrc = crc32_accumulate(0, queue, strt, length);
    QUEUE_Add(queue, (uint8_t)calccrc);
    QUEUE_Add(queue, (uint8_t)(calccrc >> 8));
    QUEUE_Add(queue, (uint8_t)(calccrc >> 16));
    QUEUE_Add(queue, (uint8_t)(calccrc >> 24));

    QUEUE_Add(queue, 0x03);
    return BPKT_OK;
}
