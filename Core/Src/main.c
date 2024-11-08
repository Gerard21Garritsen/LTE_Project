/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx_ll_usart.h"
#include "A7670xx.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

//global variables
char *rx_buffer; //general buffer
char rx_data[1] = {'\0'}; //This variable store character by character received by UART
char rx_isrbuffer[120] = {'\0'}; //This array store all data received by UART ISR
char rx_cleardata[120] = {'\0'}; //This array store all data received clear
char rx_networkstatus[30] = {'\0'}; //This array stores CGATT answer into UART ISR
char *gnss_data[8]; //this store GNSS data splited
char *ptrtoken; //this is the token pointer
uint8_t usart_itflag = 0; //UART ISR flag
uint8_t count = 12; //this counts each token from GNSS array
uint8_t ctn = 0; //This counts number of variables received by USART IT
uint8_t error = 0; //this variable control if happened an error


//global variables used by program
char baudrate[] = "115200";
char http_protocol[] = "http";
char ip_address[] = "189.130.47.178";
char puerto[] = "3000";
char mobile_operator[20] = {'\0'};
char mobile_cimi[16] = {'\0'};
char mobile_iccid[22] = {'\0'};
char mobile_number[15] = {'\0'};
char mobile_ip[20] = {'\0'};
char url_num[] = "pruebatrack?num=";
char url_operator[] = "&oper=";
char url_iccid[] = "&iccid=";
char url_cimi[] = "&cimi=";
char url_ipsim[] = "&ipsim=";
char url_lat[] = "&lat=";
char url_latp[] = "&latp=";
char url_lon[] = "&lon=";
char url_lonp[] = "&lonp=";
char url_date[] = "&date=";
char url_time[] = "&time=";
char url_high[] = "&high=";
char url[220] = {'\0'};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /*Begins LTE Module initialization*/
  HAL_Delay(30000); //wait 30 seconds

  A7670xx_setBaudRate(&huart1, baudrate);

  LL_USART_SetBaudRate(USART1, 25000000,LL_USART_OVERSAMPLING_16 , 115200);

  HAL_Delay(1000);

  A7670xx_disableEcho(&huart1);

  rx_buffer = A7670xx_thereisSIMCard(&huart1);

  if(strncmp(rx_buffer, "ERROR", 5) == 0)
	  error = 1;
  else
	  strcpy(mobile_cimi, rx_buffer);

  rx_buffer = A7670xx_getMobileOperator(&huart1);

  if(strncmp(rx_buffer, "ERROR", 5) == 0)
	  error = 1;
  else
	  strcpy(mobile_operator, rx_buffer);

  rx_buffer = A7670xx_getICCID(&huart1);

  if(strncmp(rx_buffer, "ERROR", 5) == 0)
	  error = 1;
  else
	  strcpy(mobile_iccid, rx_buffer);

  rx_buffer = A7670xx_getMobileNumber(&huart1);

  if(strncmp(rx_buffer, "ERROR", 5) == 0)
	  error = 1;
  else
	  strcpy(mobile_number, rx_buffer);

  rx_buffer = A7670xx_getIPSIM(&huart1);

  if(strncmp(rx_buffer, "ERROR", 5) == 0)
	  error = 1;
  else
	  strcpy(mobile_ip, rx_buffer);

  rx_buffer = A7670xx_networkStatus(&huart1);

   if(strncmp(rx_buffer, "+CGATT: 0", 9) == 0)
 	  error = 1;

  if(error == 0) //if there weren't error, enable HTTP
	  error = A7670xx_httpEnable(&huart1);

  //enable and configure GNSS
  A7670xx_gnssConfigure(&huart1, "30", "1");

  //enable UART ISR
  HAL_UART_Receive_IT(&huart1, (uint8_t*)rx_data, 1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //While loop routine

	  if(usart_itflag)
	  {
		  rx_buffer = strstr(rx_isrbuffer, "+C"); //get only clear data

		  strcpy(rx_cleardata, rx_buffer);

		  memset(rx_isrbuffer, '\0', strlen(rx_isrbuffer)); //clear rx_isrbuffer

		  usart_itflag = 0; //clear USART flag

		  if(strncmp(rx_cleardata, "+CGNSSINFO", 10) == 0) //process a GNSS data and send it by HTTP/HTTPS
		  {

			  A7670xx_networkStatusIT(&huart1);

			  while(usart_itflag == 0); //wait  until read data

			  //read buffer
			  rx_buffer = strstr(rx_isrbuffer, "+C");

			  strcpy(rx_networkstatus, rx_buffer);

			  memset(rx_isrbuffer, '\0', strlen(rx_isrbuffer));

			  usart_itflag = 0; //clear USART flag

			  if(strcmp(rx_networkstatus, "+CGATT: 1") == 0) //if LET is connected, send data through HTTP
			  {
				  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

				  //split string received by ',' character
				  ptrtoken = strtok(rx_cleardata, ",");

				  while((ptrtoken != NULL) && (count > 1))
				  {
					  count--;

					  if(count <= 7)
						  gnss_data[count] = ptrtoken;

					  ptrtoken = strtok(NULL, ",");
				  }

				  memset(rx_cleardata, '\0', strlen(rx_cleardata)); //clear data array
				  memset(rx_networkstatus, '\0', strlen(rx_networkstatus)); //clear network status array

				  //built url with data
				  strcat(url, url_num);
				  strcat(url, mobile_number);
				  strcat(url, url_iccid);
				  strcat(url, mobile_iccid);
				  strcat(url, url_cimi);
				  strcat(url, mobile_cimi);
				  strcat(url, url_operator);
				  strcat(url, mobile_operator);
				  strcat(url, url_ipsim);
				  strcat(url, mobile_ip);
				  strcat(url, url_lat);
				  strcat(url, gnss_data[7]);
				  strcat(url, url_latp);
				  strcat(url, gnss_data[6]);
				  strcat(url, url_lon);
				  strcat(url, gnss_data[5]);
				  strcat(url, url_lonp);
				  strcat(url, gnss_data[4]);
				  strcat(url, url_date);
				  strcat(url, gnss_data[3]);
				  strcat(url, url_time);
				  strcat(url, gnss_data[2]);
				  strcat(url, url_high);
				  strcat(url, gnss_data[1]);

				  //send data through HTTP
				  A7670xx_httpPOST(&huart1, http_protocol, ip_address, puerto, url);

				  memset(url, '\0', strlen(url)); //clear url array

				  count = 12; //reset counter
			  }

		  }

	  }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           PA8 PA11 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB12 PB13 PB14 PB15
                           PB3 PB4 PB5 PB6
                           PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

//UART Rx ISR routine
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */

	if(((rx_data[0] == '\r' && ctn > 4)) && (usart_itflag == 0)) //if data received has ended
	{
		ctn = 0; //clear counter

		usart_itflag = 1; //sets end of string flag
	}
	else if(usart_itflag == 0)
	{
		if(strlen(rx_isrbuffer) >= 150) //to avoid overflows
			memset(rx_isrbuffer, '\0', strlen(rx_isrbuffer));

		if(rx_data[0] != '\r' && rx_data[0] != '\n')
			strcat(rx_isrbuffer, rx_data);

		ctn++;
	}

	HAL_UART_Receive_IT(&huart1, (uint8_t*)rx_data, 1);
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
