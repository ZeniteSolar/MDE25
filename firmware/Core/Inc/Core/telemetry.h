#ifndef __CORE_TELEMETRY_H__
#define __CORE_TELEMETRY_H__

#include "main.h"
#include <stdint.h>

typedef struct
{
  uint16_t preamble;

  /** Sensing Metrics */
  float sense_input_voltage;
  float sense_input_current;
  float sense_output_voltage;
  float sense_control_point;

  /** Control Metrics */
  float control_setpoint;
  float control_error;

  /** Actuation Metrics */
  float pwm_duty;
  float pwm_effective_duty;

  /** Status Metrics */
  uint8_t status_connected;
} telemetry_t;

/**
 * @brief Updates the telemetry metrics
 */
void telemetry_update(void);

#endif /** !__CORE_TELEMETRY_H__ */
