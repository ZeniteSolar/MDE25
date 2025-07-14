#include <math.h>
#include <stdint.h>

#include "config.h"
#include "Core/pwm.h"
#include "stm32l4xx_ll_tim.h"
#include "tim.h"

/**
 * ============================
 * Forward Declarations
 * ============================
 */

static void pwm_invert_switching_side(void);
static void pwm_tim_update_signal(void);

/**
 * ============================
 * Implementations
 * ============================
 */

void pwm_init(void)
{
  /** Start base TIME and interrupts */
  HAL_TIM_Base_Start_IT(&htim1);

  /** Start PWM channels */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

  /** Set initial duty cycle */
  pwm_set_duty(INITIAL_PWM_DUTY);
  pwm_set_frequency(INITIAL_PWM_FREQUENCY);
}

void pwm_set_duty(float duty)
{
  uint16_t arr = LL_TIM_GetAutoReload(htim1.Instance);
  uint16_t duty_cycle = arr - (uint16_t)(arr * fabs(duty));

  LL_TIM_OC_SetMode(htim1.Instance, LL_TIM_CHANNEL_CH1, duty > 0.0f ? LL_TIM_OCMODE_PWM2 : LL_TIM_OCMODE_PWM1);
  LL_TIM_OC_SetMode(htim1.Instance, LL_TIM_CHANNEL_CH2, duty > 0.0f ? LL_TIM_OCMODE_PWM1 : LL_TIM_OCMODE_PWM2);

  uint16_t cc1 = LL_TIM_OC_GetCompareCH1(htim1.Instance);
  LL_TIM_OC_SetCompareCH1(htim1.Instance, cc1 == 0U ? duty_cycle : 0U);
  LL_TIM_OC_SetCompareCH2(htim1.Instance, cc1 == 0U ? 0U : duty_cycle);

  LL_TIM_GenerateEvent_UPDATE(htim1.Instance);
}

float pwm_get_duty(void)
{
  uint16_t arr = LL_TIM_GetAutoReload(htim1.Instance);
  uint16_t cc1 = LL_TIM_OC_GetCompareCH1(htim1.Instance);
  uint16_t cc2 = LL_TIM_OC_GetCompareCH2(htim1.Instance);
  float multiplier = LL_TIM_OC_GetMode(htim1.Instance, LL_TIM_CHANNEL_CH1) == LL_TIM_OCMODE_PWM2 ? 1.0f : -1.0f;

  return ((float)(arr - cc1 + cc2) / (float)arr) * multiplier;
}

void pwm_set_frequency(uint32_t frequency)
{
  /** Only allow frequencies above 1300Hz to avoid pre scalar */
  if (frequency < 1300U)
  {
    return;
  }

  float duty = pwm_get_duty();

  uint32_t arr = HAL_RCC_GetPCLK1Freq() / frequency;
  LL_TIM_SetPrescaler(htim1.Instance, 0U);
  LL_TIM_SetAutoReload(htim1.Instance, arr);
  LL_TIM_GenerateEvent_UPDATE(htim1.Instance);

  pwm_set_duty(duty);
}

uint32_t pwm_get_frequency(void)
{
  return HAL_RCC_GetPCLK1Freq() / LL_TIM_GetAutoReload(htim1.Instance);
}

static void pwm_invert_switching_side(void)
{
  uint16_t cc1 = LL_TIM_OC_GetCompareCH1(htim1.Instance);
  uint16_t cc2 = LL_TIM_OC_GetCompareCH2(htim1.Instance);
  LL_TIM_OC_SetCompareCH2(htim1.Instance, cc1);
  LL_TIM_OC_SetCompareCH1(htim1.Instance, cc2);
}

/**
 * @brief Since bootstrap capacitor cannot keep 100% duty cycle continuously, we need to invert the switching side of
 * the bridge from time to time.
 *
 *     |---N---|---N---|
 *  H1: ⎻_⎻_⎻_⎻_⎻⎻⎻⎻⎻⎻⎻⎻
 *  L1: _⎻_⎻_⎻_⎻________
 *  H2: _________⎻_⎻_⎻_⎻
 *  L2: ⎻⎻⎻⎻⎻⎻⎻⎻⎻_⎻_⎻_⎻_
 */
static void pwm_tim_update_signal(void)
{
  static uint16_t repetitions_counter = 0U;

  if (repetitions_counter >= PWM_INVERT_N_REPETITIONS)
  {
    repetitions_counter = 0U;
    pwm_invert_switching_side();
    return;
  }
  repetitions_counter++;
}

/**
 * ============================
 * Interruptions
 * ============================
 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == htim1.Instance)
  {
    pwm_tim_update_signal();
  }
}
