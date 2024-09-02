/**
  ******************************************************************************
  * @file     	queue.c
  * @author		beede
  * @version	1V0
  * @date		Nov 25, 2023
  * @brief
  */


/* Includes ------------------------------------------------------------------*/
#include "benQueue.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Initialize the queue system
  * @param	queue: pointer to the queue struct to initialize
  * @param	pBuff: pointer to the buffer array to use
  * @param	size: size of the buffer. Must be a power of 2
  * @retval	None
  */
void ESPQ_Initialize(ESPQ_Typedef *queue, uint8_t *pBuff, uint32_t size)
{
	queue->pBuff = pBuff;
	queue->size = size;
	queue->in = 0;
	queue->out = 0;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Add a byte to the queue
  * @param	queue: pointer to the queue struct
  * @param	data: byte to add
  * @retval ESPQ_STATUS
  */
ESPQ_STATUS ESPQ_Add(ESPQ_Typedef *queue, uint8_t data)
{
	if(ESPQ_SPACE(queue) == 0)
		return ESPQ_NOSPACE;

	queue->pBuff[queue->in] = data;
	queue->in = ESPQ_PTRLOOP(queue, queue->in + 1);

	return ESPQ_OK;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Add data from an array to the queue
  * @param	queue: pointer to the queue struct
  * @param	data: pointer to data array to add
  * @param	length: amount of data to add
  * @retval ESPQ_STATUS
  */
ESPQ_STATUS ESPQ_AddArray(ESPQ_Typedef *queue, uint8_t *data, uint32_t length)
{
	if(ESPQ_SPACE(queue) < length)
		return ESPQ_NOSPACE;

	for(uint32_t i = 0; i < length; i++)
		queue->pBuff[ESPQ_PTRLOOP(queue, queue->in + i)] = data[i];
	queue->in = ESPQ_PTRLOOP(queue, queue->in + length);

	return ESPQ_OK;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Add data from an array to the queue
  * @param	queue: pointer to the queue struct
  * @param	data: pointer to data array to add
  * @param	length: amount of data to add
  * @retval ESPQ_STATUS
  */
ESPQ_STATUS ESPQ_AddQueue(ESPQ_Typedef *queue, ESPQ_Typedef *data, uint32_t length)
{
	if(ESPQ_SPACE(queue) < length)
		return ESPQ_NOSPACE;
	if(ESPQ_COUNT(data) < length)
		return ESPQ_NOTENOUGHDATA;

	for(uint32_t i = 0; i < length; i++)
		queue->pBuff[ESPQ_PTRLOOP(queue, queue->in + i)] = data->pBuff[ESPQ_PTRLOOP(data, data->out + i)];

	queue->in = ESPQ_PTRLOOP(queue, queue->in + length);
	data->out = ESPQ_PTRLOOP(data, data->out + length);

	return ESPQ_OK;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Get the byte at the out pointer referenced index location
  * @param	queue: pointer to the queue struct
  * @param	index: index to read with reference to the out pointer
  * @retval value or 0xff if the index exceeds the data
  */
uint8_t ESPQ_ElementAt(ESPQ_Typedef *queue, uint32_t index)
{
	if(index >= ESPQ_COUNT(queue))
		return 0xff;

	return queue->pBuff[ESPQ_PTRLOOP(queue, (index + queue->out))];
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Read the next byte from the queue and remove it
  * @param	queue: pointer to the queue struct
  * @retval value of 0xff if there is not data
  */
uint8_t ESPQ_ReadOutByte(ESPQ_Typedef *queue)
{
	if(ESPQ_COUNT(queue) == 0)
		return 0xff;

	uint8_t byte = queue->pBuff[queue->out];
	queue->out = ESPQ_PTRLOOP(queue, (queue->out + 1));
	return byte;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Read and remove the a number of bytes from the queue to an array
  * @param	queue: pointer to the queue from which to read
  * @param	data: pointer to the data into which to read
  * @param	length: amount of data to read out
  * @retval ESPQ_STATUS
  */
ESPQ_STATUS ESPQ_ReadOutArray(ESPQ_Typedef *queue, uint8_t *data, uint32_t length)
{
	if(ESPQ_COUNT(queue) < length)
		return ESPQ_NOTENOUGHDATA;

	for(uint32_t i = 0; i < length; i++)
		data[i] = queue->pBuff[ESPQ_PTRLOOP(queue, (queue->out + i))];
	queue->out = ESPQ_PTRLOOP(queue, (queue->out + length));

	return ESPQ_OK;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Read and remove the a number of bytes from the queue to another queue
  * @param	queue: pointer to the queue from which to read
  * @param	data: pointer to the queue into which to read
  * @param	length: amount of data to read out
  * @retval ESPQ_STATUS
  */
ESPQ_STATUS ESPQ_ReadOutQueue(ESPQ_Typedef *queue, ESPQ_Typedef *data, uint32_t length)
{
	if(ESPQ_COUNT(queue) < length)
		return ESPQ_NOTENOUGHDATA;
	if(ESPQ_SPACE(data) < length)
		return ESPQ_NOSPACE;

	for(uint32_t i = 0; i < length; i++)
		data->pBuff[ESPQ_PTRLOOP(data, (data->in + i))] = queue->pBuff[ESPQ_PTRLOOP(queue, (queue->out + i))];
	queue->out = ESPQ_PTRLOOP(queue, (queue->out + length));
	data->in = ESPQ_PTRLOOP(data, (data->in + length));

	return ESPQ_OK;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Read a number of bytes from the queue to an array
  * @param	queue: pointer to the queue from which to read
  * @param	data: pointer to the data into which to read
  * @param	length: amount of data to read out
  * @retval ESPQ_STATUS
  */
ESPQ_STATUS ESPQ_ReadToArray(ESPQ_Typedef *queue, uint32_t offset, uint8_t *data, uint32_t length)
{
	for(uint32_t i = 0; i < length; i++)
		data[i] = queue->pBuff[ESPQ_PTRLOOP(queue, (queue->out + offset + i))];

	return ESPQ_OK;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Remove bytes off the front of the queue
  * @param	count: number of bytes to remove
  * @retval value
  */
ESPQ_STATUS ESPQ_Remove(ESPQ_Typedef *queue, uint32_t count)
{
	if(ESPQ_COUNT(queue) < count)
		return ESPQ_PARAM;

	queue->out = ESPQ_PTRLOOP(queue, (queue->out + count));
	return ESPQ_OK;
}
