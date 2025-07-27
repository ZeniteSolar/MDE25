#include "tim.h"
#include "stm32l4xx_ll_tim.h"

#include "config.h"
#include "Core/status.h"
#include "Core/pwm.h"

typedef void (*on_status_finished_fn_t)(void);

/**
 * ============================
 * Locals
 * ============================
 */

static uint8_t device_connected = 1U;

static uint16_t status_tone_repetitions = 0U;
static float status_tone_pwm = STATUS_TONE_PWM_VOLUME;

/** Pointer for callback when finished */
static on_status_finished_fn_t status_on_finished_fn = NULL;

/**
 * ============================
 * Forward Declarations
 * ============================
 */

static void status_init(void);
static void play_tone(float duration_s, float frequency_hz, on_status_finished_fn_t next_fn);
static void status_on_connect_1(void);
static void status_on_connect_2(void);
static void status_on_disconnect_1(void);
static void status_on_disconnect_2(void);
static void status_on_disconnected(void);

/**
 * ============================
 * Implementations
 * ============================
 */

/** On MIC Connected sequence */

void status_on_connect(void)
{
  /** 3 beeps - LOW -> MEDIUM -> HIGH */
  if (device_connected == 1U)
  {
    return;
  }

  device_connected = 1U;
  /** First tone LOW */
  play_tone(STATUS_TONE_DURATION_S, STATUS_TONE_FREQUENCY_LOW_HZ, status_on_connect_1);
}

static void status_on_connect_1(void)
{
  /** Second tone MEDIUM */
  play_tone(STATUS_TONE_DURATION_S, STATUS_TONE_FREQUENCY_MEDIUM_HZ, status_on_connect_2);
}

static void status_on_connect_2(void)
{
  /** Third and final tone HIGH */
  play_tone(STATUS_TONE_DURATION_S * 2.0f, STATUS_TONE_FREQUENCY_HIGH_HZ, NULL);
}

/** On MIC Disconnected sequence */

void status_on_disconnect(void)
{
  /** 3 beeps - HIGH -> MEDIUM -> LOW */
  if (device_connected == 0U)
  {
    status_on_disconnected();
    return;
  }

  device_connected = 0U;
  /** First tone HIGH */
  play_tone(STATUS_TONE_DURATION_S, STATUS_TONE_FREQUENCY_HIGH_HZ, status_on_disconnect_1);
}

static void status_on_disconnect_1(void)
{
  /** Second tone MEDIUM */
  play_tone(STATUS_TONE_DURATION_S, STATUS_TONE_FREQUENCY_MEDIUM_HZ, status_on_disconnect_2);
}

static void status_on_disconnect_2(void)
{
  /** Third and final tone LOW */
  play_tone(STATUS_TONE_DURATION_S * 2.0f, STATUS_TONE_FREQUENCY_LOW_HZ, NULL);
}

/** On device without connection sequence */

void status_on_disconnected(void)
{
  /** Single beep */
  play_tone(STATUS_TONE_DURATION_S, STATUS_TONE_FREQUENCY_HIGH_HZ, NULL);
}

uint8_t status_get_connected(void)
{
  return device_connected;
}

void status_on_tim_update(void)
{
  if (status_tone_repetitions > 0U)
  {
    status_tone_repetitions--;
    status_tone_pwm = -status_tone_pwm;
    pwm_set_duty(status_tone_pwm);

    /** Retrigger the timer */
    status_init();
  }
  else
  {
    pwm_set_duty(0.0f);
    if (status_on_finished_fn != NULL)
    {
      status_on_finished_fn();
    }
  }
}

void status_init(void)
{
  /** TIM6 is being used as status timer */
  HAL_TIM_Base_Stop_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim6);
}

static void play_tone(float duration_s, float frequency_hz, on_status_finished_fn_t next_fn)
{
  float single_pulse_duration_s = 1.0f / frequency_hz;
  float clk = (float)HAL_RCC_GetPCLK1Freq() / 2.0f;
  uint32_t top = (uint32_t)(clk * single_pulse_duration_s);

  status_tone_repetitions = (uint16_t)(duration_s / single_pulse_duration_s);
  LL_TIM_SetAutoReload(htim6.Instance, top);
  status_on_finished_fn = next_fn;
  status_init();
}
