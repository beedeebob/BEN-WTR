/**
  ******************************************************************************
  * @file     	espPktIds.h
  * @author		ben
  * @version	1V0
  * @date		Feb 29, 2024
  */

#ifndef ESPPKTIDS_H_
#define ESPPKTIDS_H_

/* Includes ------------------------------------------------------------------*/

/* Exported defines ----------------------------------------------------------*/
#define espPkt_SetWeather					1
/*
Set the weather values
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_SetWeather         Packet
1-4                                         Temperature in 100 C (little endian)
5-8                                         Pressure in Pa X 256 (little endian)
9-12                                        Humidity in % X 1024 (little endian)
*/

#define espPkt_WifiConnect                  2
/*
Connect to a wifi
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_WifiConnect        Packet
-                                           SSID string null terminated
-                                           Password string null terminated
*/

#define espPkt_SetMQTTBrokerHost            3
/*
Startup the MQTT
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_SetMQTTTopic       Packet
-                                           MQTT broker host null terminating string
*/

#define espPkt_SetMQTTBrokerLogin           4
/*
Startup the MQTT
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_SetMQTTTopic       Packet
-                                           MQTT broker username null terminating string
-                                           MQTT broker password null terminating string
*/

#define espPkt_SetMQTTTopic			        5
/*
Startup the MQTT
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_SetMQTTTopic       Packet
-                                           MQTT topic null terminating string
*/

#define espPkt_StartMQTT				    6
/*
Startup the MQTT
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_SetMQTTTopic       Packet
*/

#define espPkt_ACK				            7
/*
Startup the MQTT
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_ACK                Packet
1                                           Packet being acked
*/

#define espPkt_NACK				            8
/*
Startup the MQTT
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_NACK               Packet
1                                           Packet being nacked
*/

#define espPkt_StartWeb				        9
/*
Startup the Website
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_StartWeb           Packet
*/

#define espPkt_Status						10
/*
Startup the Website
BYTE    BIT       VALUE                     DESCRIPTION
0       -         espPkt_Status           	Packet
1		0x01								WIFI Ready
		0x02								MQTT Ready
		0x04								MQTT Published
*/

/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

#endif /* ESPPKTIDS_H_ */
