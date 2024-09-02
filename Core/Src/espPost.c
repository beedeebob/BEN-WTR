/**
  ******************************************************************************
  * @file     	espPost.c
  * @author		beede
  * @version	1V0
  * @date		Sep 2, 2024
  * @brief
  */
/**
  ******************************************************************************
  * @file     	esp.c
  * @author		ben
  * @version	1V0
  * @date		Mar 5, 2024
  * @brief
  */


/* Includes ------------------------------------------------------------------*/
#include "espPost.h"
#include "esp.h"
#include "espPktIds.h"
#include "stdbool.h"
#include "main.h"

#include "led.h"
#include "weather.h"

/* Private define ------------------------------------------------------------*/
#define WTRPST_TIMEOUT					5000

#define WTRPST_TICKSINIT()				uint32_t startTicks = HAL_GetTick()
#define WTRPST_TICKSPASSED				(HAL_GetTick() - startTicks)
#define WTRPST_TICKSLEFT(TO)			((WTRPST_TICKSPASSED > (TO)) ? 0 : ((TO) - WTRPST_TICKSPASSED))
enum
{
	WTRPST_EVENT_WIFIACK = 0x01,
	WTRPST_EVENT_MQTTACK = 0x02,
	WTRPST_EVENT_WIFIOK = 0x04,
	WTRPST_EVENT_MQTTOK = 0x08,
	WTRPST_EVENT_WTHRACK = 0x10,
	WTRPST_EVENT_WTHROK= 0x20,
};
typedef enum
{
	WTRPST_ERR_OK = 0,
	WTRPST_ERR_TIMEOUT = -1
}WTRPST_ERR;
typedef struct
{
	bool post:1;							//Set when weather update should be posted
	bool wifiConnectACK:1;					//Set when wifi connected command has been received
	bool wifiConnected:1;					//Set when wifi has connected
	bool mqttConnectACK:1;					//Set when mqtt connection command has been received
	bool mqttConnected:1;					//Set when mqtt connected and ready
	bool weatherPostACK:1;					//Set when weather post is acknowledged
}WTRPST_Flags;

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ESP_td esp;

static uint8_t state = WTRPST_STATE_IDLE;
static uint8_t tmr;
static uint8_t txBuffer[30];
static WTRPST_Flags flags;


/* Private function prototypes -----------------------------------------------*/
static void WTRPST_task(WTRPST_td *esp);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief	ESP initialize
  * @param	None
  * @retval	None
  */
void WTRPST_Init(void)
{

}

/*----------------------------------------------------------------------------*/
/**
  * @brief	ESP control millisecond tick
  * @param	None
  * @retval	None
  */
void WTRPST_tick(void)
{
	WTRPST_task(&esp);
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	ESP control tick
  * @param	esp: pointer to the esp
  * @retval	None
  */
enum
{
	WTRPST_STATE_IDLE = 0,
	WTRPST_STATE_AWAITSTABLEPOWER,
	WTRPST_STATE_AWAITWIFICONNECTACK,
	WTRPST_STATE_AWAITWIFICONNECTED,
	WTRPST_STATE_AWAITMQTTCONNECTACK,
	WTRPST_STATE_AWAITMQTTCONNECTED,
	WTRPST_STATE_AWAITMQTTPOSTACK,
	WTRPST_STATE_SHUTDOWN,

};
static void WTRPST_task(WTRPST_td *esp)
{
	//Timer
	if(tmr)
		tmr--;

	//State machine
	switch(state)
	{
	case WTRPST_STATE_IDLE:
		if(!flags.post)
			break;
		flags.post = false;

		//Hardware power up
		HAL_GPIO_WritePin(GPIO_ESP_FLASH_GPIO_Port, GPIO_ESP_FLASH_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIO_ESP_NPD_EN_GPIO_Port, GPIO_ESP_NPD_EN_Pin, GPIO_PIN_SET);
		tmr = 50;
		state = WTRPST_STATE_AWAITSTABLEPOWER;
		break;

	case WTRPST_STATE_AWAITSTABLEPOWER:
		if(tmr != 0)
			break;

		if(ESP_Command(&esp, espPkt_WifiConnect, NULL, 0) == HAL_OK)
		{
			state = WTRPST_STATE_AWAITWIFICONNECTACK;
			tmr = 100;
		}
		break;

	case WTRPST_STATE_AWAITWIFICONNECTACK:
		if(flags.wifiConnectACK)
		{
			state = WTRPST_STATE_AWAITWIFICONNECTED;
			tmr = WTRPST_TIMEOUT;
		}
		else if (tmr == 0)
		{
			state = WTRPST_STATE_AWAITSTABLEPOWER;
		}
		break;

	case WTRPST_STATE_AWAITWIFICONNECTED:
		if(flags.wifiConnected)
		{
			if(ESP_Command(&esp, espPkt_StartMQTT, NULL, 0) == ESP_OK)
			{
				state = WTRPST_STATE_AWAITMQTTCONNECTACK;
				tmr = 100;
			}
		}
		else if (tmr == 0)
		{
			state = WTRPST_STATE_SHUTDOWN;
		}
		break;

	case WTRPST_STATE_AWAITMQTTCONNECTACK:
		if(flags.mqttConnectACK)
		{
			state = WTRPST_STATE_AWAITMQTTCONNECTED;
			tmr = WTRPST_TIMEOUT;
		}
		else if (tmr == 0)
		{
			state = WTRPST_STATE_AWAITWIFICONNECTED;
		}
		break;

	case WTRPST_STATE_AWAITMQTTCONNECTED:
		if(flags.mqttConnected)
		{
			uint8_t len = 0;
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
			if(ESP_Command(&esp, espPkt_SetWeather, txBuffer, len) == ESP_OK)
			{
				state = WTRPST_STATE_AWAITMQTTPOSTACK;
				tmr = 100;
			}
		}
		else if (tmr == 0)
		{
			state = WTRPST_STATE_SHUTDOWN;
		}
		break;

	case WTRPST_STATE_AWAITMQTTPOSTACK:
		if(flags.weatherPostACK)
		{
			state = WTRPST_STATE_SHUTDOWN;
		}
		else if (tmr == 0)
		{
			state = WTRPST_STATE_AWAITMQTTCONNECTED;
		}
		break;



	osDelay(50);	//TODO: Verify time necessity

	//Start Wifi
	while(WTRPST_TICKSLEFT(WTRPST_TIMEOUT) > 0)
	{

			osEventFlagsWait(espEvents, WTRPST_EVENT_WIFIACK, 0, 100);
		else
			osDelay(10);
	}

	//Start MQTT
	while(WTRPST_TICKSLEFT(WTRPST_TIMEOUT) > 0)
	{
		uint8_t len = 0;
		txBuffer[len++] = espPkt_StartMQTT;
		if(WTRPST_SendPacket(txBuffer, len) == HAL_OK)
			osEventFlagsWait(espEvents, WTRPST_EVENT_MQTTOK, 0, 100);
		else
			osDelay(10);
	}

	//Wait for wifi and mqtt
	osEventFlagsWait(espEvents, (WTRPST_EVENT_WIFIOK | WTRPST_EVENT_MQTTOK), osFlagsWaitAll, WTRPST_TICKSLEFT(WTRPST_TIMEOUT));

	//Wait for weather update
	uint32_t flags = osEventFlagsWait(systemEvents, SYS_EVENT_WEATHER | SYS_EVENT_WEATHERCMPLT, osFlagsNoClear | osFlagsWaitAny, WTRPST_TICKSLEFT(WTRPST_TIMEOUT));
	if(!(flags & SYS_EVENT_WEATHER))
	{
		//Hardware shutdown
		HAL_GPIO_WritePin(GPIO_WTRPST_NFLASH_GPIO_Port, GPIO_WTRPST_NFLASH_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIO_WTRPST_EN_GPIO_Port, GPIO_WTRPST_EN_Pin, GPIO_PIN_RESET);

		osEventFlagsSet(systemEvents, SYS_EVENT_ESPCMPLT);
		osThreadTerminate(espTaskHandle);
		return;
	}

	//Transmit weather to ESP
	while(WTRPST_TICKSLEFT(WTRPST_TIMEOUT) > 0)
	{

		else
			osDelay(10);
	}

	//Wait for publish
	osEventFlagsWait(espEvents, WTRPST_EVENT_WTHROK, 0, WTRPST_TICKSLEFT(WTRPST_TIMEOUT));

	//Hardware shutdown
	HAL_GPIO_WritePin(GPIO_WTRPST_NFLASH_GPIO_Port, GPIO_WTRPST_NFLASH_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO_WTRPST_EN_GPIO_Port, GPIO_WTRPST_EN_Pin, GPIO_PIN_RESET);

	//Publish Complete
	if(flags & WTRPST_EVENT_WTHROK)
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

	//Tick the communications
	ESPCOMMS_tick(esp);
	return;
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	ESP state control system
  * @param	None
  * @retval	None
  */
void WTRPST_ESPCOMMSHandler(uint8_t *data, uint32_t length)
{
	switch(data[0])
	{
	case espPkt_ACK:
		if(data[1] == espPkt_WifiConnect)
			osEventFlagsSet(espEvents, WTRPST_EVENT_WIFIACK);
		else if(data[1] == espPkt_StartMQTT)
			osEventFlagsSet(espEvents, WTRPST_EVENT_MQTTACK);
		else if (data[1] == espPkt_SetWeather)
			osEventFlagsSet(espEvents, WTRPST_EVENT_WTHRACK);
		break;
	case espPkt_Status:
		if(data[1] & 0x01)
			osEventFlagsSet(espEvents, WTRPST_EVENT_WIFIOK);
		else
			osEventFlagsClear(espEvents, WTRPST_EVENT_WIFIOK);
		if(data[1] & 0x02)
			osEventFlagsSet(espEvents, WTRPST_EVENT_MQTTOK);
		else
			osEventFlagsClear(espEvents, WTRPST_EVENT_MQTTOK);
		if(data[1] & 0x04)
			osEventFlagsSet(espEvents, WTRPST_EVENT_WTHROK);
		break;
	}
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Post weather readings
  * @param	pressure
  * @param	temperature
  * @param	humidity
  * @retval	WTRPST_Result
  */
WTRPST_Result WTRPST_PostWeatherUpdate(WTRPST_td *esp, uint32_t pressure, int32_t temperature, uint32_t humidity)
{
	if(esp->hasWeatherUpdate)
		return WTRPST_BUSY;

	esp->pressure = pressure;
	esp->temperature = temperature;
	esp->humidity = humidity;
	esp->hasWeatherUpdate = true;
	return WTRPST_OK;
}
