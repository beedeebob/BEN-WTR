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
#include "cmsis_os.h"
#include "main.h"
#include "comms.h"
#include "espPktIds.h"
#include "led.h"
#include "weather.h"

/* Private define ------------------------------------------------------------*/
#define ESP_TIMEOUT					5000

#define ESP_TICKSINIT()				uint32_t startTicks = HAL_GetTick()
#define ESP_TICKSPASSED				(HAL_GetTick() - startTicks)
#define ESP_TICKSLEFT(TO)			((ESP_TICKSPASSED > (TO)) ? 0 : ((TO) - ESP_TICKSPASSED))
enum
{
	ESP_EVENT_WIFIACK = 0x01,
	ESP_EVENT_MQTTACK = 0x02,
	ESP_EVENT_WIFIOK = 0x04,
	ESP_EVENT_MQTTOK = 0x08,
	ESP_EVENT_WTHRACK = 0x10,
	ESP_EVENT_WTHROK= 0x20,
};
typedef enum
{
	ESP_ERR_OK = 0,
	ESP_ERR_TIMEOUT = -1
}ESP_ERR;

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t txBuffer[30];

/* Private function prototypes -----------------------------------------------*/
static void ESP_task(ESP_td *esp);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief	ESP initialize
  * @param	None
  * @retval	None
  */
void ESP_Init(void)
{

}

/*----------------------------------------------------------------------------*/
/**
  * @brief	ESP control millisecond tick
  * @param	None
  * @retval	None
  */
static void ESP_millisecond(void)
{
	ESP_task(&esp);
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	ESP control tick
  * @param	esp: pointer to the esp
  * @retval	None
  */
enum
{
	ESP_STATE_INIT = 0,
	ESP_STATE_AWAITSTABLEPOWER,
	ESP_STATE_IDLE,

};
static void ESP_task(ESP_td *esp)
{
	//Timer
	if(esp->tmr)
		esp->tmr--;

	//State machine
	switch(esp->state)
	{
	case ESP_STATE_INIT:
		//Hardware power up
		HAL_GPIO_WritePin(GPIO_ESP_FLASH_GPIO_Port, GPIO_ESP_FLASH_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIO_ESP_NPD_EN_GPIO_Port, GPIO_ESP_NPD_EN_Pin, GPIO_PIN_SET);
		esp->tmr = 50;
		esp->state = ESP_STATE_AWAITSTABLEPOWER;
		break;

	case ESP_STATE_AWAITSTABLEPOWER:
		if(esp->tmr != 0)
			break;
		esp->state = ESP_STATE_IDLE;

	case ESP_STATE_IDLE:
		if(esp->hasWeatherUpdate)
		{
			uint8_t len = 0;
			txBuffer[len++] = espPkt_WifiConnect;
			if(ESP_SendPacket(txBuffer, len) == HAL_OK)
		}
		break;
	}

	osDelay(50);	//TODO: Verify time necessity

	//Start Wifi
	while(ESP_TICKSLEFT(ESP_TIMEOUT) > 0)
	{

			osEventFlagsWait(espEvents, ESP_EVENT_WIFIACK, 0, 100);
		else
			osDelay(10);
	}

	//Start MQTT
	while(ESP_TICKSLEFT(ESP_TIMEOUT) > 0)
	{
		uint8_t len = 0;
		txBuffer[len++] = espPkt_StartMQTT;
		if(ESP_SendPacket(txBuffer, len) == HAL_OK)
			osEventFlagsWait(espEvents, ESP_EVENT_MQTTOK, 0, 100);
		else
			osDelay(10);
	}

	//Wait for wifi and mqtt
	osEventFlagsWait(espEvents, (ESP_EVENT_WIFIOK | ESP_EVENT_MQTTOK), osFlagsWaitAll, ESP_TICKSLEFT(ESP_TIMEOUT));

	//Wait for weather update
	uint32_t flags = osEventFlagsWait(systemEvents, SYS_EVENT_WEATHER | SYS_EVENT_WEATHERCMPLT, osFlagsNoClear | osFlagsWaitAny, ESP_TICKSLEFT(ESP_TIMEOUT));
	if(!(flags & SYS_EVENT_WEATHER))
	{
		//Hardware shutdown
		HAL_GPIO_WritePin(GPIO_ESP_NFLASH_GPIO_Port, GPIO_ESP_NFLASH_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIO_ESP_EN_GPIO_Port, GPIO_ESP_EN_Pin, GPIO_PIN_RESET);

		osEventFlagsSet(systemEvents, SYS_EVENT_ESPCMPLT);
		osThreadTerminate(espTaskHandle);
		return;
	}

	//Transmit weather to ESP
	while(ESP_TICKSLEFT(ESP_TIMEOUT) > 0)
	{
		uint8_t len = 0;
		txBuffer[len++] = espPkt_SetWeather;
		txBuffer[len++] = (uint8_t)(WTHR_Temperature);
		txBuffer[len++] = (uint8_t)(WTHR_Temperature >> 8);
		txBuffer[len++] = (uint8_t)(WTHR_Temperature >> 16);
		txBuffer[len++] = (uint8_t)(WTHR_Temperature >> 24);
		txBuffer[len++] = (uint8_t)(WTHR_Pressure);
		txBuffer[len++] = (uint8_t)(WTHR_Pressure >> 8);
		txBuffer[len++] = (uint8_t)(WTHR_Pressure >> 16);
		txBuffer[len++] = (uint8_t)(WTHR_Pressure >> 24);
		txBuffer[len++] = (uint8_t)(WTHR_Humidity);
		txBuffer[len++] = (uint8_t)(WTHR_Humidity >> 8);
		txBuffer[len++] = (uint8_t)(WTHR_Humidity >> 16);
		txBuffer[len++] = (uint8_t)(WTHR_Humidity >> 24);
		if(ESP_SendPacket(txBuffer, len) == HAL_OK)
			osEventFlagsWait(espEvents, ESP_EVENT_WTHRACK, 0, 100);
		else
			osDelay(10);
	}

	//Wait for publish
	osEventFlagsWait(espEvents, ESP_EVENT_WTHROK, 0, ESP_TICKSLEFT(ESP_TIMEOUT));

	//Hardware shutdown
	HAL_GPIO_WritePin(GPIO_ESP_NFLASH_GPIO_Port, GPIO_ESP_NFLASH_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO_ESP_EN_GPIO_Port, GPIO_ESP_EN_Pin, GPIO_PIN_RESET);

	//Publish Complete
	if(flags & ESP_EVENT_WTHROK)
	{
		osEventFlagsSet(systemEvents, SYS_EVENT_MQTTPUBLISHED);

		//Flash LED
		LED_ON();
		osDelay(20);
		LED_OFF();
	}

	//Wait for weather update
	osEventFlagsSet(systemEvents, SYS_EVENT_ESPCMPLT);
	osThreadTerminate(espTaskHandle);
	return;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	ESP state control system
  * @param	None
  * @retval	None
  */
void ESP_ESPCOMMSHandler(uint8_t *data, uint32_t length)
{
	switch(data[0])
	{
	case espPkt_ACK:
		if(data[1] == espPkt_WifiConnect)
			osEventFlagsSet(espEvents, ESP_EVENT_WIFIACK);
		else if(data[1] == espPkt_StartMQTT)
			osEventFlagsSet(espEvents, ESP_EVENT_MQTTACK);
		else if (data[1] == espPkt_SetWeather)
			osEventFlagsSet(espEvents, ESP_EVENT_WTHRACK);
		break;
	case espPkt_Status:
		if(data[1] & 0x01)
			osEventFlagsSet(espEvents, ESP_EVENT_WIFIOK);
		else
			osEventFlagsClear(espEvents, ESP_EVENT_WIFIOK);
		if(data[1] & 0x02)
			osEventFlagsSet(espEvents, ESP_EVENT_MQTTOK);
		else
			osEventFlagsClear(espEvents, ESP_EVENT_MQTTOK);
		if(data[1] & 0x04)
			osEventFlagsSet(espEvents, ESP_EVENT_WTHROK);
		break;
	}
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Post weather readings
  * @param	pressure
  * @param	temperature
  * @param	humidity
  * @retval	ESP_Result
  */
ESP_Result ESP_PostWeatherUpdate(ESP_td *esp, uint32_t pressure, int32_t temperature, uint32_t humidity)
{
	if(esp->hasWeatherUpdate)
		return ESP_BUSY;

	esp->pressure = pressure;
	esp->temperature = temperature;
	esp->humidity = humidity;
	esp->hasWeatherUpdate = true;
	return ESP_OK;
}
