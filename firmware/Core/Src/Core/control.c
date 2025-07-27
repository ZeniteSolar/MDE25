#include "main.h"
#include <stdio.h>

#include "config.h"
#include "utils.h"

#include "Core/control.h"
#include "Core/sense.h"

/**
 * ============================
 * Locals
 * ============================
 */

static uint8_t setpoint_locked = 0U;
static float setpoint = 0.0f;
static float setpoint_avg[CONTROL_SETPOINT_AVG_WINDOW] = {0.0f};
static float p_gain = CONTROL_INITIAL_GAIN_P;
static float i_gain = CONTROL_INITIAL_GAIN_I;
static float d_gain = CONTROL_INITIAL_GAIN_D;
static float control_error = 0.0f;
static feedback_fn_t feedback_fn = NULL;
static actuate_fn_t actuate_fn = NULL;

/**
 * ============================
 * Forward Declarations
 * ============================
 */

static float control_pid_compute(float setpoint, float feedback, float *computed_error);

/**
 * ============================
 * Implementations
 * ============================
 */

void control_init(feedback_fn_t feedback, actuate_fn_t actuate)
{
  feedback_fn = feedback;
  actuate_fn = actuate;
}

void control_update(void)
{
  /* Must be initialized already */
  if (feedback_fn == NULL || actuate_fn == NULL)
  {
    return;
  }

  float control_point = control_pid_compute(setpoint, feedback_fn(), &control_error);

  /* Clamp control point */
  control_point = clampf(control_point, CONTROL_ACTUATE_MIN_VALUE, CONTROL_ACTUATE_MAX_VALUE);
  /* Pass control point through range */
  control_point = passf(control_point, 0.0f, CONTROL_ACTUATE_CUT_MIN_VALUE, CONTROL_ACTUATE_CUT_MAX_VALUE);

#if DEBUG_CONTROL_ENABLE
  printf("S %.5f, F %.5f, E %.5f, D %.5f\n", setpoint, feedback_fn(), control_error, control_point);
#endif

  actuate_fn(control_point);
}

void control_set_setpoint(float point)
{
  if (setpoint_locked == 1U)
  {
    return;
  }

  /* Update setpoint average buffer */
  for (uint8_t i = 0; i < CONTROL_SETPOINT_AVG_WINDOW - 1; i++)
  {
    setpoint_avg[i] = setpoint_avg[i + 1];
  }
  setpoint_avg[CONTROL_SETPOINT_AVG_WINDOW - 1] = point;

  /* Compute setpoint average from buffer */
  float setpoint_avg_value = 0.0f;
  for (uint8_t i = 0; i < CONTROL_SETPOINT_AVG_WINDOW; i++)
  {
    setpoint_avg_value += setpoint_avg[i];
  }
  setpoint_avg_value /= CONTROL_SETPOINT_AVG_WINDOW;

  /* Update setpoint */
  setpoint = setpoint_avg_value;
}

float control_get_setpoint(void)
{
  return setpoint;
}

float control_get_error(void)
{
  return control_error;
}

void control_reset_setpoint(float point)
{
  for (uint8_t i = 0; i < CONTROL_SETPOINT_AVG_WINDOW; i++)
  {
    setpoint_avg[i] = point;
  }
  setpoint = point;
}

void control_lock_setpoint(float point)
{
  setpoint = point;
  setpoint_locked = 1U;
}

void control_unlock_setpoint(void)
{
  setpoint_locked = 0U;
}

void control_set_p_gain(float gain)
{
  p_gain = gain;
}

void control_set_i_gain(float gain)
{
  i_gain = gain;
}

void control_set_d_gain(float gain)
{
  d_gain = gain;
}

static float control_pid_compute(float setpoint, float feedback, float *computed_error)
{
  static float integral = 0.0f;
  static float error_previous = 0.0f;

  /* Compute error */
  float error = setpoint - feedback;
  if (computed_error != NULL)
  {
    *computed_error = error;
  }

  /* Compute time step */
  float dt = sense_get_period();

  /* Compute integral */
  integral += error * dt;

  /* Clamp integral */
  integral = clampf(integral, CONTROL_INTEGRAL_MIN_VALUE, CONTROL_INTEGRAL_MAX_VALUE);

  /* Compute derivative */
  float derivative = (error - error_previous) / dt;

  /* Update previous error */
  error_previous = error;

  /* Compute PID */
  return p_gain * error + i_gain * integral + d_gain * derivative;
}
