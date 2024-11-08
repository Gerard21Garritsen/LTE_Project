/*
 * A7670xx.h
 *
 *  Created on: Jul 23, 2024
 *      Author: garrix
 */

#ifndef INC_A7670XX_H_
#define INC_A7670XX_H_

#include "stm32f4xx_hal.h"
#include "string.h"


//Module functions

void A7670xx_setBaudRate(UART_HandleTypeDef *huart, char *baud);

void A7670xx_disableEcho(UART_HandleTypeDef *huart);

char* A7670xx_thereisSIMCard(UART_HandleTypeDef *huart);

char* A7670xx_IsRegistered(UART_HandleTypeDef *huart);

char* A7670xx_getMobileOperator(UART_HandleTypeDef *huart);

char* A7670xx_getICCID(UART_HandleTypeDef *huart);

char* A7670xx_getMobileNumber(UART_HandleTypeDef *huart);

char* A7670xx_networkStatus(UART_HandleTypeDef *huart);

char* A7670xx_getIPSIM(UART_HandleTypeDef *huart);

void A7670xx_networkStatusIT(UART_HandleTypeDef *huart);

void A7670xx_smsMessageMode(UART_HandleTypeDef *huart);

void A7670xx_smsSendMessage(UART_HandleTypeDef *huart, char *telephone, char *message);

void A7670xx_tcpipEnable(UART_HandleTypeDef *huart);

uint8_t A7670xx_tcpipSocketConfigure(UART_HandleTypeDef *huart, char *socket, char *protocol, char *ip, char *port);

char* A7670xx_tcpipSocketsData(UART_HandleTypeDef *huart);

void A7670xx_tcpipSendData(UART_HandleTypeDef *huart, char *socket, char *message);

uint8_t A7670xx_httpEnable(UART_HandleTypeDef *huart);

void A7670xx_httpPOST(UART_HandleTypeDef *huart, char *protocol, char *ip, char *port, char *url);

void A7670xx_gnssConfigure(UART_HandleTypeDef *huart, char *time, char *port);

char* A7670xx_gnssGetData(UART_HandleTypeDef *huart);


#endif /* INC_A7670XX_H_ */
