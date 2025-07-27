#ifndef __CORE_UART_SERVER_H__
#define __CORE_UART_SERVER_H__

#include "main.h"

/**
 * @brief Starts the UART server module
 */
void uart_server_init(void);

/**
 * @brief Updates the UART server module
 */
void uart_server_update(void);

/**
 * @brief Sets the size of the received buffer
 *
 * @param size The size of the received buffer
 */
void uart_server_set_rx_buffer_received(uint16_t size);

#endif /** !__CORE_UART_SERVER_H__ */
