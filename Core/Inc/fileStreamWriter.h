/**
  ******************************************************************************
  * @file     	fileStreamWriter.h
  * @author		beede
  * @version	1V0
  * @date		Jul 27, 2024
  * @brief
  */


#ifndef INC_FILESTREAMWRITER_H_
#define INC_FILESTREAMWRITER_H_

/* Includes ------------------------------------------------------------------*/
#include "bStream.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void FSW_tick(void);
BSTREAM_Enum FSW_WriteFile(BSTREAM_Reader_td *stream);

#endif /* INC_FILESTREAMWRITER_H_ */
