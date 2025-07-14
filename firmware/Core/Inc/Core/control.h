#ifndef __CORE_CONTROL_H__
#define __CORE_CONTROL_H__

#include "main.h"

/**
 * @brief Function pointer type for getting the current control point
 *
 * @return float Current control point
 */
typedef float (*feedback_fn_t)(void);

/**
 * @brief Function pointer type for actuating the control loop
 *
 * @param duty Duty cycle to set in range [-1.0, 1.0]
 */
typedef void (*actuate_fn_t)(float);

/**
 * @brief Starts the control module
 */
void control_init(feedback_fn_t feedback, actuate_fn_t actuate);

/**
 * @brief Updates the control loop
 */
void control_update(void);

/**
 * @brief Sets the setpoint for the control loop
 *
 * @param point The setpoint to set in range [-1.0, 1.0]
 */
void control_set_setpoint(float point);

/**
 * @brief Sets the P gain for the control loop
 *
 * @param p_gain The P gain to set
 */
void control_set_p_gain(float p_gain);

/**
 * @brief Sets the I gain for the control loop
 *
 * @param i_gain The I gain to set
 */
void control_set_i_gain(float i_gain);

/**
 * @brief Sets the D gain for the control loop
 *
 * @param d_gain The D gain to set
 */
void control_set_d_gain(float d_gain);


#endif /** !__CORE_CONTROL_H__ */
