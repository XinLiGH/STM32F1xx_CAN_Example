/**
  ******************************************************************************
  * @file    CAN.h
  * @author  XinLi
  * @version v1.0
  * @date    24-June-2018
  * @brief   Header file for CAN.c module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>Copyright &copy; 2018 XinLi</center></h2>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

#ifndef __CAN_H
#define __CAN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Header includes -----------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdbool.h>

/* Macro definitions ---------------------------------------------------------*/

/******************************* CAN1 Configure *******************************/
#define CAN1_TX_BUFFER_SIZE        (16)
#define CAN1_RX_BUFFER_SIZE        (16)

#define CAN1_TX_GPIO_CLOCK         RCC_APB2Periph_GPIOB
#define CAN1_RX_GPIO_CLOCK         RCC_APB2Periph_GPIOB

#define CAN1_TX_GPIO_PORT          GPIOB
#define CAN1_RX_GPIO_PORT          GPIOB

#define CAN1_TX_GPIO_PIN           GPIO_Pin_9
#define CAN1_RX_GPIO_PIN           GPIO_Pin_8

#define CAN1_IRQ_PREEMPT_PRIORITY  (0)
#define CAN1_IRQ_SUB_PRIORITY      (0)

#define CAN1_PORT_REMAP()          GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE)
/******************************************************************************/

#ifdef STM32F10X_CL
/******************************* CAN2 Configure *******************************/
#define CAN2_TX_BUFFER_SIZE        (16)
#define CAN2_RX_BUFFER_SIZE        (16)

#define CAN2_TX_GPIO_CLOCK         RCC_APB2Periph_GPIOB
#define CAN2_RX_GPIO_CLOCK         RCC_APB2Periph_GPIOB

#define CAN2_TX_GPIO_PORT          GPIOB
#define CAN2_RX_GPIO_PORT          GPIOB

#define CAN2_TX_GPIO_PIN           GPIO_Pin_13
#define CAN2_RX_GPIO_PIN           GPIO_Pin_12

#define CAN2_IRQ_PREEMPT_PRIORITY  (0)
#define CAN2_IRQ_SUB_PRIORITY      (0)

#define CAN2_PORT_REMAP()          GPIO_PinRemapConfig(GPIO_Remap_CAN2 , DISABLE)
/******************************************************************************/
#endif /* STM32F10X_CL */

/* Type definitions ----------------------------------------------------------*/
typedef enum
{
  CAN_WorkModeNormal   = CAN_Mode_Normal,
  CAN_WorkModeLoopBack = CAN_Mode_LoopBack
}CAN_WorkMode;

typedef enum
{
  CAN_BaudRate1000K = 6,
  CAN_BaudRate500K  = 12,
  CAN_BaudRate250K  = 24,
  CAN_BaudRate125K  = 48,
  CAN_BaudRate100K  = 60,
  CAN_BaudRate50K   = 120,
  CAN_BaudRate20K   = 300,
  CAN_BaudRate10K   = 600
}CAN_BaudRate;

/* Variable declarations -----------------------------------------------------*/
/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
void CAN_Configure(CAN_TypeDef *CANx, CAN_WorkMode WorkMode, CAN_BaudRate BaudRate, uint32_t StdId, uint32_t ExtId);
void CAN_Unconfigure(CAN_TypeDef *CANx);

void CAN_SetTransmitFinishCallback(CAN_TypeDef *CANx, void (*Callback)(void));
void CAN_SetReceiveFinishCallback(CAN_TypeDef *CANx, void (*Callback)(void));

uint32_t CAN_SetTransmitMessage(CAN_TypeDef *CANx, CanTxMsg *Message, uint32_t Number);
uint32_t CAN_GetReceiveMessage(CAN_TypeDef *CANx, CanRxMsg *Message, uint32_t Number);

uint32_t CAN_GetUsedTransmitBufferSize(CAN_TypeDef *CANx);
uint32_t CAN_GetUsedReceiveBufferSize(CAN_TypeDef *CANx);
uint32_t CAN_GetUnusedTransmitBufferSize(CAN_TypeDef *CANx);
uint32_t CAN_GetUnusedReceiveBufferSize(CAN_TypeDef *CANx);

bool CAN_IsTransmitBufferEmpty(CAN_TypeDef *CANx);
bool CAN_IsReceiveBufferEmpty(CAN_TypeDef *CANx);
bool CAN_IsTransmitBufferFull(CAN_TypeDef *CANx);
bool CAN_IsReceiveBufferFull(CAN_TypeDef *CANx);

void CAN_ClearTransmitBuffer(CAN_TypeDef *CANx);
void CAN_ClearReceiveBuffer(CAN_TypeDef *CANx);

bool CAN_IsTransmitMessage(CAN_TypeDef *CANx);

/* Function definitions ------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H */
