/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "can.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ID_EXT_BATERIA 0x1806E5F4;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void CAN_Filter_Config(void);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t led_timer = 1000;
uint16_t test = 0;

CAN_RxHeaderTypeDef rxHeader;
CAN_TxHeaderTypeDef txHeader;

uint8_t CanTxBuffer[8]; //Buffer escritura
uint8_t CanRxBuffer[8]; //Buffer lectura
uint32_t CanMailBox; //Para enviar
uint8_t flag_rx = 0, tx_complete = 0, tx_retry = 0; // banderas
uint8_t flag_carga_completa = 0;
uint8_t counterTime = 5; // contador tiempo


//uint8_t led_flag = 0;
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
  MX_TIM2_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  CAN_Filter_Config();
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_TIM_Base_Start_IT(&htim2);

  uint16_t voltaje_bateria = 0;
  uint8_t CanTxBuffer[8] = {0x02, 0x48, 0x01, 0x2C, 0x00, 0x00, 0x00, 0x00};
  //CanTxBuffer = data;
  //uint32_t txMailbox;

  txHeader.StdId = 0; // No se utiliza en el modo extendido
  txHeader.ExtId = ID_EXT_BATERIA; // Identificador extendido
  txHeader.RTR = CAN_RTR_DATA;
  txHeader.IDE = CAN_ID_EXT; // Usar identificador extendido
  txHeader.DLC = 8; // Longitud de datos
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(!led_timer && !flag_carga_completa){
		  led_timer = 1000;
		  HAL_GPIO_TogglePin(ONBOARD_LED_GPIO_Port, ONBOARD_LED_Pin);
		  //HAL_GPIO_TogglePin(External_LED_RED_GPIO_Port, External_LED_RED_Pin);// Asume que el LED está conectado al pin GPIOB_PIN_0
		  //HAL_GPIO_TogglePin(External_LED_GREEN_GPIO_Port, External_LED_GREEN_Pin);// Asume que el LED está conectado al pin GPIOB_PIN_0
		  if (HAL_CAN_AddTxMessage(&hcan, &txHeader, CanTxBuffer, &CanMailBox) != HAL_OK) {
			  //HAL_GPIO_TogglePin(External_LED_RED_GPIO_Port, External_LED_RED_Pin);
		          // Error al enviar el mensaje
		      }
	  }
	  if(flag_rx){
		  if (rxHeader.IDE == CAN_ID_EXT && rxHeader.ExtId == 0x18FF50E5) {
			  // Cambiar el estado del LED
			  voltaje_bateria |= CanRxBuffer[0]<<8;
			  voltaje_bateria |= CanRxBuffer[1];
			  if(voltaje_bateria == 534){
				  HAL_GPIO_TogglePin(External_LED_RED_GPIO_Port, External_LED_RED_Pin);// Asume que el LED está conectado al pin GPIOB_PIN_0
				  flag_carga_completa = 1;
			  }

		  }
		  flag_rx=0;
	  }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, CanRxBuffer) == HAL_OK){
		flag_rx = 1;
	}
}

void CAN_Filter_Config(void) //Funcion para filtro can
{
	CAN_FilterTypeDef filterConfig;
    filterConfig.FilterIdHigh = 0x0000;
    filterConfig.FilterIdLow = 0x0000;
    filterConfig.FilterMaskIdHigh = 0x0000;
    filterConfig.FilterMaskIdLow = 0x0000;
    filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    filterConfig.FilterBank = 0;
    filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterActivation = ENABLE;

	if (HAL_CAN_ConfigFilter(&hcan, &filterConfig) != HAL_OK) {
	            // Error en la configuración del filtro
	        }
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(led_timer)
		led_timer--;
	if(htim->Instance == TIM2)  // Verifica que la interrupción provino de TIM2
	    {
	    }

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
