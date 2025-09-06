#include <stdio.h>
#include "can.h"
#include "config.h"
#include "iwdg.h"

#include "tim.h"
#include "stm32l4xx_ll_tim.h"

#include "Core/status.h"
#include "Core/CAN/can_server.h"

/**
 * ============================
 * Locals
 * ============================
 */

static on_steering_angle_fn_t can_server_on_steering_angle = NULL;
static on_steering_angle_fn_t can_server_on_reset_setpoint = NULL;
static uint32_t can_server_error_count = 0;

/**
 * ============================
 * Forward Declarations
 * ============================
 */

static void can_server_filter_config(CAN_HandleTypeDef *hcan, uint16_t id, uint32_t fifo, uint32_t bank);
static void can_server_init_peripheral(void);
static char *can_server_error_to_string(uint32_t error);


/**
 * ============================
 * Implementations
 * ============================
 */

void can_server_init(on_steering_angle_fn_t on_steering_angle, on_steering_angle_fn_t on_reset_setpoint)
{
  /* Configures the CAN filter to receive MIC message  for steering wheel position */
  can_server_filter_config(&hcan1, CAN_MSG_MIC19_MDE_ID, CAN_FILTER_FIFO0, 0U);

  /* Starts the CAN peripheral */
  can_server_init_peripheral();

  /** Start Watchdog */
  HAL_TIM_Base_Start_IT(&htim7);

  can_server_on_steering_angle = on_steering_angle;
  can_server_on_reset_setpoint = on_reset_setpoint;
}

void can_server_init_peripheral(void)
{
  /* Initialize the CAN peripheral */
  HAL_CAN_Init(&hcan1);
  
  /* Activate the CAN peripheral notifications */
  HAL_CAN_ActivateNotification(&hcan1,
    CAN_IT_RX_FIFO0_MSG_PENDING |
    CAN_IT_ERROR_WARNING |
    CAN_IT_ERROR_PASSIVE |
    CAN_IT_BUSOFF |
    CAN_IT_LAST_ERROR_CODE |
    CAN_IT_ERROR);

  can_server_error_count = 0;
}

void can_server_deinit_peripheral(void)
{
  /* Deactivate the CAN peripheral notifications */
  HAL_CAN_DeInit(&hcan1);
}

void can_server_on_rx0_message_pending(void)
{
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];

  /* On new message, reset the error count */
  can_server_error_count = 0;

  if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK)
  {
    return;
  }

  /* Check ID and signature */
  if (
    rxHeader.StdId == CAN_MSG_MIC19_MDE_ID
    && rxHeader.DLC == CAN_MSG_MIC19_MDE_LENGTH
    && rxData[CAN_MSG_MIC19_MDE_SIGNATURE_BYTE] == CAN_SIGNATURE_MIC19
  ) {
    uint16_t position = rxData[CAN_MSG_MIC19_MDE_POSITION_L_BYTE] | (rxData[CAN_MSG_MIC19_MDE_POSITION_H_BYTE] << 8);
    float steering_angle_deg = (position / 1024.0f) * 2.0f - 1.0f;

    status_on_connect();

    /* Refresh the watchdog */
    HAL_IWDG_Refresh(&hiwdg);

    if (can_server_on_steering_angle != NULL)
    {
      /** Reset TIM7 Counter */
      LL_TIM_SetCounter(htim7.Instance, 0U);

      can_server_on_steering_angle(steering_angle_deg);
    }
  }
}

void can_server_on_watchdog_timeout(void)
{

  /* If the can had an error, reinit the peripheral */
  if (can_server_error_count > 0)
  {
    can_server_deinit_peripheral();
    HAL_Delay(10);
    can_server_init_peripheral();
  }

  status_on_disconnect();

  if (can_server_on_reset_setpoint != NULL)
  {
    can_server_on_reset_setpoint(0.0f);
  }
}

static void can_server_filter_config(CAN_HandleTypeDef *hcan, uint16_t id, uint32_t fifo, uint32_t bank)
{
  CAN_FilterTypeDef filterConfig = {0};

  filterConfig.FilterActivation = ENABLE;
  filterConfig.FilterBank = bank;
  filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  filterConfig.FilterFIFOAssignment = fifo;

  filterConfig.FilterIdHigh     = (id << 5U);
  filterConfig.FilterIdLow      = 0x0000U;
  filterConfig.FilterMaskIdHigh = (0x7FFU << 5U);
  filterConfig.FilterMaskIdLow  = 0x0000U;

  if (HAL_CAN_ConfigFilter(hcan, &filterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

char *can_server_error_to_string(uint32_t error)
{
  switch (error) {
    case HAL_CAN_ERROR_BOF:
      return "Bus off error";
    case HAL_CAN_ERROR_EPV:
      return "Error Passive";
    case HAL_CAN_ERROR_EWG:
      return "Protocol Error Warning";
    case HAL_CAN_ERROR_STF:
      return "Stuff error";
    case HAL_CAN_ERROR_FOR:
      return "Form error";
    case HAL_CAN_ERROR_ACK:
      return "Acknowledgment error";
    case HAL_CAN_ERROR_BR:
      return "Bit recessive error";
    case HAL_CAN_ERROR_BD:
      return "Bit dominant error";
    case HAL_CAN_ERROR_CRC:
      return "CRC error";
    case HAL_CAN_ERROR_RX_FOV0:
      return "Rx FIFO0 overrun error";
    case HAL_CAN_ERROR_RX_FOV1:
      return "Rx FIFO1 overrun error";
    case HAL_CAN_ERROR_TX_ALST0:
      return "TxMailbox 0 transmit failure due to arbitration lost";
    case HAL_CAN_ERROR_TX_TERR0:  
      return "TxMailbox 0 transmit failure due to transmit error";
    case HAL_CAN_ERROR_TX_ALST1:
      return "TxMailbox 1 transmit failure due to arbitration lost";
    case HAL_CAN_ERROR_TX_TERR1:
      return "TxMailbox 1 transmit failure due to transmit error";
    case HAL_CAN_ERROR_TX_ALST2:
      return "TxMailbox 2 transmit failure due to arbitration lost";
    case HAL_CAN_ERROR_TX_TERR2:
      return "TxMailbox 2 transmit failure due to transmit error";
    case HAL_CAN_ERROR_TIMEOUT:
      return "Timeout error";
    case HAL_CAN_ERROR_NOT_INITIALIZED:
      return "Peripheral not initialized";
    case HAL_CAN_ERROR_NOT_READY:
      return "Peripheral not ready";
    case HAL_CAN_ERROR_NOT_STARTED:
      return "Peripheral not started";
    case HAL_CAN_ERROR_PARAM:
      return "Parameter error";
    default:
      return "Unknown error";
  }
}

void can_server_on_error(void)
{
  uint32_t error = HAL_CAN_GetError(&hcan1);
  printf("CAN error: %s\n", can_server_error_to_string(error));
  can_server_error_count++;
}
