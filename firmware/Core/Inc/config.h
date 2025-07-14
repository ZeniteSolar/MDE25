#ifndef __CONFIG_H__
#define __CONFIG_H__

#define UART_RX_BUFFER_SIZE 256U

#define UART_MAX_MESSAGE_SIZE 128U

#define PWM_INVERT_N_REPETITIONS 4U

#define INITIAL_PWM_FREQUENCY 24000U

#define INITIAL_PWM_DUTY 0.0f

/**
 * ============================
 * Control
 * ============================
 */
#define CONTROL_ACTUATE_MAX_VALUE 0.9f

#define CONTROL_ACTUATE_MIN_VALUE -0.9f

#define CONTROL_INTEGRAL_MAX_VALUE 0.1f

#define CONTROL_INTEGRAL_MIN_VALUE -0.1f



#endif /** !__CONFIG_H__ */
