/**
  ******************************************************************************
  * @file     	espDefs.h
  * @author		beede
  * @version	1V0
  * @date		Sep 2, 2024
  * @brief
  */


#ifndef ESP01_ESPDEFS_H_
#define ESP01_ESPDEFS_H_

/* Includes ------------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
#define ESPPKT_MAXDATALENGTH      		100
#if (ESPCOMMS_BUFFERSIZE >= (ESPPKT_MAXDATALENGTH + 12))
#error ESPCOMMS_BUFFERSIZE should be large enough to hold ESPPKT_MAXDATALENGTH + 12
#endif

#define ESPCOMMS_BUFFERSIZE				128

/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* ESP01_ESPDEFS_H_ */
