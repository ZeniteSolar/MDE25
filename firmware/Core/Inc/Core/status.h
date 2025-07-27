#ifndef __CORE_STATUS_H__
#define __CORE_STATUS_H__

#include "main.h"
#include <stdint.h>

/**
 * @brief Plays tone pattern when the device gets connection from MIC module
 */
void status_on_connect(void);

/**
 * @brief Plays tone pattern when the device disconnects from MIC module
 */
void status_on_disconnect(void);

/**
 * @brief Callback function for when the TIM period is elapsed
 */
void status_on_tim_update(void);

/**
 * @brief Returns the status of the device
 */
uint8_t status_get_connected(void);

#endif /** !__CORE_STATUS_H__ */
