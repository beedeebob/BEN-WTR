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
#include "espUsartEncoding.h"
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
static uint32_t crc32_accumulate(uint32_t crc, ESPQ_Typedef *queue, uint32_t offset, uint32_t len)
{
    int k;

    crc = ~crc;
    while (len--) {
        crc ^= queue->pBuff[ESPQ_PTRLOOP(queue, offset++)];
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
  * @retval	ESPPKT_DECODEEnum
  */
ESPPKT_DECODEEnum ESPPKT_Decode(ESPQ_Typedef *queue, ESPPKT_RxPacket_TD *packet)
{
    if(ESPQ_COUNT(queue) < ESPPKT_OVERHEAD)
        return ESPPKT_NOTENOUGHDATA;

    if(ESPQ_ElementAt(queue, 0) != 0x02)
        return ESPPKT_STX;

    uint32_t calccrc = 0;
    calccrc = crc32_accumulate(calccrc, queue, queue->out, 3);
    uint32_t lclcrc = ESPQ_TOU32(queue, queue->out + 3);
    if(lclcrc != calccrc)
        return ESPPKT_HCRC;

    uint16_t length = ESPQ_ElementAt(queue, 1);
    length += (ESPQ_ElementAt(queue, 2) << 8);
    if((length > ESPPKT_MAXDATALENGTH) || (length == 0))
        return ESPPKT_LENGTH;

    if(ESPQ_COUNT(queue) < ESPPKT_PACKETSIZE(length))
        return ESPPKT_NOTENOUGHDATA;

    if(ESPQ_ElementAt(queue, ESPPKT_PACKETSIZE(length) - 1) !=  0x03)
        return ESPPKT_ETX;

    calccrc = 0;
    calccrc = crc32_accumulate(calccrc, queue, queue->out + 7, length);
    lclcrc = ESPQ_TOU32(queue, queue->out + ESPPKT_PACKETSIZE(length) - 5);
    if(lclcrc != calccrc)
        return ESPPKT_DCRC;

    //All good now
    ESPQ_ReadToArray(queue, 7, packet->data, length);
    packet->length = length;
    return ESPPKT_OK;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Parse for packet
  * @param	queue: Queue from which to remove the packet
  * @param[out]	packet: pointer to the returned packet when valid
  * @retval	ESPPKT_STATUS_ENUM
  */
ESP_Result ESPPKT_Encode(uint8_t *data, uint16_t length, ESPQ_Typedef *queue)
{
    if(ESPQ_SPACE(queue) < ESPPKT_PACKETSIZE(length))
        return ESP_NOSPACE;
    if(length > ESPPKT_MAXDATALENGTH)
    	return ESP_LIMITSEXCEEDED;

    uint32_t strt = queue->in;
    ESPQ_Add(queue, 0x02);

    ESPQ_Add(queue, (uint8_t)length);
    ESPQ_Add(queue, (uint8_t)(length >> 8));

    uint32_t calccrc = crc32_accumulate(0, queue, strt, 3);
    ESPQ_Add(queue, (uint8_t)calccrc);
    ESPQ_Add(queue, (uint8_t)(calccrc >> 8));
    ESPQ_Add(queue, (uint8_t)(calccrc >> 16));
    ESPQ_Add(queue, (uint8_t)(calccrc >> 24));

    strt = queue->in;
    ESPQ_AddArray(queue, data, length);

    calccrc = crc32_accumulate(0, queue, strt, length);
    ESPQ_Add(queue, (uint8_t)calccrc);
    ESPQ_Add(queue, (uint8_t)(calccrc >> 8));
    ESPQ_Add(queue, (uint8_t)(calccrc >> 16));
    ESPQ_Add(queue, (uint8_t)(calccrc >> 24));

    ESPQ_Add(queue, 0x03);
    return ESP_OK;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Begin encoding a packet
  * @param	queue: Queue into which the data is written
  * @param	totallength: total length of the packet
  * @param	partData: pointer to the part of the data
  * @param	partlength: length of the data part
  * @param	[out]crc: pointer to the returned CRC value
  * @retval	ESP_Result
  */
ESP_Result ESPPKT_EncodeStart(ESPQ_Typedef *queue, uint16_t totallength, uint8_t *partData, uint16_t partlength, uint32_t *crc)
{
    if(ESPQ_SPACE(queue) < ESPPKT_PACKETSIZE(totallength))
        return ESP_NOSPACE;
    if(totallength > ESPPKT_MAXDATALENGTH)
    	return ESP_LIMITSEXCEEDED;

    uint32_t strt = queue->in;
    ESPQ_Add(queue, 0x02);

    ESPQ_Add(queue, (uint8_t)totallength);
    ESPQ_Add(queue, (uint8_t)(totallength >> 8));

    uint32_t calccrc = crc32_accumulate(0, queue, strt, 3);
    ESPQ_Add(queue, (uint8_t)calccrc);
    ESPQ_Add(queue, (uint8_t)(calccrc >> 8));
    ESPQ_Add(queue, (uint8_t)(calccrc >> 16));
    ESPQ_Add(queue, (uint8_t)(calccrc >> 24));

    strt = queue->in;
    ESPQ_AddArray(queue, partData, partlength);
    *crc = crc32_accumulate(0, queue, strt, partlength);

    return ESP_OK;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Encode another segment of a packet
  * @param	queue: Queue into which the data is written
  * @param	partData: pointer to the part of the data
  * @param	partlength: length of the data part
  * @param	[out]crc: pointer to the returned CRC value
  * @retval	ESP_Result
  */
ESP_Result ESPPKT_EncodePart(ESPQ_Typedef *queue, uint8_t *partData, uint16_t partlength, uint32_t *crc)
{
	uint32_t strt = queue->in;
    ESPQ_AddArray(queue, partData, partlength);

    *crc = crc32_accumulate(*crc, queue, strt, partlength);
    return ESP_OK;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Parse for packet
  * @param	queue: Queue into which the data is written
  * @param	data: pointer to the part of the data
  * @param	length: length of the data part
  * @param	[out]crc: pointer to the returned CRC value
  * @retval	ESP_Result
  */
ESP_Result ESPPKT_EncodeEnd(ESPQ_Typedef *queue, uint8_t *data, uint16_t length, uint32_t *crc)
{
	uint32_t strt = queue->in;
    ESPQ_AddArray(queue, data, length);

    uint32_t calccrc = crc32_accumulate(*crc, queue, strt, length);
    ESPQ_Add(queue, (uint8_t)calccrc);
    ESPQ_Add(queue, (uint8_t)(calccrc >> 8));
    ESPQ_Add(queue, (uint8_t)(calccrc >> 16));
    ESPQ_Add(queue, (uint8_t)(calccrc >> 24));

    ESPQ_Add(queue, 0x03);
    return ESP_OK;
}
