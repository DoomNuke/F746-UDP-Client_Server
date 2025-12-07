/*
 * Peripherials_Check.c
 *
 *  Created on: Nov 9, 2025
 *      Author: doomnuke
 */
#include "Peripherials_Check.h"

#include <string.h>


#include "../Main_Server/Main_Server.h"

/*
 * The status is the value of success or failure on each peripherial checked
 * sTime and sDate used for RTC
 */
volatile uint8_t status;

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;


/*
 * Time and date
 */
void GetTimeAndDate(void)
{
	char msg[MSG_LEN];
	int msg_len;

	if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	if(HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}

	msg_len = snprintf(msg, sizeof(msg), "Current Date: %02d%02d%02d\r\n", sDate.WeekDay, sDate.Month, sDate.Year + 2000);
	HAL_UART_Transmit(&huart3, (uint8_t*)msg, msg_len, 500);

	msg_len = snprintf(msg, sizeof(msg), "Current Time: %02d%02d%02d\r\n",  sTime.Hours, sTime.Minutes, sTime.Seconds);
	HAL_UART_Transmit(&huart3, (uint8_t*)msg, msg_len, 500);
}

/*
 * ADC Functions,
 * ADC_Check = we check the ADC with polling, get the value, set the result to ADC_OK and we stop and return the result
 * ADC_Result = calls ADC_Check and determines by the result if it was successful or not, either way it returns a
 * value so it can be passed on to be sent to the user
 */
ADC_Result ADC_Check(void)
{
	ADC_Result Result;
	Result.status = ADC_ERROR;
	Result.measured = 0;
	Result.known_value = 0;

	HAL_ADC_Start(&hadc1);
	if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
	{
		Result.measured = HAL_ADC_GetValue(&hadc1);
		Result.status = ADC_OK;
	}

	HAL_ADC_Stop(&hadc1);
	return Result;
}

void ADC_Outcome(newTask *Task)
{
	ADC_Result Result = ADC_Check();
	char msg[MSG_LEN];
	int msg_len;
	uint32_t known_val = Task->adc_known_val;

	Result.known_value = known_val;

	if(Result.status == ADC_ERROR)
	{
		msg_len = snprintf(msg, sizeof(msg), "Operation has not been completed\r\n");
		return;
	}

	if (Result.measured == Result.known_value)
	{
		msg_len = snprintf(Task->adc_result, sizeof(Task->adc_result) , "ADC OK: Expeceted:%lu Measured:%lu \r\n", result.known_value, result.measured);
	}
	else
	{
		msg_len = snprintf(msg, sizeof(msg), "ADC conversion error, Expected: %lu, Measured: %lu\r\n", result.known_value, result.measured);
		HAL_UART_Transmit(&huart3, (uint8_t*)msg, msg_len, 500);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if(hadc->Instance == ADC3)
	{
		status = Status_Success;
	}
}

/*
 * I2C = we wait for the ADC result first, we enter a loop of 5 retries,
 * we first set the volatile value to failure before we start to transmit it via DMA,
 * we wait until the callback is updated from status_failure to success and if we do we break off the loop
 * we return the date and time we return the ADC values and the test values of the peripherial
 */

void I2C_Test(void)
{
	newTask Task;
	ADC_Result Result;
	char msg[MSG_LEN];
	int retries = 5;
	int msg_len;
	uint8_t result [] = {0x00, 0x01, 0x02};
	uint16_t DevAddress = 0x10;
	uint32_t Peri_Measured_Time;

	Result = ADC_Check();
	Task.adc_known_val = Result.measured;


	for(int i = 0; i < sizeof(retries); i++)
	{
		status = Status_Failure;
		Start_Time();
		HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit_DMA(&hi2c2, DevAddress, result, sizeof(result));

		if(ret != HAL_OK) {
		    msg_len = snprintf(msg, sizeof(msg), "I2C DMA transfer could not start...\r\n");
		    HAL_UART_Transmit(&huart3, (uint8_t *)msg, msg_len, 500);
		    return;
		}

		/*
		 * waiting until the callback updates status
		 */
		while(status == Status_Failure) {
		}

		Stop_Time();
		Peri_Measured_Time = Measure_Time();
		Task.Measured_Time = Peri_Measured_Time;

		if(status == Status_Success)
		{
			break;
		}
		else
		{
			msg_len = snprintf(msg, sizeof(msg), "I2C has not been sent, sending again...");
			HAL_UART_Transmit(&huart3, (uint8_t*)msg, msg_len, 500);
			return;
		}
	}

	GetTimeAndDate();
	ADC_Outcome(&Task);
	msg_len = snprintf(msg, sizeof(msg), "Time:%lu\r\n I2C values: %u | %u | %u\r\n",Task.Measured_Time,result[1], result[2], result[3]);
}

void HAL_I2C_TxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C2)
	{
		status = Status_Success;
	}
}


/*
 * SPI = we wait for the ADC result first, we enter a loop of 5 retries,
 * we first set the volatile value to failure before we start to transmit it via DMA,
 * we wait until the callback is updated from status_failure to success and if we do we break off the loop
 * we return the date and time we return the ADC values and the test values of the peripherial
 */

void SPI_Test(void)
{
	newTask Task;
	ADC_Result Result;
	char msg[MSG_LEN];
	int retries = 5;
	int msg_len;
	uint8_t SPI_Buf_TX[] = "Hey SPI!\r\n";
	uint32_t Peri_Measured_Time;

	Result = ADC_Check();
	Task.adc_known_val = Result.measured;


	for(int i = 0; i < retries; i++)
	{
		status = Status_Failure;
		Start_Time();
		HAL_StatusTypeDef ret = HAL_SPI_Transmit_DMA(&hspi1, SPI_Buf_TX, sizeof(SPI_Buf_TX));
		if(ret != HAL_OK)
		{
			msg_len = snprintf(msg, sizeof(msg), "SPI DMA transfer could not start...\r\n");
			HAL_UART_Transmit(&huart3, (uint8_t *)msg, msg_len, 500);
			return;
		}

		while(status == Status_Failure) {
		}

		Stop_Time();
		Peri_Measured_Time = Measure_Time();
		Task.Measured_Time = Peri_Measured_Time;

		if (status == Status_Success)
		{

			break;
		}
		else
		{
			msg_len = snprintf(msg, sizeof(msg), "SPI has not been sent, sending again...\r\n");
			HAL_UART_Transmit(&huart3, (uint8_t*)msg, msg_len, 500);
			return;
		}
	}

	GetTimeAndDate();
	ADC_Outcome(&Task);
	msg_len = snprintf(msg, sizeof(msg), "Time:%lu\r\n SPI values: %s\r\n",Task.Measured_Time,SPI_Buf_TX);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance == SPI1)
	{
		status = Status_Success;
	}
}

/*
 * UART = we wait for the ADC result first, we enter a loop of 5 retries,
 * we first set the volatile value to failure before we start to transmit it,
 * we wait until the callback is updated from status_failure to success and if we do we break off the loop
 * we return the date and time we return the ADC values and the test values of the peripherial
 */


void UART_Test(void)
{
	newTask Task;
	ADC_Result Result;
	char msg[MSG_LEN];
	char UART_MSG[] = "Hello UART!\r\n";
	int retries = 5;
	int msg_len;
	uint32_t Peri_Measured_Time;

	Result = ADC_Check();
	Task.adc_known_val = Result.measured;


	for(int i = 0; i < retries; i++)
	{
		status = Status_Failure;
		HAL_StatusTypeDef ret = HAL_UART_Transmit(&huart5, (uint8_t *)UART_MSG, strlen(UART_MSG), 500);
		if(ret != HAL_OK)
		{
			msg_len = snprintf(msg, sizeof(msg), "UART transfer could not start...\r\n");
			HAL_UART_Transmit(&huart3, (uint8_t *)msg, msg_len, 500);
			return;
		}

		while(status == Status_Failure) {
		}

		Stop_Time();
		Peri_Measured_Time = Measure_Time();
		Task.Measured_Time = Peri_Measured_Time;

		if (status == Status_Success)
		{
			break;
		}
		else
		{
			msg_len = snprintf(msg, sizeof(msg), "UART has not been sent, sending again...\r\n");
			HAL_UART_Transmit(&huart3, (uint8_t*)msg, msg_len, 500);
			return;
		}
	}

	GetTimeAndDate();
	ADC_Outcome(&Task);
	msg_len = snprintf(msg, sizeof(msg), "Time:%lu\r\n UART values: %s\r\n",Task.Measured_Time,UART_MSG);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == UART5)
	{
		status = Status_Success;
	}
}
