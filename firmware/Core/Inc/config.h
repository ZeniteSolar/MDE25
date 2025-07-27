#ifndef __CONFIG_H__
#define __CONFIG_H__

/**
 * ============================
 * Telemetry
 * ============================
 */

#define TELEMETRY_UPDATE_INTERVAL_MS 25U
#define TELEMETRY_PREAMBLE 0x5A54U

/**
 * ============================
 * Debug
 * ============================
 */

#define DEBUG_CONTROL_ENABLE    0U
#define DEBUG_TELEMETRY_ENABLE  1U

/**
 * ============================
 * UART
 * ============================
 */

#define UART_RX_BUFFER_SIZE 128U
#define UART_MAX_MESSAGE_SIZE 64U

/**
 * ============================
 * PWM
 * ============================
 */

#define PWM_INVERT_N_REPETITIONS 3U
#define INITIAL_PWM_FREQUENCY 24000U
#define INITIAL_PWM_DUTY 0.0f

#define PWM_MAX_IN_VOLTAGE 60.0f
#define PWM_MIN_IN_VOLTAGE 24.0f

#define PWM_MAX_OUT_VOLTAGE 12.0f

/**
 * ============================
 * Control
 * ============================
 */

#define CONTROL_INITIAL_GAIN_P 2.8f
#define CONTROL_INITIAL_GAIN_I 0.0f
#define CONTROL_INITIAL_GAIN_D 0.0f

#define CONTROL_SETPOINT_AVG_WINDOW 15U

#define CONTROL_ACTUATE_MAX_VALUE 1.0f
#define CONTROL_ACTUATE_MIN_VALUE -1.0f

#define CONTROL_ACTUATE_CUT_MIN_VALUE -0.06f
#define CONTROL_ACTUATE_CUT_MAX_VALUE 0.06f

#define CONTROL_INTEGRAL_MAX_VALUE 0.1f
#define CONTROL_INTEGRAL_MIN_VALUE -0.1f

/**
 * ============================
 * Beep
 * ============================
 */

#define STATUS_TONE_FREQUENCY_LOW_HZ 2093.0f
#define STATUS_TONE_FREQUENCY_MEDIUM_HZ 2794.0f
#define STATUS_TONE_FREQUENCY_HIGH_HZ 4186.0f
#define STATUS_TONE_DURATION_S 0.16f
#define STATUS_TONE_PWM_VOLUME 0.70f

#endif /** !__CONFIG_H__ */
