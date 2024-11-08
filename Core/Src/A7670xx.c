/*
 * A7670xx.c
 *
 *  Created on: Jul 23, 2024
 *      Author: garrix
 *      Details: This is the source code to manage A7670xx LTE communication module
 */
#include "A7670xx.h"


//constants section

//Ask for configuration
const char Ask_CREG[] = "AT+CREG?\r\n";
const char Ask_CFUN[] = "AT+CFUN?\r\n";
const char Ask_SIM[] = "AT+CIMI\r\n";
const char Ask_CSPN[] = "AT+CSPN?\r\n";
const char Ask_ICCID[] = "AT+CICCID\r\n";
const char Ask_NUM[] = "AT+CNUM\r\n";
const char Ask_CGATT[] = "AT+CGATT?\r\n";
const char Ask_IP[] = "AT+CGPADDR\r\n";
const char Ask_CMGF[] = "AT+CMGF?\r\n";
const char Ask_CIPMODE[] = "AT+CIPMODE?\r\n";
const char Ask_SocketsData[] = "AT+CIPOPEN?\r\n";
const char Ask_GNSSINFO[] = "AT+CGNSSINFO?\r\n";


//Set up a parameter
const char Echo_Disable[] = "ATE0\r\n";
const char SMS_Mode[] = "AT+CMGF=1\r\n";
const char TCPIP_Enable[] = "AT+NETOPEN=1\r\n";
const char GNSS_Enable[] = "AT+CGNSSPWR=1\r\n";
const char CFUN_Enable[] = "AT+CFUN=1\r\n";
const char GNSS_GetData[] = "AT+CGNSSINFO\r\n";
const char Baud_Configure[] = "AT+IPR=";
const char SMS_Send[] = "AT+CMGS=";
const char TCPIP_Socket[] = "AT+CIPOPEN=";
const char TCPIP_SendData[] = "AT+CIPSEND=";
const char HTTP_Enable[] = "AT+HTTPINIT\r\n";
const char HTTP_Params[] = "AT+HTTPPARA=";
const char HTTP_PostMethod[] = "AT+HTTPACTION=";
const char GNSS_Port[] = "AT+CGNSSPORTSWITCH=";
const char GNSS_SetINFO[] = "AT+CGNSSINFO=";
const char send_command = 26; //This is the SUB ASCII Character


//define functions

void A7670xx_setBaudRate(UART_HandleTypeDef *huart, char *baud)
{
	//This function set up LTE baud rate
	char data[10], command[20];

	strcpy(command, Baud_Configure);

	strcat(command, baud);
	strcat(command, "\r\n");

	HAL_UART_Transmit(huart, (uint8_t*)command, strlen(command), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 100);
}


void A7670xx_disableEcho(UART_HandleTypeDef *huart)
{
	//This function disable Echo in commands
	char data[10];

	HAL_UART_Transmit(huart, (uint8_t*)Echo_Disable, strlen(Echo_Disable), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 100);
}


char* A7670xx_thereisSIMCard(UART_HandleTypeDef *huart)
{
	//This function ask if LTE module has a SIM Card plugged

	static char read[20];
	char *data;
	uint8_t count1 = 0, count2 = 0;

	HAL_UART_Transmit(huart, (uint8_t*)Ask_SIM, strlen(Ask_SIM), 20);

	HAL_UART_Receive(huart, (uint8_t*)read, sizeof(read), 50);

	//return string if there is a SIM Card Plugged

	while(count1 < strlen(read))
	{
		if(isdigit(read[count1]) != NULL)
			break;

		count1++;
	}


	while(count2 < strlen(read))
	{
		if(read[count2] == '\r' && count2 > count1)
		{
			read[count2] = '\0';

			break;
		}

		count2++;
	}

	if(strchr(read, '+') != NULL) //SIM Card isn't plugged
		data = strstr(read, "ERROR");
	else
		data = strchr(read, read[count1]);

	return data;
}


char* A7670xx_IsRegistered(UART_HandleTypeDef *huart)
{
	//This function ask for CREG command

	static char read[20];
	char *data;
	uint8_t count = 0;

	HAL_UART_Transmit(huart, (uint8_t*)Ask_CREG , strlen(Ask_CREG) , 20);

	HAL_UART_Receive(huart, (uint8_t*)read, sizeof(read), 50);

	//delete \r
	while(count <= strlen(read))
	{
		if(read[count] == '\r' && count > 4)
		{
			read[count] = '\0';

			break;
		}

		count++;
	}


	data = strstr(read, "+CREG:");

	return data;
}


char* A7670xx_getMobileOperator(UART_HandleTypeDef *huart)
{
	//This function gets Mobile Operator

	static char read[30];
	char *data, *operator, *token;
	uint8_t count = 0;

	HAL_UART_Transmit(huart, (uint8_t*)Ask_CSPN, strlen(Ask_CSPN), 20);

	HAL_UART_Receive(huart, (uint8_t*)read, sizeof(read), 50);

	while(count <= strlen(read))
	{
		if(read[count] == '\r' && count > 4)
		{
			read[count] = '\0';

			break;
		}

		count++;
	}

	//gets data

	operator = strchr(read, '+');

	if(strncmp(operator, "+CSPN", 5) == 0)
	{
		operator = strstr(read, "\""); //get only data

		token = strtok(operator, ","); //split by comma

		data = token; //get data

		for(uint8_t i = 0; i < strlen(data); i++) //if mobile operator name has a blank space, fill it with '_'
		{
			if(*(data + i) == ' ')
			{
				*(data + i) = '_';

				break;
			}
		}
	}
	else
		data = strstr(read, "ERROR");

	return data;
}


char* A7670xx_getICCID(UART_HandleTypeDef *huart)
{
	//This function gets ICCID data from SIM Card (Impressed code over SIM)
	static char buffer[30];
	char *data;
	uint8_t counter = 0;

	HAL_UART_Transmit(huart, (uint8_t*)Ask_ICCID, strlen(Ask_ICCID) , 20);

	HAL_UART_Receive(huart, (uint8_t*)buffer, sizeof(buffer), 50);

	//define end string
	while(counter < strlen(buffer))
	{
		if(buffer[counter] == '\r' && counter > 4)
		{
			buffer[counter] = '\0';

			break;
		}

		counter++;
	}

	counter = 0; //clear counter

	data = strchr(buffer, '+'); //get string from '+' character

	if(strncmp(data, "+ICCID", 6) == 0)
	{
		while(buffer[counter] != ' ' && counter < strlen(buffer))
			counter++;

		data = strchr(buffer, buffer[counter + 1]);
	}
	else
		data = strstr(buffer, "ERROR");

	return data;
}


char* A7670xx_getMobileNumber(UART_HandleTypeDef *huart)
{
	//This function gets current Mobile Number
	static char buffer[30];
	char *number, *token, *data;
	uint8_t counter = 0;

	HAL_UART_Transmit(huart, (uint8_t*)Ask_NUM, strlen(Ask_NUM), 20);

	HAL_UART_Receive(huart, (uint8_t*)buffer, sizeof(buffer), 50);

	while(counter < strlen(buffer))
	{
		if(buffer[counter] == '\r' && counter > 4)
		{
			buffer[counter] = '\0';

			break;
		}

		counter++;
	}

	number = strchr(buffer, '+');

	if(strncmp(number, "+CNUM", 5) == 0)
	{
		number = strstr(buffer, " ");

		token = strtok(number, ",");

		token = strtok(NULL, ",");

		data = token; //get mobile number

	}
	else
		data = strstr(buffer, "ERROR");

	return data;
}


char* A7670xx_networkStatus(UART_HandleTypeDef *huart)
{
	//This function ask for Network status

	static char read[20];
	char *data;
	uint8_t count = 0;

	HAL_UART_Transmit(huart, (uint8_t*)Ask_CGATT, strlen(Ask_CGATT), 20);

	HAL_UART_Receive(huart, (uint8_t*)read, sizeof(read), 50);

	while(count <= strlen(read))
	{
		if(read[count] == '\r' && count > 4)
		{
			read[count] = '\0';

			break;
		}

		count++;
	}

	data = strstr(read, "+CGATT:");

	return data;

}


char* A7670xx_getIPSIM(UART_HandleTypeDef *huart)
{
	//This function gets SIM Card IP address
	static char buffer[30] = {'\0'}, str[30] = {'\0'};
	char *temp, *data;
	uint8_t count = 0;

	HAL_UART_Transmit(huart, (uint8_t*)Ask_IP, strlen(Ask_IP), 20);

	HAL_UART_Receive(huart, (uint8_t*)buffer, sizeof(buffer), 50);

	while(count <= strlen(buffer))
	{
		if(buffer[count] == '\r' && count > 4)
		{
			buffer[count] = '\0';

			break;
		}

		count++;
	}

	//get data

	temp = strchr(buffer, '+');

	if(strncmp(temp, "+CGPADDR", 8) == 0)
	{
		temp = strtok(buffer, ",");

		temp = strtok(NULL, ",");

		//add double '"'
		strcat(str, "\"");
		strcat(str, temp);
		strcat(str, "\"");

		data = str;

	}
	else
		data = strstr(buffer, "ERROR");

	return data;
}


void A7670xx_networkStatusIT(UART_HandleTypeDef *huart)
{
	//This function ask is LTE module is connected to the Mobile Network

	HAL_UART_Transmit(huart, (uint8_t*)Ask_CGATT, strlen(Ask_CGATT), 20);

}


void A7670xx_smsMessageMode(UART_HandleTypeDef *huart)
{
	//This function set up SMS message mode

	char data[10];

	HAL_UART_Transmit(huart, (uint8_t*)SMS_Mode, strlen(SMS_Mode), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 50);

}


void A7670xx_smsSendMessage(UART_HandleTypeDef *huart, char *telephone, char *message)
{
	//This function send a SMS

	//send SMS command with phone number
	HAL_UART_Transmit(huart, (uint8_t*)SMS_Send, strlen(SMS_Send), 20);
	HAL_UART_Transmit(huart, "\"", 1, 20);
	HAL_UART_Transmit(huart, (uint8_t*)telephone, strlen(telephone), 20);
	HAL_UART_Transmit(huart, "\"\r\n", 3, 20);

	HAL_Delay(20); //waits

	//send message
	HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), 100);
	HAL_UART_Transmit(huart, "\r", 1, 20);
	HAL_UART_Transmit(huart, (uint8_t*)send_command, strlen(send_command), 20);

}


void A7670xx_tcpipEnable(UART_HandleTypeDef *huart)
{
	//This function enable TCP/IP communication

	char data[50];

	HAL_UART_Transmit(huart, (uint8_t*)TCPIP_Enable, strlen(TCPIP_Enable), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 2000);

}


uint8_t  A7670xx_tcpipSocketConfigure(UART_HandleTypeDef *huart, char *socket, char *protocol, char *ip, char *port)
{
	//This function enable TCPIP to works as client and configure
	//a socket with server's information

	char *cipopen, *data, res[20], tcp_data[40] = {'\0'};
	uint8_t count = 0;

	//configure Socket
	strcat(tcp_data, socket);
	strcat(tcp_data, ",\"");
	strcat(tcp_data, protocol);
	strcat(tcp_data, "\",\"");
	strcat(tcp_data, ip);
	strcat(tcp_data, "\",");
	strcat(tcp_data, port);

	//if string lenght is minor than 20 characters, try again open the socket
	do{
		HAL_UART_Transmit(huart, (uint8_t*)TCPIP_Socket, strlen(TCPIP_Socket), 20);
		HAL_UART_Transmit(huart, (uint8_t*)socket, strlen(socket), 20);
		HAL_UART_Transmit(huart, ",\"", 2, 20);
		HAL_UART_Transmit(huart, (uint8_t*)protocol, strlen(protocol), 20);
		HAL_UART_Transmit(huart, "\"", 1, 20);
		HAL_UART_Transmit(huart, ",\"", 2, 20);
		HAL_UART_Transmit(huart, (uint8_t*)ip, strlen(ip), 20);
		HAL_UART_Transmit(huart, "\",", 2, 20);
		HAL_UART_Transmit(huart, (uint8_t*)port, strlen(port), 20);
		HAL_UART_Transmit(huart, "\r\n", 2, 20);

		HAL_Delay(1000);

		//ask if socket was configured with success
		cipopen = A7670xx_TCPIPSocketsData(huart);

		data = strstr(cipopen, socket);

		count++;
	}while(strncmp(data, tcp_data, strlen(tcp_data)) != 0 && count < 3 );

	HAL_UART_Receive(huart, (uint8_t*)res, sizeof(res), 100);

	if(count > 3)
		return 1; //Socket configuration fails
	else
		return 0;

}


char* A7670xx_tcpipSocketsData(UART_HandleTypeDef *huart)
{
	//This function gets data from 0 to 9 socket

	static char read[100];
	char *data;

	HAL_UART_Transmit(huart, (uint8_t*)Ask_SocketsData, strlen(Ask_SocketsData), 20);

	HAL_UART_Receive(huart, (uint8_t*)read, sizeof(read), 200);

	data = strstr(read, "+CIPOPEN:");

	return data;
}


void A7670xx_tcpipSendData(UART_HandleTypeDef *huart, char *socket, char *message)
{
	//This function send data trough TCP/UDP
	char data[20], command[20];

	//build TCPIP command with socket string
	strcpy(command, TCPIP_SendData);

	strcat(command, socket);
	strcat(command, "\r\n");

	//define which socket we'll send data
	HAL_UART_Transmit(huart, (uint8_t*)command, strlen(command), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 50);

	//Send message
	HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), 200);
	HAL_UART_Transmit(huart, (uint8_t*)send_command, strlen(send_command), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 50);

}


uint8_t A7670xx_httpEnable(UART_HandleTypeDef *huart)
{
	//This function enables HTTP communication as client

	char data[100], rx_data[10];
	uint8_t count = 0;

	do{
		HAL_UART_Transmit(huart, (uint8_t*)HTTP_Enable, strlen(HTTP_Enable), 20);

		HAL_UART_Receive(huart, (uint8_t*)rx_data, sizeof(rx_data), 50);

		HAL_Delay(100);

		count++;
	}while(strncmp(rx_data, "ERROR", 5) == 0 && count < 3);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 5000);

	if(count == 3)
		return 1;
	else
		return 0;
}


void A7670xx_httpPOST(UART_HandleTypeDef *huart, char *protocol, char *ip, char *port, char *url)
{
	//This function configures HTTP parameters as follows

	char post_method[20] = {'\0'}, command[260] = {'\0'};

	//set up HTTP URL
	strcat(command, HTTP_Params);
	strcat(command, "\"URL\",\"");
	strcat(command, protocol);
	strcat(command, "://");
	strcat(command, ip);
	strcat(command, ":");
	strcat(command, port);
	strcat(command, "/");
	strcat(command, url);
	strcat(command, "\"\r\n");

	//set up post command
	strcat(post_method, HTTP_PostMethod);
	strcat(post_method, "1\r\n");

	//send command
	HAL_UART_Transmit(huart, (uint8_t*)command, strlen(command), 300);

	//send HTTP method
	HAL_UART_Transmit(huart, (uint8_t*)post_method, strlen(post_method), 20);

}


void A7670xx_gnssConfigure(UART_HandleTypeDef *huart, char *time, char *port)
{
	//This function configure GNNS to send data each x time trough UART or USB port

	char data[20], set_time[20], set_port[30], *gnss;

	strcpy(set_time, GNSS_SetINFO);

	strcpy(set_port, GNSS_Port);


	strcat(set_time, time);
	strcat(set_time, "\r\n");

	strcat(set_port, port);
	strcat(set_port, "\r\n");

	//Enable GNSS
	HAL_UART_Transmit(huart, (uint8_t*)GNSS_Enable, strlen(GNSS_Enable), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 5000);

	//confirm that GNSS data is ready!
	do{
		HAL_Delay(5000); //wait 5 seconds

		gnss = A7670xx_gnssGetData(huart);

	}while(strncmp(gnss, "+CGNSSINFO: ,,,", 15) == 0);

	//if GNSS data is ready!

	//set up GNSS data time
	HAL_UART_Transmit(huart, (uint8_t*)set_time, strlen(set_time), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 50);

	//set up which port GNSS will send data
	HAL_UART_Transmit(huart, (uint8_t*)set_port, strlen(set_port), 20);

	HAL_UART_Receive(huart, (uint8_t*)data, sizeof(data), 50);

}


char* A7670xx_gnssGetData(UART_HandleTypeDef *huart)
{
	//This function gets GNSS data
	static char buffer[120];
	char *gnssdata;
	uint8_t count;

	//request GNSS data
	HAL_UART_Transmit(huart, (uint8_t*)GNSS_GetData, strlen(GNSS_GetData), 20);

	HAL_UART_Receive(huart, (uint8_t*)buffer, sizeof(buffer), 200);

	while(count <= strlen(buffer))
	{
		if(buffer[count] == '\r' && count > 4)
		{
			buffer[count] = '\0';

			break;
		}

		count++;
	}

	//clear data received
	gnssdata = strstr(buffer, "+CGNSSINFO");

	return gnssdata;
}
