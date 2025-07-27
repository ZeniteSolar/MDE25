#include "usart.h"

#include "config.h"

#include "Core/sense.h"
#include "Core/control.h"
#include "Core/pwm.h"
#include "Core/status.h"
#include "Core/telemetry.h"

#include <stdio.h>

/**
 * ============================
 * Locals
 * ============================
 */

static telemetry_t telemetry = {
  .preamble = TELEMETRY_PREAMBLE,
};

/**
 * ============================
 * Implementations
 * ============================
 */

void telemetry_update(void)
{
#if DEBUG_TELEMETRY_ENABLE
  telemetry.sense_input_current = sense_get_input_current();
  telemetry.sense_input_voltage = sense_get_input_voltage();
  telemetry.sense_output_voltage = sense_get_output_voltage();
  telemetry.sense_control_point = sense_get_control_point();

  telemetry.control_setpoint = control_get_setpoint();
  telemetry.control_error = control_get_error();

  telemetry.pwm_duty = pwm_get_duty();
  telemetry.pwm_effective_duty = pwm_get_effective_duty();
  telemetry.status_connected = status_get_connected();

  printf(
    "VI:%f, II:%f, VO:%f, CP:%f, SP:%f, ER:%f, D:%f, ED:%f, ST:%hu\n",
    telemetry.sense_input_voltage,
    telemetry.sense_input_current,
    telemetry.sense_output_voltage,
    telemetry.sense_control_point,
    telemetry.control_setpoint,
    telemetry.control_error,
    telemetry.pwm_duty,
    telemetry.pwm_effective_duty,
    telemetry.status_connected
  );
#endif
}
