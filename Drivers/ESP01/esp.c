/**
  ******************************************************************************
  * @file     	esp.c
  * @author		ben
  * @version	1V0
  * @date		Mar 5, 2024
  * @brief
  */


/* Includes ------------------------------------------------------------------*/
#include "esp.h"
#include "espPktIds.h"
#include "string.h"
#include "stddef.h"
#include "espUsartEncoding.h"
#include "espCommunications.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static ESP_td *espBase;

/* Private function prototypes -----------------------------------------------*/
static void ESP_task(ESP_td *esp);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief	ESP initialize
  * @param	esp: pointer to the esp control system
  * @retval	None
  */
void ESP_Initialize(ESP_td *esp)
{
	memset(esp, 0, sizeof(ESP_td));
	ESPCOMMS_structInit(&esp->comms);
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Start the esp control
  * @param	esp: pointer to the esp control system
  * @retval	ESP_Result
  */
ESP_Result ESP_Start(ESP_td *esp)
{
	//Check if already in list
	ESP_td *srch = espBase;
	while((srch != NULL) && (srch != esp))
		srch = srch->next;
	if(srch != NULL)
		return ESP_ALREADYEXISTS;

	//Add to stack
	esp->next = espBase;
	espBase = esp;
	return ESP_OK;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Close the ESP control
  * @param	esp: pointer to the esp control system
  * @retval	ESP_Result
  */
ESP_Result ESP_Close(ESP_td *esp)
{
	//Find previous element in chain
	ESP_td *srch = espBase;
	ESP_td *prev = NULL;
	while((srch != NULL) && (srch != esp))
	{
		prev = srch;
		srch = srch->next;
	}

	//Remove from stack
	if(srch == NULL)
		return ESP_OK;
	else if (prev == NULL)
		espBase = esp->next;
	else
		prev->next = esp->next;
	return ESP_OK;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	ESP control millisecond tick
  * @param	None
  * @retval	None
  */
void ESP_tick(void)
{
	if(espBase == NULL)
		return;

	ESP_td *esp = espBase;
	while(esp != NULL)
	{
		ESP_task(esp);
		esp = esp->next;
	}
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	ESP control tick
  * @param	esp: pointer to the esp
  * @retval	None
  */
static void ESP_task(ESP_td *esp)
{
	//Tick the communications
	ESPCOMMS_tick(esp);
	return;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Send ESP command
  * @param	esp: pointer to the esp
  * @retval	None
  */
ESP_Result ESP_Command(ESP_td *esp, uint16_t command, uint8_t *parameters, uint16_t parameterSize)
{
	if(ESPCOMMS_GetTransmitSpace(esp) < ESPPKT_PACKETSIZE(2 + parameterSize))
		return ESP_NOSPACE;

	return ESPCOMMS_Command(esp, command, parameters, parameterSize);
}
