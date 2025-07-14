#ifndef __CORE_PWM_H__
#define __CORE_PWM_H__

#include "main.h"

/**
 * @brief Starts the PWM module
 */
void pwm_init(void);

/**
 * @brief Sets the duty cycle of the PWM signal
 *
 * @param duty Duty cycle in percentage (-1.0 to 1.0)
 */
void pwm_set_duty(float duty);

/**
 * @brief Gets the duty cycle of the PWM signal
 *
 * @return float Duty cycle in percentage (-1.0 to 1.0)
 */
float pwm_get_duty(void);

/**
 * @brief Sets the frequency of the PWM signal
 *
 * @param frequency Frequency in Hz
 */
void pwm_set_frequency(uint32_t frequency);

/**
 * @brief Gets the frequency of the PWM signal
 *
 * @return uint32_t Frequency in Hz
 */
uint32_t pwm_get_frequency(void);

#endif /** !__CORE_PWM_H__ */
