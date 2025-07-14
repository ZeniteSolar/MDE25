#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "Core/pwm.h"
#include "Core/sense.h"
#include "usart.h"
#include "Core/uart_server.h"

typedef enum {
  UART_SERVER_STATE_WAIT_START,
  UART_SERVER_STATE_WAIT_TERMINATOR,
  UART_SERVER_STATE_NEW_MESSAGE,
  UART_SERVER_STATE_ERROR,
} uart_server_state_t;

typedef enum {
  UART_SERVER_COMMAND_SET_DUTY = 'D',
  UART_SERVER_COMMAND_SET_FREQUENCY = 'F',
  UART_SERVER_COMMAND_SET_CONTROL_P_GAIN = 'P',
  UART_SERVER_COMMAND_SET_CONTROL_I_GAIN = 'I',
  UART_SERVER_COMMAND_SET_CONTROL_D_GAIN = 'D',
  UART_SERVER_COMMAND_SET_SAMPLING_FREQUENCY = 'S',
} uart_server_commands_t;

/**
 * ============================
 * Locals
 * ============================
 */

/** UART RX Buffers */
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t uart_rx_buffer_parsed  = 0U;
static volatile uint16_t uart_rx_buffer_received = 0U;

/** UART Parser Buffers */
static uint8_t uart_parser_buffer[UART_MAX_MESSAGE_SIZE];
static uint16_t uart_parser_buffer_index = 0U;
static uart_server_state_t uart_parser_state = UART_SERVER_STATE_WAIT_START;

/**
 * ============================
 * Forward Declarations
 * ============================
 */

static void uart_parse_message(const char *message);
static uart_server_state_t uart_parse_byte(uint8_t byte);

/**
 * ============================
 * Implementations
 * ============================
 */

void uart_server_init(void)
{
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_buffer, UART_RX_BUFFER_SIZE);
  uart_rx_buffer_parsed = 0U;
  uart_rx_buffer_received = 0U;
}

void uart_server_update(void)
{
  while (uart_rx_buffer_parsed != uart_rx_buffer_received)
  {
    uart_rx_buffer_parsed = 1U + (uart_rx_buffer_parsed) % UART_RX_BUFFER_SIZE;

    uart_server_state_t parse_result = uart_parse_byte(uart_rx_buffer[uart_rx_buffer_parsed - 1U]);

    if (parse_result == UART_SERVER_STATE_NEW_MESSAGE) {
      uart_parse_message((const char *)uart_parser_buffer);
    }
  }
}

static void uart_parse_message(const char *message)
{
  /** Messages should be in the format: <command>:<value (float)> */
  char *cmd_str = strtok((char*)message, ":");
  char *value_str = strtok(NULL, ":");
  if (cmd_str == NULL || value_str == NULL) {
    return;
  }
  float value = atof(value_str);

  switch (cmd_str[0])
  {
  case UART_SERVER_COMMAND_SET_DUTY:
    pwm_set_duty(value);
    break;
  case UART_SERVER_COMMAND_SET_FREQUENCY:
    pwm_set_frequency((uint32_t)value);
    break;
  case UART_SERVER_COMMAND_SET_SAMPLING_FREQUENCY:
    sense_set_sampling_frequency((uint32_t)value);
    break;
  default:
    break;
  }
}

static uart_server_state_t uart_parse_byte(uint8_t byte)
{
  switch (uart_parser_state)
  {
    case UART_SERVER_STATE_WAIT_START:
      /** New message started */
      if (byte == '<') {
        /** Reset parser buffer */
        uart_parser_buffer_index = 0U;
        memset(uart_parser_buffer, 0U, UART_MAX_MESSAGE_SIZE);
        /** Discard delimiter and advance parser state */
        uart_parser_state = UART_SERVER_STATE_WAIT_TERMINATOR;
      }
      break;
    case UART_SERVER_STATE_WAIT_TERMINATOR:
      /** Message terminated */
      if (byte == '>') {
        /** Discard terminator and add \0 to the end of the message */
        uart_parser_buffer[uart_parser_buffer_index++] = '\0';
        /** Reset parser to new message */
        uart_parser_state = UART_SERVER_STATE_WAIT_START;
        /** Informs top level that a new message is available */
        return UART_SERVER_STATE_NEW_MESSAGE;
      }
      /** Message buffer overflow */
      if (uart_parser_buffer_index >= UART_RX_BUFFER_SIZE) {
        /** Reset parser */
        uart_parser_state = UART_SERVER_STATE_WAIT_START;
        /** Informs top level that an error occurred */
        return UART_SERVER_STATE_ERROR;
      }
      /** Add message content */
      uart_parser_buffer[uart_parser_buffer_index++] = byte;
      break;
    default:
      /** Reset parser */
      uart_parser_state = UART_SERVER_STATE_WAIT_START;
  }
  return uart_parser_state;
}

/**
 * ============================
 * Interruptions
 * ============================
 */

/** RX */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  /** We only care about UART1 */
  if (huart != &huart1) {
    return;
  }

  uart_rx_buffer_received = Size;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  /** We only care about UART1 */
  if (huart != &huart1) {
    return;
  }

  if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)) {
    __HAL_UART_CLEAR_OREFLAG(huart);
  }
  if (__HAL_UART_GET_FLAG(huart, UART_FLAG_FE)) {
    __HAL_UART_CLEAR_FEFLAG(huart);
  }

  uart_server_init();
}
