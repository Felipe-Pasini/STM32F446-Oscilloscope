/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           main.c
  * @brief          Main Program Body
  * @author 		Felipe Pasini Graeff
  * @date 			June 26, 2021
  * @version 		1.2
  ******************************************************************************
  * Up to 3 channel (PA0, PA1 e PA4) Oscilloscope application, with sampling rate up to 2 MHz
  * (for 1 channel only) and calculation of the period of the acquired signal. <br>
  * Data is send for visualization via serial port (USART2).<br>
  *
  *
  * Data transfer protocol (Micro -> PC): <br>
  *
  * <pre>  2 bytes -> 1st channel period
  *   2 bytes -> 2nd channel period (if enabled)
  *   2 bytes -> 3rd channel period (if enabled)
  *
  *   2 bytes -> number of samples (commom for every channel)
  *
  *   1 byte ('n' value in channel 1)              \
  *   1 byte ('n' value in channel 2, if enabled)   > until 'n' = number of samples
  *   1 byte ('n' value in channel 3, if enabled)  /
  * </pre>
  * Data transfer protocol (PC -> Micro):<br>
  *
  * <pre>  character 'C' -> next byte will be configuration byte
  * 					    the following 2 bytes will be the number of samples to be acquired
  *
  *   Configuration byte:
  *      bits 0-3 (less significant) -> Sample Rate
  *         0b0000: 1kHz
  *         0b0001: 5kHz
  *         0b0010: 10kHz
  *         0b0011: 50kHz
  *         0b0100: 100kHz
  *         0b0101: 500kHz
  *         0b0110: 1MHz (Can be used only for 1 or 2 channel)
  *         0b0111: 2MHz (Can be used only for 1 channel)
  *         0b1xxx: Not used
  *
  *      bits 4-6 -> Number of channels
  *         0b000xxxx: 1 channel
  *         0b001xxxx: 2 channels
  *         0b010xxxx: 3 channels
  *         0b011xxxx: Not used
  *         0b1xxxxxx: Not used
  *
  *      bit 7: -> Number of samples is set by the user
  *         0b0xxxxxxx: Number of samples was not set (will be automatically defined)
  *         0b1xxxxxxx: Number of asmples was set by the user
  *
  *   2 bytes -> Number of samples (Ignored if the number of samples was not set)
  *
  *   Character 'S' -> Start of conversion (even if there was no configuration)
  *      Default configuration:
  *         Sampling rate = 500kHz
  *         Number of channels = 1
  *         Number of samples = not set
  *      If there is any anterior configuration, this one shall be used
  *
  *
  * </pre>
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MIN 0
#define MAX 1
#define FALSE 0
#define TRUE 1
#define MAX_NUM_OF_CONVERSIONS 10000
#define MAX_NUM_OF_CHANNELS 3
#define START_OF_CONVERSION 83
#define DEFINE_CONFIG 67
#define COM_TIMEOUT 10000
#define BAUD_115200_WAIT_TIME 10000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
///serial data received
unsigned char data = 0;
///pointer to the serial data received
unsigned char *pointer=&data;

///if set, the next byte to be received is the configuration byte
char Config=FALSE;
///if set, the user has defined the number of samples to be displayed
char N_Samples_is_Set=FALSE;
///number of channels used in acquisition
unsigned char Number_of_Channels = 1;
///number of samples to be sent via serial COM
unsigned int Number_of_Samples;
///main array with the ADC values (raw data)
unsigned char array[MAX_NUM_OF_CONVERSIONS*MAX_NUM_OF_CHANNELS];
///current operating channel during processing
unsigned char channel=0;
///number of conversions made by ADC
unsigned int ADC_conversions=0;
///signal period of each channel
unsigned int period[MAX_NUM_OF_CHANNELS]={0,0,0};
///minimum and maximum position in each channel (to determine period)
unsigned int position[2][2500][MAX_NUM_OF_CHANNELS];
///Initial value from which the serial data shall be sent
unsigned int Init_Value = 0;

///Auto Reload Preload values for each sampling rate
const unsigned int SR_Available[]=
{
		39999,//0 - 1k
		7999,//1 - 5k
		3999,//2 - 10k
		799,//3 - 50k
		399,//4 - 100k
		79,//5 - 500k
		39,//6 - 1M
		19//7 - 2M
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Checks the number of samples to be sent via serial based on the signal period
 * Also defines the Init_Value (from which the samples shal be sent).
 * @retval int HAL_OK
 */
int GetSamplesAuto (void)
{
	int Max_Period = 0;

	for(channel=0;channel<Number_of_Channels;channel++)///Checks the maximum period between all channels
	{
		if(period[channel]>Max_Period)Max_Period=period[channel];
	}

	if(Max_Period==0)Max_Period=MAX_NUM_OF_CONVERSIONS;///If a complete period could not be found in any channel, sends all samples

	if(Max_Period>(MAX_NUM_OF_CONVERSIONS/2))///If the period is more than 50% of the samples, sends all samples
	{
		Number_of_Samples=MAX_NUM_OF_CONVERSIONS;
		Init_Value=0;
	}else if(Max_Period>(MAX_NUM_OF_CONVERSIONS/10))///If the period is between 10% and 50% of total samples, sends 2 periods of samples
	{
		Number_of_Samples=2*Max_Period;

		//2 periods + first minimum value gets over the total number of samples?
		if((position[MIN][0][0]+Number_of_Samples)>MAX_NUM_OF_CONVERSIONS)
		{
			Init_Value=0;//then, samples shall be sent from point 0
		} else
		{
			Init_Value=position[MIN][0][0];//if not, samples are sent starting at the first minimum point
		}
	}else if(Max_Period>(MAX_NUM_OF_CONVERSIONS/20))///If the period is between 5% and 10% of the total samples, sends 2000 samples
	{
		Number_of_Samples=(MAX_NUM_OF_CONVERSIONS/5);
		Init_Value=position[MIN][0][0];
	}else if(Max_Period<=(MAX_NUM_OF_CONVERSIONS/20))///If the period is lower than 5% of the total samples, sends 1000 samples
	{
		Number_of_Samples=(MAX_NUM_OF_CONVERSIONS/10);
		Init_Value=position[MIN][0][0];
	}


	return HAL_OK;
}

/**
 *@brief Transmit data using serial port
 *@retval int Hal_OK
 */
int Transmit (void)
{
	unsigned int cont=0;

	if(N_Samples_is_Set==FALSE)///if the number of samples was not set by the user
	{
		GetSamplesAuto();
	}else
	{
		Init_Value=0;///if the user set the number of samples, sends that amount of samples, beginning on the first one
	}

	for(channel=0;channel<Number_of_Channels;channel++)///Sends each channel period
	{
		pointer=&period[channel];
		HAL_UART_Transmit(&huart2, pointer, 2, COM_TIMEOUT);
	}

	pointer=&Number_of_Samples;///Sends the number of samples to be sent
	HAL_UART_Transmit(&huart2, pointer, 2, COM_TIMEOUT);


	pointer=&array[Init_Value];///Sends the samples
	for (cont=0;cont<(Number_of_Samples*Number_of_Channels);cont++)
	{
		HAL_UART_Transmit(&huart2, pointer, 1, COM_TIMEOUT);
		pointer++;

		/**During transmission, timed pauses are made in the data sending,
		 * to adjust the clock and avoid transmission errors
		 * (theoretical baudrate is not the same as the practical, occasioning in loss of data)
		 */
		if(cont%1000==0)
		{
			for(int aux=0;aux<BAUD_115200_WAIT_TIME;aux++){}
		}
	}

	cont=0;
	pointer=&data;

	return HAL_OK;

}

/**
 * @brief Analises the collected samples, obtaining, for each channel, the period of the signal
 * @retval int HAL_OK
 */

int WaveAnalysis(void)
{
	int aux=0;///auxiliary variable
	unsigned int cont_min=0;
	unsigned int cont_max=0;
	unsigned int diff_bw_min[Number_of_Channels];
	unsigned int diff_bw_max[Number_of_Channels];

	unsigned char Next_is=MIN;
	unsigned char value[2][Number_of_Channels];
	unsigned char margin[Number_of_Channels];

	for (channel=0;channel<Number_of_Channels;channel++)///course through all channels
	{
		value[MIN][channel]=255;
		value[MAX][channel]=0;
		diff_bw_max[channel]=0;
		diff_bw_min[channel]=0;
		position[MIN][0][channel]=0;
		position[MAX][0][channel]=0;

		for(aux=channel;aux<(MAX_NUM_OF_CONVERSIONS*Number_of_Channels);aux+=Number_of_Channels)///Finds the maximum and minimum value for this channel
		{
			if(array[aux]<value[MIN][channel])//new minimum value
			{
				value[MIN][channel]=array[aux];
			}
			if(array[aux]>value[MAX][channel])//new maximum value
			{
				value[MAX][channel]=array[aux];
			}
		}

		margin[channel] =(int)((value[MAX][channel]-value[MIN][channel])/10);///margin to classify a value as a minimum or maximum = 10% of the signal's amplitude

		if(margin[channel]<=1)///Checks whether the margin is too small
		{
			period[channel]=0;
		}else{
			Next_is=MIN;
			cont_max=0;
			cont_min=0;

			///For each channel:

			/**
			 * Verify the position of all minimum and maximum values, alternately and <br>
			 * stores these positions in "position[MAX or MIN][value][channel]". <br>
			 * Verifies the maximum time between two subsequent maximums or minimums. This time will be the period of the signal (worst scenario)
			 */

			for(aux=channel;aux<(MAX_NUM_OF_CONVERSIONS*Number_of_Channels);aux+=Number_of_Channels)
			{
				if((array[aux]>=(value[MAX][channel] - margin[channel]))&&(Next_is==MAX))///New maximum value (with margin)
				{
					position[MAX][cont_max][channel]=aux;
					if(cont_max>=1)///already encountered 2 maximum values?
					{
						if(diff_bw_max[channel]<(position[MAX][cont_max][channel]-position[MAX][cont_max-1][channel]))///always gets the greater difference between two maximum points
						{
							diff_bw_max[channel]=position[MAX][cont_max][channel]-position[MAX][cont_max-1][channel];
						}
					}
					Next_is=MIN;
					cont_max++;
				}
				if((array[aux]<=(value[MIN][channel] + margin[channel]))&&(Next_is==MIN))///New minimum value (with margin)
				{
					position[MIN][cont_min][channel]=aux;
					if(cont_min>=1)///already encountered 2 minimum values?
					{
						if(diff_bw_min[channel]<(position[MIN][cont_min][channel]-position[MIN][cont_min-1][channel]))///always gets the greater difference between two minimum points
						{
							diff_bw_min[channel]=position[MIN][cont_min][channel]-position[MIN][cont_min-1][channel];
						}
					}
					Next_is=MAX;
					cont_min++;
				}
			}

			if(diff_bw_max[channel]>=diff_bw_min[channel])///period will be the greatest difference between two subsequent maximum or minimum points
			{
				period[channel]=diff_bw_max[channel]/Number_of_Channels;

			}else
			{
				period[channel]=diff_bw_min[channel]/Number_of_Channels;
			}
		}

	}

	return HAL_OK;
}


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
  MX_ADC1_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */

  ///Request data receiving via serial
  __HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);
  HAL_UART_Receive_IT(&huart2, pointer, 1);

  ///Sets the default sampling rate (500kHz)
  __HAL_TIM_SET_AUTORELOAD(&htim8,SR_Available[5]);

  __HAL_ADC_ENABLE(&hadc1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 240;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @brief ADC's end of conversion interrupt
 * @retval none
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
		HAL_TIM_Base_Stop(&htim8);//Stops the trigger for the ADC (TIM 8)
		ADC_conversions=0;//Resets the number of conversions
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
		if(WaveAnalysis()==HAL_OK)//Analyzes the waveform obtained
		{
			Transmit();//Transmit data
		}
		__HAL_ADC_CLEAR_FLAG(&hadc1,ADC_FLAG_OVR);//Clear overrun flag (preventively)
		__HAL_ADC_CLEAR_FLAG(&hadc1,ADC_FLAG_EOC);//Clear the end of conversion flag
	//}
}

/**
 * @brief Serial data received interrupt
 * @param huart UART_Handle
 * @retval none
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	///Checks whether the byte received was:

	///Start of conversion (character 'S')
	if(data==START_OF_CONVERSION)
	{
		ADC_conversions=0;
		__HAL_ADC_CLEAR_FLAG(&hadc1,ADC_FLAG_OVR);//clears overrun flag
		__HAL_TIM_CLEAR_FLAG(&htim8,TIM_FLAG_UPDATE);

		HAL_TIM_GenerateEvent(&htim8, TIM_EVENTSOURCE_UPDATE);//refreshes TIM peripheral

		HAL_ADC_Start_DMA(&hadc1, &array[0], MAX_NUM_OF_CONVERSIONS*Number_of_Channels);
		HAL_TIM_Base_Start(&htim8);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
	}

	if(Config>=2)///Some of the 2 byte of the number of samples
	{
		Number_of_Samples|= (data<<((Config-2)*8));
		Config++;
		if(Config==4)
		{
			Config=FALSE;
		}
	}
	if(Config==TRUE)///Configuration byte
	{
		Config=0;
		Config|=(data & 0xF);//sample rate is the first 4 bits of data
		__HAL_TIM_SET_AUTORELOAD(&htim8,SR_Available[(int)Config]);//sets sample rate

		Number_of_Channels=((data>>4)&(0x7));
		ADC1->SQR1&=~ADC_SQR1_L;
		ADC1->SQR1|=((Number_of_Channels-1)<<20);//sets the number of channels per conversion

		N_Samples_is_Set = (data>>7);
		Number_of_Samples=0;

		Config=2;
	}
	if(data==DEFINE_CONFIG)///Define the configuration (character 'C')
	{
		Config=TRUE;
	}

	HAL_UART_Receive_IT(&huart2, pointer, 1);
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
