#ifndef __CAN_SERVER_H__
#define __CAN_SERVER_H__

#include "can_ids.h"
#include "can_parser_types.h"

typedef void (*on_steering_angle_fn_t)(float);

/**
 * @brief Initializes the CAN server module
 *
 * @param on_steering_angle Callback function for when a steering angle message is received value in range [-1.0, 1.0]
 */
void can_server_init(on_steering_angle_fn_t on_steering_angle, on_steering_angle_fn_t on_reset_setpoint);

/**
 * @brief Callback function for when a message is received on FIFO0
 */
void can_server_on_rx0_message_pending(void);

/**
 * @brief Callback function for when the watchdog timeout occurs
 */
void can_server_on_watchdog_timeout(void);

#endif /** !__CAN_SERVER_H__ */
