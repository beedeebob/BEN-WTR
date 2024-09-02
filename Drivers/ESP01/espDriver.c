/**
  ******************************************************************************
  * @file     	espDriver.c
  * @author		ben
  * @version	1V0
  * @date		Aug 31, 2024
  * @brief		ST interface driver for the esp01 control
  */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "esp.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern ESP_td esp;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Transmit a USART packet
  * @param	data: pointer to the data to transmit
  * @param	size: amount of data to transmit
  * @retval	ESP_Result
  */
ESP_Result USART_Transmit(ESP_td *esp, uint8_t *data, uint32_t size)
{
	//TODO: Configure USART DMA
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Handle USART packet recieved
  * @param	data: pointer to the data to transmit
  * @param	size: amount of data to transmit
  * @retval
  */
void USART_ReceiveHandler(uint8_t *data, uint32_t size)
{
	ESP_PacketDataReceive(&esp, data, size);
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Packet received callback handler
  * @param	esp: pointer to the esp system
  * @param	packet: pointer to the received packet
  * @retval	None
  */
void ESP_PacketReceivedHandler(ESP_td *esp, ESPPKT_RxPacket_TD *packet)
{
	//TODO: Handle packet received
}
