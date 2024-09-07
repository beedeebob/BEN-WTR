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
#include "espDriver.h"
#include "espPost.h"
#include "espCommunications.h"
#include "stdbool.h"
#include "bQueue.h"
#include "string.h"

/* Private define ------------------------------------------------------------*/
#define ESPDRV_QUEUESIZE							256

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
	USART_TypeDef *usart;
	DMA_TypeDef *dma;
	uint32_t txDMAChan;
	uint32_t rxDMAChan;
	uint32_t lastRxDMAIndex;
	uint8_t txQBuff[ESPDRV_QUEUESIZE];
	uint8_t rxQBuff[ESPDRV_QUEUESIZE];
}ESP_Driver_td;

/* Private variables ---------------------------------------------------------*/
extern ESP_td esp;

static ESP_Driver_td espDriver;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Initialize the esp driver
  * @param	data: pointer to the data to transmit
  * @param	size: amount of data to transmit
  * @retval	Number transmitted
  */
void ESPDRV_Init(void)
{
	//Driver initialize
	memset(&espDriver, 0, sizeof(ESP_Driver_td));
	espDriver.usart = USART2;
	espDriver.dma = DMA1;
	espDriver.txDMAChan = LL_DMA_CHANNEL_7;
	espDriver.rxDMAChan = LL_DMA_CHANNEL_6;

	//USART RX DMA initialize
	LL_DMA_SetMemoryAddress(espDriver.dma, espDriver.rxDMAChan, (uint32_t)espDriver.rxQBuff);
	LL_DMA_SetPeriphAddress(espDriver.dma, espDriver.rxDMAChan, (uint32_t)&espDriver.usart->RDR);
	LL_DMA_SetDataLength(espDriver.dma, espDriver.rxDMAChan, sizeof(espDriver.rxDMAChan));
	LL_USART_EnableDMAReq_RX(espDriver.usart);
	LL_DMA_EnableChannel(espDriver.dma, espDriver.rxDMAChan);
	espDriver.lastRxDMAIndex = (sizeof(espDriver.rxDMAChan) - LL_DMA_GetDataLength(espDriver.dma, espDriver.rxDMAChan));

	//USART TX DMA initialize
	LL_DMA_SetMemoryAddress(espDriver.dma, espDriver.txDMAChan, (uint32_t)espDriver.txQBuff);
	LL_DMA_SetPeriphAddress(espDriver.dma, espDriver.txDMAChan, (uint32_t)&espDriver.usart->TDR);
	LL_USART_EnableDMAReq_TX(espDriver.usart);

	LL_USART_Enable(espDriver.usart);
}

/*----------------------------------------------------------------------------*/

/**
  * @brief	Millisecond tick routine
  * @param	None
  * @retval	None
  */
void ESPDRV_milli(void)
{
	//Handle the USART RX DMA
	uint32_t newDMAIndex = (sizeof(espDriver.rxDMAChan) - LL_DMA_GetDataLength(espDriver.dma, espDriver.rxDMAChan));
	uint32_t count = ((newDMAIndex - espDriver.lastRxDMAIndex) & (sizeof(espDriver.rxDMAChan) - 1));

	while(count > 0)
	{
		uint32_t contiguousDataSize = sizeof(espDriver.rxQBuff) - espDriver.lastRxDMAIndex;
		if(contiguousDataSize > count)
			contiguousDataSize = count;

		ESPCOMMS_PacketDataReceive(&esp, &espDriver.rxQBuff[espDriver.lastRxDMAIndex], contiguousDataSize);
		count -= contiguousDataSize;
		espDriver.lastRxDMAIndex = (espDriver.lastRxDMAIndex + contiguousDataSize) & (sizeof(espDriver.rxQBuff) - 1);
	}
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Transmit a USART packet
  * @param	data: pointer to the data to transmit
  * @param	size: amount of data to transmit
  * @retval	Number transmitted
  */
uint32_t USART_Transmit(ESP_td *esp, uint8_t *data, uint32_t size)
{
	//Check if transmit busy
	if(LL_DMA_IsEnabledChannel(espDriver.dma, espDriver.txDMAChan))
		return 0;

	//Calculate length to send
	uint32_t sizeToSend = (size > sizeof(espDriver.txQBuff)) ? sizeof(espDriver.txQBuff) : size;
	if(sizeToSend == 0)
		return 0;

	//Cnfigure the DMA
	LL_DMA_SetDataLength(espDriver.dma, espDriver.txDMAChan, sizeToSend);
	LL_DMA_ClearFlag_TC7(espDriver.dma);
	LL_DMA_EnableIT_TC(espDriver.dma, espDriver.txDMAChan);
	LL_DMA_EnableChannel(espDriver.dma, espDriver.txDMAChan);

	return sizeToSend;
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief	Packet received callback handler
  * @param	esp: pointer to the esp system
  * @param	packet: pointer to the received packet
  * @retval	None
  */
void ESP_PacketReceivedHandler(ESP_td *esp, uint8_t *data, uint32_t length)
{
	WTRPST_ESPCommandHandler(data, length);
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Handle DMA transfer complete
  * @param	dma: pointer to the DMA
  * @retval	None
  */
void ESPDRV_DMATransferCompleteHandler(DMA_TypeDef * dma)
{
	LL_DMA_DisableIT_TC(espDriver.dma, espDriver.txDMAChan);
	LL_DMA_DisableChannel(espDriver.dma, espDriver.txDMAChan);
}

