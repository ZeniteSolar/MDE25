#include <stdint.h>

#include "adc.h"
#include "tim.h"
#include "Core/sense.h"
#include "Core/control.h"
#include "stm32l4xx_ll_tim.h"

typedef enum {
  ADC_RANK_CURRENT_CONTROL_POINT = 0U,
  ADC_RANK_INPUT_VOLTAGE = 1U,
  ADC_RANK_OUTPUT_VOLTAGE = 2U,
  ADC_RANK_INPUT_CURRENT = 3U,
  ADC_RANK_COUNT = 4U,
} adc_ranks_t;

/**
 * ============================
 * Locals
 * ============================
 */

uint16_t adc_dma_buffer[ADC_RANK_COUNT];

/**
 * ============================
 * Forward Declarations
 * ============================
 */


/**
 * ============================
 * Implementations
 * ============================
 */

void sense_init(void)
{
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buffer, ADC_RANK_COUNT);
  HAL_TIM_Base_Start(&htim2);
}

void sense_set_sampling_frequency(uint32_t frequency)
{
  uint32_t arr = HAL_RCC_GetPCLK1Freq() / frequency;
  LL_TIM_SetPrescaler(htim2.Instance, 0U);
  LL_TIM_SetAutoReload(htim2.Instance, arr);
  LL_TIM_GenerateEvent_UPDATE(htim2.Instance);
}

uint32_t sense_get_sampling_frequency(void)
{
  return HAL_RCC_GetPCLK1Freq() / LL_TIM_GetAutoReload(htim2.Instance);
}

float sense_get_input_voltage(void)
{
  return 0.001150f * (float)adc_dma_buffer[ADC_RANK_INPUT_VOLTAGE] + -0.088064f;
}

float sense_get_output_voltage(void)
{
  return 0.002327f * (float)adc_dma_buffer[ADC_RANK_OUTPUT_VOLTAGE] + -73.731254f;
}

float sense_get_input_current(void)
{
  return (float)adc_dma_buffer[ADC_RANK_INPUT_CURRENT] * 3.3f / 65535.0f;
}

float sense_get_control_point(void)
{
  return (float)adc_dma_buffer[ADC_RANK_CURRENT_CONTROL_POINT] * 2.0f / 65535.0f - 1.0f;
}

/**
 * ============================
 * Interruptions
 * ============================
 */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  /** We only care about ADC1 */
  if (hadc != &hadc1) {
    return;
  }

  control_update();
}
