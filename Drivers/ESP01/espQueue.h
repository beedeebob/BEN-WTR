/**
  ******************************************************************************
  * @file     	queue.h
  * @author		beede
  * @version	1V0
  * @date		Nov 25, 2023
  * @brief
  */


#ifndef ESPQ_H_
#define ESPQ_H_

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* Exported defines ----------------------------------------------------------*/
#define ESPQ_COUNT(Q)					(((Q)->in - (Q)->out) & ((Q)->size  - 1))
#define ESPQ_SPACE(Q)					((Q)->size - 1 - ESPQ_COUNT(Q))
#define ESPQ_PTRLOOP(Q, PTR)			((PTR) & (Q->size - 1))
#define ESPQ_TOU32(Q, OFFSET)			((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET)] + ((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET + 1)] << 8) + ((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET + 2)] << 16)  + ((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET + 3)] << 24) )
#define ESPQ_TOU24(Q, OFFSET)			((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET)] + ((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET + 1)] << 8) + ((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET + 2)] << 16))
#define ESPQ_TOU16(Q, OFFSET)			((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET)] + ((Q)->pBuff[ESPQ_PTRLOOP((Q), OFFSET + 1)] << 8))

/* Exported types ------------------------------------------------------------*/
typedef struct
{
	uint8_t *pBuff;
	uint32_t size;		//Must be a power of 2
	uint32_t in;
	uint32_t out;
}ESPQ_Typedef;

typedef enum
{
	ESPQ_OK = 0,
	ESPQ_NOSPACE = -1,
	ESPQ_PARAM = -2,
	ESPQ_NOTENOUGHDATA = -3
}ESPQ_STATUS;

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ESPQ_Initialize(ESPQ_Typedef *queue, uint8_t *pBuff, uint32_t size);
ESPQ_STATUS ESPQ_Add(ESPQ_Typedef *queue, uint8_t data);
ESPQ_STATUS ESPQ_AddArray(ESPQ_Typedef *queue, uint8_t *data, uint32_t length);
ESPQ_STATUS ESPQ_AddQueue(ESPQ_Typedef *queue, ESPQ_Typedef *data, uint32_t length);
uint8_t ESPQ_ElementAt(ESPQ_Typedef *queue, uint32_t index);
uint8_t ESPQ_ReadOutByte(ESPQ_Typedef *queue);
ESPQ_STATUS ESPQ_ReadOutArray(ESPQ_Typedef *queue, uint8_t *data, uint32_t length);
ESPQ_STATUS ESPQ_ReadOutQueue(ESPQ_Typedef *queue, ESPQ_Typedef *data, uint32_t length);
ESPQ_STATUS ESPQ_ReadToArray(ESPQ_Typedef *queue, uint32_t offset, uint8_t *data, uint32_t length);
ESPQ_STATUS ESPQ_Remove(ESPQ_Typedef *queue, uint32_t count);

#endif /* BUFFER_H_ */
