#include <math.h>
#include <stdint.h>

#include "stm32l4xx_ll_tim.h"

#include "config.h"
#include "utils.h"
#include "tim.h"

#include "Core/pwm.h"
#include "Core/sense.h"

/**
 * ============================
 * Locals
 * ============================
 */

static float pwm_duty = 0.0f;
static float pwm_effective_duty = 0.0f;
static uint8_t pwm_locked = 0U;

/**
 * ============================
 * Forward Declarations
 * ============================
 */

// static void pwm_invert_switching_side(void);
// static void pwm_tim_update_signal(void);
static float pwm_get_duty_from_input_voltage(float input_duty, float input_voltage);

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
  if (pwm_locked)
  {
    return;
  }

  pwm_set_duty_forced(duty);
}

void pwm_set_duty_forced(float duty)
{
  duty = clampf(duty, -1.0f, 1.0f);
  pwm_duty = duty;
  pwm_effective_duty = pwm_get_duty_from_input_voltage(duty, sense_get_input_voltage());

  uint16_t arr = LL_TIM_GetAutoReload(htim1.Instance);
  uint16_t duty_cycle = arr - (uint16_t)(arr * fabs(pwm_effective_duty));

  LL_TIM_OC_SetCompareCH2(htim1.Instance, pwm_effective_duty > 0.0f ? arr - duty_cycle : (uint16_t)0U);
  LL_TIM_OC_SetCompareCH1(htim1.Instance, pwm_effective_duty > 0.0f ? (uint16_t)0U : arr - duty_cycle);

  LL_TIM_GenerateEvent_UPDATE(htim1.Instance);
}

float pwm_get_duty(void)
{
  return pwm_duty;
}

float pwm_get_effective_duty(void)
{
  return pwm_effective_duty;
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

float pwm_get_period(void)
{
  return 1.0f / (float)pwm_get_frequency();
}

void pwm_lock(void)
{
  pwm_locked = 1U;
}

void pwm_unlock(void)
{
  pwm_locked = 0U;
}

void pwm_on_tim_period_elapsed(void)
{
  //pwm_tim_update_signal();
}

static float pwm_get_duty_from_input_voltage(float input_duty, float input_voltage)
{
  float clamped_input_voltage = clampf(input_voltage, PWM_MIN_IN_VOLTAGE, PWM_MAX_IN_VOLTAGE);
  float max_duty = PWM_MAX_OUT_VOLTAGE / clamped_input_voltage;
  return mapf(input_duty, -1.0f, 1.0f, -max_duty, max_duty);
}

// static void pwm_invert_switching_side(void)
// {
//   uint16_t cc1 = LL_TIM_OC_GetCompareCH1(htim1.Instance);
//   uint16_t cc2 = LL_TIM_OC_GetCompareCH2(htim1.Instance);
//   LL_TIM_OC_SetCompareCH2(htim1.Instance, cc1);
//   LL_TIM_OC_SetCompareCH1(htim1.Instance, cc2);
// }

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
// static void pwm_tim_update_signal(void)
// {
//   static uint16_t repetitions_counter = 0U;
//
//   if (repetitions_counter >= PWM_INVERT_N_REPETITIONS)
//   {
//     repetitions_counter = 0U;
//     pwm_invert_switching_side();
//     return;
//   }
//   repetitions_counter++;
// }
