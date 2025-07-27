#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "usart.h"

#include "Core/control.h"
#include "Core/pwm.h"
#include "Core/sense.h"
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
  UART_SERVER_COMMAND_SET_CONTROL_P_GAIN = 'p',
  UART_SERVER_COMMAND_SET_CONTROL_I_GAIN = 'i',
  UART_SERVER_COMMAND_SET_CONTROL_D_GAIN = 'd',
  UART_SERVER_COMMAND_SET_SAMPLING_FREQUENCY = 's',
  UART_SERVER_COMMAND_SET_PWM_LOCK = 'L',
  UART_SERVER_COMMAND_MCU_RESET = 'R',
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

    if (parse_result == UART_SERVER_STATE_NEW_MESSAGE)
    {
      uart_parse_message((const char *)uart_parser_buffer);
    }
  }
}

void uart_server_set_rx_buffer_received(uint16_t size)
{
  uart_rx_buffer_received = size;
}

static void uart_parse_message(const char *message)
{
  /** Messages should be in the format: <command>:<value (float)> */
  char *cmd_str = strtok((char*)message, ":");
  char *value_str = strtok(NULL, ":");
  if (cmd_str == NULL || value_str == NULL)
  {
    return;
  }
  float value = atof(value_str);

  switch (cmd_str[0])
  {
  case UART_SERVER_COMMAND_SET_DUTY:
    pwm_set_duty_forced(value);
    break;
  case UART_SERVER_COMMAND_SET_FREQUENCY:
    pwm_set_frequency((uint32_t)value);
    break;
  case UART_SERVER_COMMAND_SET_CONTROL_P_GAIN:
    control_set_p_gain(value);
    break;
  case UART_SERVER_COMMAND_SET_CONTROL_I_GAIN:
    control_set_i_gain(value);
    break;
  case UART_SERVER_COMMAND_SET_CONTROL_D_GAIN:
    control_set_d_gain(value);
    break;
  case UART_SERVER_COMMAND_SET_SAMPLING_FREQUENCY:
    sense_set_sampling_frequency((uint32_t)value);
    break;
  case UART_SERVER_COMMAND_SET_PWM_LOCK:
    if (value > 0.0f)
    {
      pwm_lock();
    }
    else
    {
      pwm_unlock();
    }
    break;
  case UART_SERVER_COMMAND_MCU_RESET:
    HAL_NVIC_SystemReset();
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
      if (byte == '<')
      {
        /** Reset parser buffer */
        uart_parser_buffer_index = 0U;
        memset(uart_parser_buffer, 0U, UART_MAX_MESSAGE_SIZE);
        /** Discard delimiter and advance parser state */
        uart_parser_state = UART_SERVER_STATE_WAIT_TERMINATOR;
      }
      break;
    case UART_SERVER_STATE_WAIT_TERMINATOR:
      /** Message terminated */
      if (byte == '>')
      {
        /** Discard terminator and add \0 to the end of the message */
        uart_parser_buffer[uart_parser_buffer_index++] = '\0';
        /** Reset parser to new message */
        uart_parser_state = UART_SERVER_STATE_WAIT_START;
        /** Informs top level that a new message is available */
        return UART_SERVER_STATE_NEW_MESSAGE;
      }
      /** Message buffer overflow */
      if (uart_parser_buffer_index >= UART_RX_BUFFER_SIZE)
      {
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
