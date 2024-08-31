/**
  ******************************************************************************
  * @file     	usbPacketIDs.h
  * @author		beede
  * @version	1V0
  * @date		Jul 20, 2024
  * @brief
  */


#ifndef INC_USBPACKETIDS_H_
#define INC_USBPACKETIDS_H_

/* Includes ------------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
enum
{
    pktACK = 0,
    pktNACK = 1,

    pktDeviceInfo = 2,

	pktFileHeaderRequest = 12,
	pktFileHeader = 13,

	pktUSRDataRequest = 14,
	pktUSRData = 15,
	pktUSRClose = 16,
	pktUSRAlive = 17,

};

/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* INC_USBPACKETIDS_H_ */
