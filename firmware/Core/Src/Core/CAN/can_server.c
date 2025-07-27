#include <stdio.h>
#include "can.h"
#include "config.h"

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

/**
 * ============================
 * Forward Declarations
 * ============================
 */

static void can_server_filter_config(CAN_HandleTypeDef *hcan, uint16_t id, uint32_t fifo, uint32_t bank);

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
  HAL_CAN_Start(&hcan1);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  /** Start Watchdog */
  HAL_TIM_Base_Start_IT(&htim7);

  can_server_on_steering_angle = on_steering_angle;
  can_server_on_reset_setpoint = on_reset_setpoint;
}

void can_server_on_rx0_message_pending(void)
{
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];

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
