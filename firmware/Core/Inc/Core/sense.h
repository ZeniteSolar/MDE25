#ifndef __CORE_SENSE_H__
#define __CORE_SENSE_H__

#include "main.h"

/**
 * @brief Starts the sense module
 */
void sense_init(void);

/**
 * @brief Returns current measured input voltage
 *
 * @return float Input voltage in volts
 */
float sense_get_input_voltage(void);

/**
 * @brief Sets the sampling frequency for the sense module
 *
 * @param frequency Sampling frequency in Hz
 */
void sense_set_sampling_frequency(uint32_t frequency);

/**
 * @brief Returns the sampling frequency for the sense module
 *
 * @return uint32_t Sampling frequency in Hz
 */
uint32_t sense_get_sampling_frequency(void);

/**
 * @brief Returns current measured output voltage
 *
 * @return float Output voltage in volts
 */
float sense_get_output_voltage(void);

/**
 * @brief Returns current measured input current
 *
 * @return float Input current in amps
 */
float sense_get_input_current(void);

/**
 * @brief Returns current measured position
 *
 * @return float Current control position
 */
float sense_get_control_point(void);

#endif /** !__CORE_SENSE_H__ */
