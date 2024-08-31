/**
  ******************************************************************************
  * @file     	fileStreamWriter.c
  * @author		beede
  * @version	1V0
  * @date		Jul 27, 2024
  * @brief
  */


/* Includes ------------------------------------------------------------------*/
#include "fileStreamWriter.h"
#include "stddef.h"
#include "stdint.h"
#include "utils.h"
#include "assert.h"

/* Private define ------------------------------------------------------------*/


//STATES
enum
{
	FSW_STATE_IDLE = 0,
	FSW_STATE_WRITESTART,
	FSW_STATE_WRITE,
	FSW_STATE_WRITECOMPLETE,
};

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static BSTREAM_Reader_td *fileStream;
static uint8_t state;
static uint32_t offset;
static uint32_t crc;
static uint8_t data[256];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Tick routine
  * @param	None
  * @retval	None
  */
void FSW_tick(void)
{
	switch(state)
	{
	case FSW_STATE_IDLE:
		break;

	case FSW_STATE_WRITESTART:
		fileStream->open(fileStream);
		state = FSW_STATE_WRITE;

	case FSW_STATE_WRITE:

		uint32_t count = 0;
		BSTREAM_Enum result = fileStream->count(fileStream, offset, &count);
		if(result == BSTREAM_CLOSED)
		{
			state = FSW_STATE_WRITECOMPLETE;
			break;
		}

		if(count == 0)
			break;

		if(count > sizeof(data))
			count = sizeof(data);
		fileStream->readData(fileStream, offset, data, count, NULL);
		crc = crc32_calculateData(crc, data, 0, count);

		offset += count;
		if(offset >= fileStream->length)
			state = FSW_STATE_WRITECOMPLETE;
		else
			break;

	case FSW_STATE_WRITECOMPLETE:
		assert(crc == fileStream->crc);
 		fileStream->close(fileStream);
		fileStream = NULL;
		state = FSW_STATE_IDLE;
		break;
	}
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Write file to flash
  * @param	stream: Stream from which to read the data
  * @retval	BSTREAM_Enum
  */
BSTREAM_Enum FSW_WriteFile(BSTREAM_Reader_td *stream)
{
	if(fileStream != NULL)
		return (stream == fileStream) ? BSTREAM_OK : BSTREAM_BUSY;

	fileStream = stream;
	offset = 0;
	crc = 0;
	state = FSW_STATE_WRITESTART;
	return BSTREAM_OK;
}
