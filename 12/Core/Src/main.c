/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdio.h>
#include <string.h>

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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char TxDataBuffer[32] =
{ 0 };
char RxDataBuffer[32] =
{ 0 };
int16_t inputchar = 0;
uint16_t state = 0;
float f = 1;
float T = 0;
float Vhigh = 3.3;
float Vlow = 0.0;
int slope = 0;
float Duty_cycle = 0;
int wave = 0;
float VIn = 0;
float VOut = 0;
float deltaV = 0;
float delta_t = 0;
float deltaVfinal = 0;
double angle = 0;
float t = 0;
float pi = 3.14;
uint16_t VhighADC = 0;
uint16_t VlowADC = 0;
static uint64_t timestamp = 0;
static uint64_t wavetime = 0;

uint16_t ADCin = 0;
uint64_t _micro = 0;
uint16_t dataOut = 0;
	uint8_t DACConfig = 0b0011;  //SPIจะมี16bit โหมดในSPIใช้4bit ข้อมูลอีก12bit
enum state
{
	state_start = 0,
	state_Menu = 10,
	state_Sawtooth = 20,
	state_Sine = 30,
	state_Square = 40,
	state_functionSawtooth = 50,
	state_functionSine = 60,
	state_functionSquare = 70
};

char Menu[]="press 1 : Sawtooth wave\r\n"
			    "press 2 : Sine wave\r\n"
			    "press 3 : Square wave\r\n\r\n";
	char SawtoothMenu[]="SawtoothMenu\r\n "
			            "press a: +frequency\r\n "
			            "press s: -frequency\r\n "
		                "press q: +V high\r\n "
		                "press w: -V high\r\n "
		                "press e: +V low \r\n "
		                "press r: -V low\r\n "
		                "press d: slop Up\r\n "
		                "press f: slope down\r\n "
		                "press x: back\r\n\r\n";
	char SineMenu[]="SineMenu\r\n "
			        "press a: +frequency\r\n "
			        "press s: -frequency\r\n "
			        "press q: +V high\r\n "
			        "press w: -V high\r\n "
			        "press e: +V low \r\n "
			        "press r: -V low\r\n"
			        "press x: back\r\n\r\n ";
	char SquareMenu[]="SquareMenu\r\n "
			          "press a: +frequency\r\n "
			          "press s: -frequency\r\n "
				      "press q: +V high\r\n "
				      "press w: -V high\r\n "
				      "press e: +V low \r\n "
				      "press r: -V low\r\n "
				      "press d: +Duty cycle\r\n "
				      "press f: -Duty cycle\r\n "
				      "press x: back\r\n\r\n ";
	char showf[10] = "";
	char showVH[20] = "";
	char showVL[20] = "";
	char showDutyCycle[40] = "";
	char SlopeUp[] = "SlopeUp\r\n";
	char SlopeDown[] = "SlopeDown\r\n";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI3_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void MCP4922SetOutput(uint8_t Config, uint16_t DACOutput);
uint64_t micros();
int16_t UARTRecieveIT();
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI3_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADCin, 1);

	HAL_GPIO_WritePin(LOAD_GPIO_Port, LOAD_Pin, GPIO_PIN_RESET);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		HAL_UART_Receive_IT(&huart2,  (uint8_t*)RxDataBuffer, 10);
		inputchar = UARTRecieveIT();
		switch (state)
		{
			case state_start:
				wave = 0;
//				char Menu[]="press 1 : Sawtooth wave\r\n"
//						    "press 2 : Sine wave\r\n"
//						    "press 3 : Square wave\r\n\r\n ";
				HAL_UART_Transmit(&huart2, (uint8_t*)Menu, strlen(Menu),10);
				state = state_Menu;
				break;
			case state_Menu:
				if (inputchar == '1')
				{
					state = state_Sawtooth;
				}
				else if (inputchar == '2')
				{
					state = state_Sine;
				}
				else if (inputchar == '3')
				{
					state = state_Square;
				}
				else
				{
					state = state_Menu;
				}
				break;
			case state_Sawtooth:
//				char SawtoothMenu[]="SawtoothMenu\r\n "
//									"press a:+frequency\r\n "
//									"press s:-frequency\r\n "
//									"press q: +V high\r\n "
//									"press w: -V high\r\n "
//									"press e: +V low \r\n "
//									"press r: -V low\r\n "
//									"press d: slop Up\r\n "
//									"press f: slope down\r\n "
//									"press x: back\r\n\r\n ";
				HAL_UART_Transmit(&huart2, (uint8_t*)SawtoothMenu, strlen(SawtoothMenu),100);
				state = state_functionSawtooth;
				break;
			case state_functionSawtooth:
				wave = 1;
				if (inputchar == 'a') //เพิ่มความถี่
				{
					f += 0.1;
					if(f>10)
					{
						f = 10;
					}
					sprintf(showf,"f = %f Hz\r\n",f);
					HAL_UART_Transmit(&huart2, (uint8_t*)showf, strlen(showf), 10);
					state = state_functionSawtooth;
				}
				else if(inputchar == 's')  //ลดความถี่
				{
					f -= 0.1;
					if(f<=0)
					{
						f = 0;
					}
					sprintf(showf,"f = %f Hz\r\n",f);
					HAL_UART_Transmit(&huart2, (uint8_t*)showf, strlen(showf), 10);
					state = state_functionSawtooth;
				}
				else if(inputchar == 'q')  //เพิ่ม V high
				{
					Vhigh = Vhigh + 0.1 ;
					if(Vhigh>3.3)
					{
						Vhigh = 3.3;
					}
					sprintf(showVH,"Vhigh = %f Volt\r\n",Vhigh);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVH, strlen(showVH), 10);
					state = state_functionSawtooth;
				}
				else if(inputchar == 'w')  //ลด V high
				{
					Vhigh = Vhigh - 0.1 ;
					if(Vhigh<0)
					{
						Vhigh = 0;
					}
					sprintf(showVH,"Vhigh = %f Volt\r\n",Vhigh);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVH, strlen(showVH), 10);
					state = state_functionSawtooth;
				}
				else if(inputchar == 'e')  //เพิ่ม V low
				{
					Vlow = Vlow + 0.1 ;
					if(Vlow>3.3)
					{
						Vlow = 3.3;
					}
					sprintf(showVL,"VLow = %f Volt\r\n",Vlow);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVL, strlen(showVL), 10);
					state = state_functionSawtooth;
				}
				else if(inputchar == 'r')  //ลด V low
				{
					Vlow = Vlow - 0.1 ;
					if(Vlow<0)
					{
						Vlow = 0;
					}
					sprintf(showVL,"VLow = %f Volt\r\n",Vlow);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVL, strlen(showVL), 10);
					state = state_functionSawtooth;
				}
				else if(inputchar == 'd')  //slope up
				{
					slope = 1;
					HAL_UART_Transmit(&huart2, (uint8_t*)SlopeUp, strlen(SlopeUp), 10);
					state = state_functionSawtooth;
				}
				else if(inputchar == 'f')  //slope down
				{
					slope = -1;
					HAL_UART_Transmit(&huart2, (uint8_t*)SlopeDown, strlen(SlopeDown), 10);
					state = state_functionSawtooth;
				}
				else if(inputchar == 'x')
				{
					state = state_start;
				}
				else
				{
					state = state_functionSawtooth;
				}
				break;
			case state_Sine:
//				char SineMenu[]="SineMenu\r\n "
//								"press a:+frequency\r\n "
//								"press s:-frequency\r\n "
//								"press q: +V high\r\n "
//								"press w: -V high\r\n "
//								"press e: +V low \r\n "
//								"press r: -V low\r\n "
//								"press x: back\r\n\r\n ";
				HAL_UART_Transmit(&huart2, (uint8_t*)SineMenu, strlen(SineMenu),100);
				state = state_functionSine;
				break;
			case state_functionSine:
				wave = 2;
				if (inputchar == 'a') //เพิ่มความถี่
				{
					f = f + 0.1;
					if(f>10)
					{
						f = 10;
					}
					sprintf(showf,"f = %f Hz\r\n",f);
					HAL_UART_Transmit(&huart2, (uint8_t*)showf, strlen(showf), 10);
					state = state_functionSine;
				}
				else if(inputchar == 's')  //ลดความถี่
				{
					f = f - 0.1;
					if(f<0)
					{
						f = 0;
					}
					sprintf(showf,"f = %f Hz\r\n",f);
					HAL_UART_Transmit(&huart2, (uint8_t*)showf, strlen(showf), 10);
					state = state_functionSine;
				}
				else if(inputchar == 'q')  //เพิ่ม V high
				{
					Vhigh = Vhigh + 0.1 ;
					if(Vhigh>3.3)
					{
						Vhigh = 3.3;
					}
					sprintf(showVH,"Vhigh = %f Volt\r\n",Vhigh);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVH, strlen(showVH), 10);
					state = state_functionSine;
				}
				else if(inputchar == 'w')  //ลด V high
				{
					Vhigh = Vhigh - 0.1 ;
					if(Vhigh<0)
					{
						Vhigh = 0;
					}
					sprintf(showVH,"Vhigh = %f Volt\r\n",Vhigh);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVH, strlen(showVH), 10);
					state = state_functionSine;
				}
				else if(inputchar == 'e')  //เพิ่ม V low
				{
					Vlow = Vlow + 0.1 ;
					if(Vlow>3.3)
					{
						Vlow = 3.3;
					}
					sprintf(showVL,"VLow = %f Volt\r\n",Vlow);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVL, strlen(showVL), 10);
					state = state_functionSine;
				}
				else if(inputchar == 'r')  //ลด V low
				{
					Vlow = Vlow - 0.1 ;
					if(Vlow<0)
					{
						Vlow = 0;
					}
					sprintf(showVL,"VLow = %f Volt\r\n",Vlow);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVL, strlen(showVL), 10);
					state = state_functionSine;
				}
				else if(inputchar == 'x')  //ย้อน�?ลับไป menu
				{
					state = state_start;
				}
				else
				{
					state =state_functionSine;
				}
				break;
			case state_Square:
//				char SquareMenu[]="SquareMenu\r\n "
//						          "press a:+frequency\r\n "
//						          "press s:-frequency\r\n "
//								  "press q: +V high\r\n "
//								  "press w: -V high\r\n "
//								  "press e: +V low \r\n "
//								  "press r: -V low\r\n "
//								  "press d: +Duty_cyccle\r\n "
//								  "press f: -Duty_cyccle\r\n "
//								  "press x: back\r\n ";
				HAL_UART_Transmit(&huart2, (uint8_t*)SquareMenu, strlen(SquareMenu),100);
				state = state_functionSquare;
				break;
			case state_functionSquare:
				wave = 3;
				if (inputchar == 'a')  //เพิ่มความถี่
				{
					f = f + 0.1;
					if(f>10)
					{
						f = 10;
					}
					sprintf(showf,"f = %f Hz\r\n",f);
					HAL_UART_Transmit(&huart2, (uint8_t*)showf, strlen(showf), 10);
					state = state_functionSquare;
				}
				else if(inputchar == 's')  //ลดความถี่
				{
					f = f - 0.1;
					if(f<0)
					{
						f = 0;
					}
					sprintf(showf,"f = %f Hz\r\n",f);
					HAL_UART_Transmit(&huart2, (uint8_t*)showf, strlen(showf), 10);
					state = state_functionSquare;
				}
				else if(inputchar == 'q')  //เพิ่ม V high
				{
					Vhigh = Vhigh + 0.1 ;
					if(Vhigh>3.3)
					{
						Vhigh = 3.3;
					}
					sprintf(showVH,"Vhigh = %f Volt\r\n",Vhigh);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVH, strlen(showVH), 10);
					state = state_functionSquare;
				}
				else if(inputchar == 'w')  //ลด V high
				{
					Vhigh = Vhigh - 0.1 ;
					if(Vhigh<0)
					{
						Vhigh = 0;
					}
					sprintf(showVH,"Vhigh = %f Volt\r\n",Vhigh);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVH, strlen(showVH), 10);
					state = state_functionSquare;
				}
				else if(inputchar == 'e')  //เพิ่ม V low
				{
					Vlow = Vlow + 0.1 ;
					if(Vlow>3.3)
					{
						Vlow = 3.3;
					}
					sprintf(showVL,"VLow = %f Volt\r\n",Vlow);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVL, strlen(showVL), 10);
					state = state_functionSquare;
				}
				else if(inputchar == 'r')  //ลด V low
				{
					Vlow = Vlow - 0.1 ;
					if(Vlow<0)
					{
						Vlow = 0;
					}
					sprintf(showVL,"VLow = %f Volt\r\n",Vlow);
					HAL_UART_Transmit(&huart2, (uint8_t*)showVL, strlen(showVL), 10);
					state = state_functionSquare;
				}
				else if(inputchar == 'd')  //เพิ่ม Duty cycle
				{
					Duty_cycle = Duty_cycle + 10;
					if(Duty_cycle > 100)
					{
						Duty_cycle = 100;
					}
					sprintf(showDutyCycle,"DutyCycle = %f Percent\r\n",Duty_cycle);
					HAL_UART_Transmit(&huart2, (uint8_t*)showDutyCycle, strlen(showDutyCycle), 10);
					state = state_functionSquare;
				}
				else if(inputchar == 'f')  //ลด Duty cycle
				{
					Duty_cycle = Duty_cycle - 10;
					if(Duty_cycle < 0)
					{
						Duty_cycle = 0;
					}
					sprintf(showDutyCycle,"DutyCycle = %f Percent\r\n",Duty_cycle);
					HAL_UART_Transmit(&huart2, (uint8_t*)showDutyCycle, strlen(showDutyCycle), 10);
					state = state_functionSquare;
				}
				else if(inputchar == 'x')  //�?ลับไปที่ menu
				{
					state = state_start;
				}
				else
				{
					state = state_functionSquare;
				}
				break;
		}
		if (micros() - timestamp > 100) //10k Hz
		{
			timestamp = micros();
			if(wave == 1) //sawtooth
			{
				if(slope == 1)  //slope up
				{
					T = 1/f;
					if(micros() - wavetime <= (1000000*T))  //เปลี่ยนหน่วยเป็น microsec
					{
						deltaVfinal = Vhigh - Vlow;
						delta_t = micros() - wavetime;
						deltaV = Vlow + (deltaVfinal)*(delta_t/(1000000*T));
						dataOut = deltaV*4096/3.3;
					}
					else if(micros() - wavetime > (1000000*T))
					{
						wavetime = micros();
					}
				}
				else if(slope == -1) //slope down
				{
					if(micros() - wavetime <= (1000000*T))
					{
						deltaVfinal = Vhigh - Vlow;
						delta_t = micros() - wavetime;
						deltaV = Vhigh - (deltaVfinal)*(delta_t/(1000000*T));
						dataOut = deltaV*4096/3.3;
					}
					else if(micros() - wavetime > (1000000*T))
					{
						wavetime = micros();
					}
				}
			}
			else if(wave == 2)  // sine wave
			{
				T = 1/f;
				if(micros() - wavetime <= (1000000*T))
				{
					deltaVfinal = Vhigh - Vlow;
					delta_t = micros() - wavetime;
					VOut = Vlow + ((sin((delta_t/(1000000*T))*2*pi)+1)*(deltaVfinal/2));
					dataOut = VOut*4096/3.3;
				}
				else if(micros() - wavetime > (1000000*T))
				{
					wavetime = micros();
				}
			}
			else if(wave == 3)  //square wave
			{
				T = 1/f;
				if(micros() - wavetime <= (1000000*T))
				{
					if(micros() - wavetime <= (1000000*T)*(Duty_cycle/100))
					{
						VOut = Vhigh;
					}
					else if(micros() - wavetime > (1000000*T)*(Duty_cycle/100))
					{
						VOut = Vlow;
					}
					dataOut = VOut*4096/3.3;
				}
				else if(micros() - wavetime > (1000000*T))
				{
					wavetime = micros();
				}
			}
			if (hspi3.State == HAL_SPI_STATE_READY && HAL_GPIO_ReadPin(SPI_SS_GPIO_Port, SPI_SS_Pin) == GPIO_PIN_SET)
			{
				MCP4922SetOutput(DACConfig, dataOut);  //DACConfigคือ โหลดที่ให้ทำงาน  dataOutคือข้อมูลที่ส่ง 12bit
			}
			timestamp = micros();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 99;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 99;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_SS_GPIO_Port, SPI_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SHDN_GPIO_Port, SHDN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LOAD_GPIO_Port, LOAD_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin LOAD_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LOAD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_SS_Pin */
  GPIO_InitStruct.Pin = SPI_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_SS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SHDN_Pin */
  GPIO_InitStruct.Pin = SHDN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SHDN_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void MCP4922SetOutput(uint8_t Config, uint16_t DACOutput)
{
	uint32_t OutputPacket = (DACOutput & 0x0fff) | ((Config & 0xf) << 12);  //DACOutputคือ ข้อมูล12bit Configคือ โหมด4bit
	HAL_GPIO_WritePin(SPI_SS_GPIO_Port, SPI_SS_Pin, GPIO_PIN_RESET);  //เปลี่ยน SS ให้เป็น low เพื่อให้เริ่มส่งข้อมูล
	HAL_SPI_Transmit_IT(&hspi3, &OutputPacket, 1);  //ส่งข้อมูล
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi3)  //เมื่อส่งข้อมูลครบ 16bit
		{
		HAL_GPIO_WritePin(SPI_SS_GPIO_Port, SPI_SS_Pin, GPIO_PIN_SET);  //เปลี่ยน SS ให้เป็น high คือสิ้นสุดการส่งข้อมูล
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim2)
	{
		_micro += 65535;
	}
}

inline uint64_t micros()
{
	return htim2.Instance->CNT + _micro;
}
int16_t UARTRecieveIT()
{
	static uint32_t dataPos =0;
	int16_t data=-1;
	if(huart2.RxXferSize - huart2.RxXferCount!=dataPos)
	{
		data=RxDataBuffer[dataPos];
		dataPos= (dataPos+1)%huart2.RxXferSize;
	}
	return data;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
