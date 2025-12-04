/*
 * Peripherials_Check.h
 *
 *  Created on: Nov 9, 2025
 *      Author: doomnuke
 */

#ifndef PERIPHERIALS_CHECK_PERIPHERIALS_CHECK_H_
#define PERIPHERIALS_CHECK_PERIPHERIALS_CHECK_H_


#define Status_Success 0x83
#define Status_Failure 0x84


#include "../Main_Server/Main_Server.h"

/*
 * Peripherials Handlers
 */

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

extern ADC_HandleTypeDef hadc1;

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;
extern DMA_HandleTypeDef hdma_i2c2_tx;
extern DMA_HandleTypeDef hdma_i2c4_rx;


extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;

extern TIM_HandleTypeDef htim1;

extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart3;

/*
 * Setup part of enums, structs
 */

typedef enum {
    ADC_OK = 0,
    ADC_FAIL = 1,
    ADC_ERROR = 2
} ADC_Status;


typedef struct ADC_Res{
  ADC_Status status;
  uint32_t known_value;
  uint32_t measured;
}ADC_Result;

/*
 * Variables for callbacks
 */

extern volatile uint8_t status;

/*
 * Functions + helper functions definitions, and declaring handlers
 */

/*
 * Helper Functions for using TIM1 for measurements of each peripherial check
 * using inline instead of code-smelling the whole peripherial checkups
 */

static inline void Start_Time(void)
{
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	HAL_TIM_Base_Start(&htim1);
}

static inline void Stop_Time(void)
{
	HAL_TIM_Base_Stop(&htim1);
}

static inline uint32_t Measure_Time(void)
{
	return __HAL_TIM_GET_COUNTER(&htim1);
}

void GetTimeAndDate(void);
ADC_Result ADC_Check(void);
void ADC_Outcome(newTask *Task);
void I2C_Test(void);
void SPI_Test(void);
void UART_Test(void);




#endif /* PERIPHERIALS_CHECK_PERIPHERIALS_CHECK_H_ */
