/**
  ******************************************************************************
  * @file     	led.c
  * @author		beede
  * @version	1V0
  * @date		Jul 15, 2024
  * @brief
  */


/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief
  * @param
  * @retval
  */
void LED_SetBrightness(uint8_t pwm)
{
	LL_TIM_OC_SetCompareCH1(TIM1, pwm);
}

/*----------------------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
uint8_t LED_GetBrightness(void)
{
	return LL_TIM_OC_GetCompareCH1(TIM1);
}
