/**
  ******************************************************************************
  * @file     	led.h
  * @author		beede
  * @version	1V0
  * @date		Jul 15, 2024
  * @brief
  */


#ifndef INC_LED_H_
#define INC_LED_H_

/* Includes ------------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LED_SetBrightness(uint8_t pwm);
uint8_t LED_GetBrightness(void);

#endif /* INC_LED_H_ */
