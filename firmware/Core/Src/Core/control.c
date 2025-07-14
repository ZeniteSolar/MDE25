#include "main.h"
#include "Core/control.h"
#include "Core/sense.h"
#include "config.h"

/**
 * ============================
 * Locals
 * ============================
 */

static float setpoint = 0.0f;
static float p_gain = 0.1f;
static float i_gain = 0.0f;
static float d_gain = 0.0f;
static feedback_fn_t feedback_fn = NULL;
static actuate_fn_t actuate_fn = NULL;

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

void control_init(feedback_fn_t feedback, actuate_fn_t actuate)
{
  feedback_fn = feedback;
  actuate_fn = actuate;
}

float control_pid_compute(float setpoint, float feedback)
{
  static float integral = 0.0f;
  static float error_previous = 0.0f;

  /* Compute error */
  float error = setpoint - feedback;

  /* Compute time step */
  float dt = 1.0f / (float)sense_get_sampling_frequency();

  /* Compute integral */
  integral += error * dt;

  /* Clamp integral */
  if (integral > CONTROL_INTEGRAL_MAX_VALUE) {
    integral = CONTROL_INTEGRAL_MAX_VALUE;
  }
  if (integral < CONTROL_INTEGRAL_MIN_VALUE) {
    integral = CONTROL_INTEGRAL_MIN_VALUE;
  }

  /* Compute derivative */
  float derivative = (error - error_previous) / dt;

  /* Update previous error */
  error_previous = error;

  /* Compute PID */
  return p_gain * error + i_gain * integral + d_gain * derivative;
}

void control_update(void)
{
  if (feedback_fn == NULL || actuate_fn == NULL) {
    return;
  }

  float control_point = control_pid_compute(setpoint, feedback_fn());

  /* Clamp control point */
  if (control_point > CONTROL_ACTUATE_MAX_VALUE) {
    control_point = CONTROL_ACTUATE_MAX_VALUE;
  }

  if (control_point < CONTROL_ACTUATE_MIN_VALUE) {
    control_point = CONTROL_ACTUATE_MIN_VALUE;
  }

  actuate_fn(control_point);
}

void control_set_setpoint(float point)
{
  setpoint = point;
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
