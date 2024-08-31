/**
  ******************************************************************************
  * @file     	weather.c
  * @author		beede
  * @version	1V0
  * @date		Jul 14, 2024
  * @brief
  */


/* Includes ------------------------------------------------------------------*/
#include "weather.h"
#include "bme280.h"
#include "led.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static	uint32_t pressure;
static	int32_t temperature;
static uint32_t humidity;

/* Private function prototypes -----------------------------------------------*/
static void WTR_ManageSensorReadings(void);
static void WTR_ManageLED(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Tick control routine
  * @param	None
  * @retval	None
  */
void WTR_milli(void)
{
	WTR_ManageSensorReadings();
	WTR_ManageLED();
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Manage starting and receiving the BME sensor updates
  * @param	None
  * @retval	None
  */
static void WTR_ManageSensorReadings(void)
{
	static BME_Access_td bmeAccess;
	static uint16_t sensorTmr = 0;
	if(sensorTmr)
		sensorTmr--;

	static uint8_t sensorState = 0;
	switch(sensorState)
	{
	case 0:
		BME_InitTypeDef setup;
		BME_initSetupStruct(&setup);
		setup.mode = BME_REG_CTRLMEAS_MODE_FORCED;
		if(BME_StartUp(&bmeAccess, &setup) == BME_ERROK)
		{
			sensorState = 1;
		}
		break;
	case 1:
		if((bmeAccess.complete != true) || (bmeAccess.result != BME_ERROK))
			break;

		if(sensorTmr > 0)
			break;

		if(BME_forcedReadOfSensors(&bmeAccess) == BME_ERROK)
		{
			sensorTmr = 5000;
			sensorState = 2;
		}
	case 2:
		if((bmeAccess.complete != true) || (bmeAccess.result != BME_ERROK))
			break;

		pressure = bmeAccess.pressure;
		temperature = bmeAccess.temperature;
		humidity = bmeAccess.humidity;

		sensorState = 1;
		break;
	}
}

/*----------------------------------------------------------------------------*/
/**
  * @brief	Manage LED
  * @param	None
  * @retval	None
  */
static void WTR_ManageLED(void)
{
	static uint16_t tmr;
	if(tmr)
		tmr--;

	if(tmr == 0)
	{
		tmr = 1000;
		LED_SetBrightness(5);
	}
	else if (tmr == 950)
	{
		LED_SetBrightness(0);
	}
}
