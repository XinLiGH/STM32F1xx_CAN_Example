/**
  ******************************************************************************
  * @file    CAN.c
  * @author  XinLi
  * @version v1.0
  * @date    24-June-2018
  * @brief   CAN module driver.
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

/* Header includes -----------------------------------------------------------*/
#include "CAN.h"
#include "RingBuffer.h"

/* Macro definitions ---------------------------------------------------------*/
/* Type definitions ----------------------------------------------------------*/
/* Variable declarations -----------------------------------------------------*/
static volatile bool can1InitFlag     = false;
static volatile bool can1TransmitFlag = false;

static volatile void (*can1TransmitFinishCallback)(void) = 0;
static volatile void (*can1ReceiveFinishCallback)(void)  = 0;

static RingBuffer *can1TxBuffer = 0;
static RingBuffer *can1RxBuffer = 0;

#ifdef STM32F10X_CL
static volatile bool can2InitFlag     = false;
static volatile bool can2TransmitFlag = false;

static volatile void (*can2TransmitFinishCallback)(void) = 0;
static volatile void (*can2ReceiveFinishCallback)(void)  = 0;

static RingBuffer *can2TxBuffer = 0;
static RingBuffer *can2RxBuffer = 0;
#endif /* STM32F10X_CL */

/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
/* Function definitions ------------------------------------------------------*/

/**
  * @brief  CAN configure.
  * @param  [in] CANx:     Where x can be 1 or 2 to select the CAN peripheral.
  * @param  [in] WorkMode: Work mode.
  * @param  [in] BaudRate: Communication baud rate.
  * @param  [in] StdId:    Filter standard frame ID.
  * @param  [in] ExtId:    Filter extended frame ID.
  * @return None.
  */
void CAN_Configure(CAN_TypeDef *CANx, CAN_WorkMode WorkMode, CAN_BaudRate BaudRate, uint32_t StdId, uint32_t ExtId)
{
  GPIO_InitTypeDef      GPIO_InitStructure      = {0};
  CAN_InitTypeDef       CAN_InitStructure       = {0};
  CAN_FilterInitTypeDef CAN_FilterInitStructure = {0};
  NVIC_InitTypeDef      NVIC_InitStructure      = {0};
  
  if(CANx == CAN1)
  {
    if(can1InitFlag == false)
    {
      can1InitFlag = true;
      
      can1TransmitFlag = false;
      
      can1TransmitFinishCallback = 0;
      can1ReceiveFinishCallback  = 0;
      
      can1TxBuffer = RingBuffer_Malloc(sizeof(CanTxMsg) * CAN1_TX_BUFFER_SIZE);
      can1RxBuffer = RingBuffer_Malloc(sizeof(CanRxMsg) * CAN1_RX_BUFFER_SIZE);
      
#ifdef STM32F10X_CL
      if(can2InitFlag == false)
#endif /* STM32F10X_CL */
      {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
      }
      
      RCC_APB2PeriphClockCmd(CAN1_TX_GPIO_CLOCK | CAN1_RX_GPIO_CLOCK | RCC_APB2Periph_AFIO, ENABLE);
      
      GPIO_InitStructure.GPIO_Pin   = CAN1_TX_GPIO_PIN;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(CAN1_TX_GPIO_PORT, &GPIO_InitStructure);
      
      GPIO_InitStructure.GPIO_Pin   = CAN1_RX_GPIO_PIN;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
      GPIO_Init(CAN1_RX_GPIO_PORT, &GPIO_InitStructure);
      
      CAN1_PORT_REMAP();
      
      CAN_InitStructure.CAN_Prescaler = BaudRate;
      CAN_InitStructure.CAN_Mode      = WorkMode;
      CAN_InitStructure.CAN_SJW       = CAN_SJW_1tq;
      CAN_InitStructure.CAN_BS1       = CAN_BS1_3tq;
      CAN_InitStructure.CAN_BS2       = CAN_BS2_2tq;
      CAN_InitStructure.CAN_TTCM      = DISABLE;
      CAN_InitStructure.CAN_ABOM      = ENABLE;
      CAN_InitStructure.CAN_AWUM      = DISABLE;
      CAN_InitStructure.CAN_NART      = ENABLE;
      CAN_InitStructure.CAN_RFLM      = DISABLE;
      CAN_InitStructure.CAN_TXFP      = ENABLE;
      
      CAN_DeInit(CAN1);
      CAN_Init(CAN1, &CAN_InitStructure);
      
      CAN_FilterInitStructure.CAN_FilterIdHigh         = (uint16_t)((((StdId<<18)|ExtId)<<3)>>16);
      CAN_FilterInitStructure.CAN_FilterIdLow          = (uint16_t)(((StdId<<18)|ExtId)<<3);
      CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = (~((uint16_t)((((StdId<<18)|ExtId)<<3)>>16)))&0xFFFF;
      CAN_FilterInitStructure.CAN_FilterMaskIdLow      = (~((uint16_t)(((StdId<<18)|ExtId)<<3)))&0xFFF8;
      CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
      CAN_FilterInitStructure.CAN_FilterNumber         = 0;
      CAN_FilterInitStructure.CAN_FilterMode           = CAN_FilterMode_IdMask;
      CAN_FilterInitStructure.CAN_FilterScale          = CAN_FilterScale_32bit;
      CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
      CAN_FilterInit(&CAN_FilterInitStructure);
      
      CAN_ITConfig(CAN1, CAN_IT_TME | CAN_IT_FMP0 |CAN_IT_FF0 | CAN_IT_FOV0 | CAN_IT_FMP1 | CAN_IT_FF1 | CAN_IT_FOV1 |
                         CAN_IT_WKU | CAN_IT_SLK  |CAN_IT_EWG | CAN_IT_EPV  | CAN_IT_BOF  | CAN_IT_LEC | CAN_IT_ERR, DISABLE);
      CAN_ITConfig(CAN1, CAN_IT_TME | CAN_IT_FMP0, ENABLE);
      
#ifdef STM32F10X_CL
      NVIC_InitStructure.NVIC_IRQChannel                   = CAN1_TX_IRQn;
#else
      NVIC_InitStructure.NVIC_IRQChannel                   = USB_HP_CAN1_TX_IRQn;
#endif /* STM32F10X_CL */
      
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN1_IRQ_PREEMPT_PRIORITY;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority        = CAN1_IRQ_SUB_PRIORITY;
      NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
      NVIC_Init(&NVIC_InitStructure);
      
#ifdef STM32F10X_CL
      NVIC_InitStructure.NVIC_IRQChannel                   = CAN1_RX0_IRQn;
#else
      NVIC_InitStructure.NVIC_IRQChannel                   = USB_LP_CAN1_RX0_IRQn;
#endif /* STM32F10X_CL */
      
      NVIC_Init(&NVIC_InitStructure);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == false)
    {
      can2InitFlag = true;
      
      can2TransmitFlag = false;
      
      can2TransmitFinishCallback = 0;
      can2ReceiveFinishCallback  = 0;
      
      can2TxBuffer = RingBuffer_Malloc(sizeof(CanTxMsg) * CAN2_TX_BUFFER_SIZE);
      can2RxBuffer = RingBuffer_Malloc(sizeof(CanRxMsg) * CAN2_RX_BUFFER_SIZE);
      
      if(can1InitFlag == false)
      {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
      }
      
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
      RCC_APB2PeriphClockCmd(CAN2_TX_GPIO_CLOCK | CAN2_RX_GPIO_CLOCK | RCC_APB2Periph_AFIO, ENABLE);
      
      GPIO_InitStructure.GPIO_Pin   = CAN2_TX_GPIO_PIN;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(CAN2_TX_GPIO_PORT, &GPIO_InitStructure);
      
      GPIO_InitStructure.GPIO_Pin   = CAN2_RX_GPIO_PIN;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
      GPIO_Init(CAN2_RX_GPIO_PORT, &GPIO_InitStructure);
      
      CAN2_PORT_REMAP();
      
      CAN_InitStructure.CAN_Prescaler = BaudRate;
      CAN_InitStructure.CAN_Mode      = WorkMode;
      CAN_InitStructure.CAN_SJW       = CAN_SJW_1tq;
      CAN_InitStructure.CAN_BS1       = CAN_BS1_3tq;
      CAN_InitStructure.CAN_BS2       = CAN_BS2_2tq;
      CAN_InitStructure.CAN_TTCM      = DISABLE;
      CAN_InitStructure.CAN_ABOM      = ENABLE;
      CAN_InitStructure.CAN_AWUM      = DISABLE;
      CAN_InitStructure.CAN_NART      = ENABLE;
      CAN_InitStructure.CAN_RFLM      = DISABLE;
      CAN_InitStructure.CAN_TXFP      = ENABLE;
      
      CAN_DeInit(CAN2);
      CAN_Init(CAN2, &CAN_InitStructure);
      
      CAN_FilterInitStructure.CAN_FilterIdHigh         = (uint16_t)((((StdId<<18)|ExtId)<<3)>>16);
      CAN_FilterInitStructure.CAN_FilterIdLow          = (uint16_t)(((StdId<<18)|ExtId)<<3);
      CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = (~((uint16_t)((((StdId<<18)|ExtId)<<3)>>16)))&0xFFFF;
      CAN_FilterInitStructure.CAN_FilterMaskIdLow      = (~((uint16_t)(((StdId<<18)|ExtId)<<3)))&0xFFF8;
      CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
      CAN_FilterInitStructure.CAN_FilterNumber         = 14;
      CAN_FilterInitStructure.CAN_FilterMode           = CAN_FilterMode_IdMask;
      CAN_FilterInitStructure.CAN_FilterScale          = CAN_FilterScale_32bit;
      CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
      CAN_FilterInit(&CAN_FilterInitStructure);
      
      CAN_ITConfig(CAN2, CAN_IT_TME | CAN_IT_FMP0 |CAN_IT_FF0 | CAN_IT_FOV0 | CAN_IT_FMP1 | CAN_IT_FF1 | CAN_IT_FOV1 |
                         CAN_IT_WKU | CAN_IT_SLK  |CAN_IT_EWG | CAN_IT_EPV  | CAN_IT_BOF  | CAN_IT_LEC | CAN_IT_ERR, DISABLE);
      CAN_ITConfig(CAN2, CAN_IT_TME | CAN_IT_FMP0, ENABLE);
      
      NVIC_InitStructure.NVIC_IRQChannel                   = CAN2_TX_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN2_IRQ_PREEMPT_PRIORITY;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority        = CAN2_IRQ_SUB_PRIORITY;
      NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
      NVIC_Init(&NVIC_InitStructure);
      
      NVIC_InitStructure.NVIC_IRQChannel                   = CAN2_RX0_IRQn;
      NVIC_Init(&NVIC_InitStructure);
    }
  }
#endif /* STM32F10X_CL */
}

/**
  * @brief  CAN unconfigure.
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @return None.
  */
void CAN_Unconfigure(CAN_TypeDef *CANx)
{
  NVIC_InitTypeDef NVIC_InitStructure = {0};
  
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      can1InitFlag = false;
      
#ifdef STM32F10X_CL
      NVIC_InitStructure.NVIC_IRQChannel                   = CAN1_TX_IRQn;
#else
      NVIC_InitStructure.NVIC_IRQChannel                   = USB_HP_CAN1_TX_IRQn;
#endif /* STM32F10X_CL */
      
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN1_IRQ_PREEMPT_PRIORITY;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority        = CAN1_IRQ_SUB_PRIORITY;
      NVIC_InitStructure.NVIC_IRQChannelCmd                = DISABLE;
      NVIC_Init(&NVIC_InitStructure);
      
#ifdef STM32F10X_CL
      NVIC_InitStructure.NVIC_IRQChannel                   = CAN1_RX0_IRQn;
#else
      NVIC_InitStructure.NVIC_IRQChannel                   = USB_LP_CAN1_RX0_IRQn;
#endif /* STM32F10X_CL */
      
      NVIC_Init(&NVIC_InitStructure);
      
      CAN_DeInit(CAN1);
      
#ifdef STM32F10X_CL
      if(can2InitFlag == false)
#endif /* STM32F10X_CL */
      {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, DISABLE);
      }
      
      can1TransmitFlag = false;
      
      can1TransmitFinishCallback = 0;
      can1ReceiveFinishCallback  = 0;
      
      RingBuffer_Free(can1TxBuffer);
      RingBuffer_Free(can1RxBuffer);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      can2InitFlag = false;
      
      NVIC_InitStructure.NVIC_IRQChannel                   = CAN2_TX_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN2_IRQ_PREEMPT_PRIORITY;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority        = CAN2_IRQ_SUB_PRIORITY;
      NVIC_InitStructure.NVIC_IRQChannelCmd                = DISABLE;
      NVIC_Init(&NVIC_InitStructure);
      
      NVIC_InitStructure.NVIC_IRQChannel                   = CAN2_RX0_IRQn;
      NVIC_Init(&NVIC_InitStructure);
      
      CAN_DeInit(CAN2);
      
      if(can1InitFlag == false)
      {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, DISABLE);
      }
      
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, DISABLE);
      
      can2TransmitFlag = false;
      
      can2TransmitFinishCallback = 0;
      can2ReceiveFinishCallback  = 0;
      
      RingBuffer_Free(can2TxBuffer);
      RingBuffer_Free(can2RxBuffer);
    }
  }
#endif /* STM32F10X_CL */
}

/**
  * @brief  CAN set transmit finish callback.
  * @param  [in] CANx:     Where x can be 1 or 2 to select the CAN peripheral.
  * @param  [in] Callback: Callback.
  * @return None.
  */
void CAN_SetTransmitFinishCallback(CAN_TypeDef *CANx, void (*Callback)(void))
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      can1TransmitFinishCallback = (volatile void (*)(void))Callback;
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      can2TransmitFinishCallback = (volatile void (*)(void))Callback;
    }
  }
#endif /* STM32F10X_CL */
}

/**
  * @brief  CAN set receive finish callback.
  * @param  [in] CANx:     Where x can be 1 or 2 to select the CAN peripheral.
  * @param  [in] Callback: Callback.
  * @return None.
  */
void CAN_SetReceiveFinishCallback(CAN_TypeDef *CANx, void (*Callback)(void))
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      can1ReceiveFinishCallback = (volatile void (*)(void))Callback;
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      can2ReceiveFinishCallback = (volatile void (*)(void))Callback;
    }
  }
#endif /* STM32F10X_CL */
}

/**
  * @brief  CAN set transmit message.
  * @param  [in] CANx:    Where x can be 1 or 2 to select the CAN peripheral.
  * @param  [in] Message: The address of the message to be transmit.
  * @param  [in] Number:  The number of the message to be transmit.
  * @return The number of message transmit.
  */
uint32_t CAN_SetTransmitMessage(CAN_TypeDef *CANx, CanTxMsg *Message, uint32_t Number)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      uint32_t available = RingBuffer_Avail(can1TxBuffer) / sizeof(CanTxMsg);
      
      if(available > Number)
      {
        Number = RingBuffer_In(can1TxBuffer, Message, sizeof(CanTxMsg) * Number) / sizeof(CanTxMsg);
      }
      else
      {
        Number = RingBuffer_In(can1TxBuffer, Message, sizeof(CanTxMsg) * available) / sizeof(CanTxMsg);
      }
      
      if(Number > 0)
      {
        if(can1TransmitFlag == false)
        {
          can1TransmitFlag = true;
          
          CanTxMsg canTxMsg = {0};
          RingBuffer_Out(can1TxBuffer, &canTxMsg, sizeof(canTxMsg));
          CAN_Transmit(CAN1, &canTxMsg);
        }
      }
      
      return Number;
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      uint32_t available = RingBuffer_Avail(can2TxBuffer) / sizeof(CanTxMsg);
      
      if(available > Number)
      {
        Number = RingBuffer_In(can2TxBuffer, Message, sizeof(CanTxMsg) * Number) / sizeof(CanTxMsg);
      }
      else
      {
        Number = RingBuffer_In(can2TxBuffer, Message, sizeof(CanTxMsg) * available) / sizeof(CanTxMsg);
      }
      
      if(Number > 0)
      {
        if(can2TransmitFlag == false)
        {
          can2TransmitFlag = true;
          
          CanTxMsg canTxMsg = {0};
          RingBuffer_Out(can2TxBuffer, &canTxMsg, sizeof(canTxMsg));
          CAN_Transmit(CAN2, &canTxMsg);
        }
      }
      
      return Number;
    }
  }
#endif /* STM32F10X_CL */
  
  return 0;
}

/**
  * @brief  CAN get receive message.
  * @param  [in] CANx:    Where x can be 1 or 2 to select the CAN peripheral.
  * @param  [in] Message: To store the address of the receive message.
  * @param  [in] Number:  To read the number of the received message.
  * @return The number of message obtained.
  */
uint32_t CAN_GetReceiveMessage(CAN_TypeDef *CANx, CanRxMsg *Message, uint32_t Number)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return RingBuffer_Out(can1RxBuffer, Message, sizeof(CanRxMsg) * Number) / sizeof(CanRxMsg);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return RingBuffer_Out(can2RxBuffer, Message, sizeof(CanRxMsg) * Number) / sizeof(CanRxMsg);
    }
  }
#endif /* STM32F10X_CL */
  
  return 0;
}

/**
  * @brief  Get the size of the CAN transmit buffer used.
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @return Used the size of the transmit buffer.
  */
uint32_t CAN_GetUsedTransmitBufferSize(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return RingBuffer_Len(can1TxBuffer) / sizeof(CanTxMsg);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return RingBuffer_Len(can2TxBuffer) / sizeof(CanTxMsg);
    }
  }
#endif /* STM32F10X_CL */
  
  return 0;
}

/**
  * @brief  Get the size of the CAN receive buffer used.
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @return Used the size of the receive buffer.
  */
uint32_t CAN_GetUsedReceiveBufferSize(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return RingBuffer_Len(can1RxBuffer) / sizeof(CanRxMsg);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return RingBuffer_Len(can2RxBuffer) / sizeof(CanRxMsg);
    }
  }
#endif /* STM32F10X_CL */
  
  return 0;
}

/**
  * @brief  Get the size of the CAN transmit buffer unused.
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @return Unused the size of the transmit buffer.
  */
uint32_t CAN_GetUnusedTransmitBufferSize(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return RingBuffer_Avail(can1TxBuffer) / sizeof(CanTxMsg);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return RingBuffer_Avail(can2TxBuffer) / sizeof(CanTxMsg);
    }
  }
#endif /* STM32F10X_CL */
  
  return 0;
}

/**
  * @brief  Get the size of the CAN receive buffer unused.
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @return Unused the size of the receive buffer.
  */
uint32_t CAN_GetUnusedReceiveBufferSize(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return RingBuffer_Avail(can1RxBuffer) / sizeof(CanRxMsg);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return RingBuffer_Avail(can2RxBuffer) / sizeof(CanRxMsg);
    }
  }
#endif /* STM32F10X_CL */
  
  return 0;
}

/**
  * @brief  Is the CAN transmit buffer empty?
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @retval true:      The transmit buffer is empty.
  * @retval false:     The transmit buffer is not empty.
  */
bool CAN_IsTransmitBufferEmpty(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return !(RingBuffer_Len(can1TxBuffer) / sizeof(CanTxMsg));
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return !(RingBuffer_Len(can2TxBuffer) / sizeof(CanTxMsg));
    }
  }
#endif /* STM32F10X_CL */
  
  return false;
}

/**
  * @brief  Is the CAN receive buffer empty?
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @retval true:      The receive buffer is empty.
  * @retval false:     The receive buffer is not empty.
  */
bool CAN_IsReceiveBufferEmpty(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return !(RingBuffer_Len(can1RxBuffer) / sizeof(CanRxMsg));
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return !(RingBuffer_Len(can2RxBuffer) / sizeof(CanRxMsg));
    }
  }
#endif /* STM32F10X_CL */
  
  return false;
}

/**
  * @brief  Is the CAN transmit buffer full?
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @retval true:      The transmit buffer is full.
  * @retval false:     The transmit buffer is not full.
  */
bool CAN_IsTransmitBufferFull(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return !(RingBuffer_Avail(can1TxBuffer) / sizeof(CanTxMsg));
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return !(RingBuffer_Avail(can2TxBuffer) / sizeof(CanTxMsg));
    }
  }
#endif /* STM32F10X_CL */
  
  return false;
}

/**
  * @brief  Is the CAN receive buffer full?
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @retval true:      The receive buffer is full.
  * @retval false:     The receive buffer is not full.
  */
bool CAN_IsReceiveBufferFull(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return !(RingBuffer_Avail(can1RxBuffer) / sizeof(CanRxMsg));
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return !(RingBuffer_Avail(can2RxBuffer) / sizeof(CanRxMsg));
    }
  }
#endif /* STM32F10X_CL */
  
  return false;
}

/**
  * @brief  Clear the CAN transmit buffer.
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @return None.
  */
void CAN_ClearTransmitBuffer(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      RingBuffer_Reset(can1TxBuffer);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      RingBuffer_Reset(can2TxBuffer);
    }
  }
#endif /* STM32F10X_CL */
}

/**
  * @brief  Clear the CAN receive buffer.
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @return None.
  */
void CAN_ClearReceiveBuffer(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      RingBuffer_Reset(can1RxBuffer);
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      RingBuffer_Reset(can2RxBuffer);
    }
  }
#endif /* STM32F10X_CL */
}

/**
  * @brief  Is the CAN transmit a message?
  * @param  [in] CANx: Where x can be 1 or 2 to select the CAN peripheral.
  * @retval true:      Is transmit a message.
  * @retval false:     Not transmit a message.
  */
bool CAN_IsTransmitMessage(CAN_TypeDef *CANx)
{
  if(CANx == CAN1)
  {
    if(can1InitFlag == true)
    {
      return can1TransmitFlag;
    }
  }
  
#ifdef STM32F10X_CL
  if(CANx == CAN2)
  {
    if(can2InitFlag == true)
    {
      return can2TransmitFlag;
    }
  }
#endif /* STM32F10X_CL */
  
  return false;
}

/**
  * @brief  This function handles CAN1 TX Handler.
  * @param  None.
  * @return None.
  */
#ifdef STM32F10X_CL
void CAN1_TX_IRQHandler(void)
#else
void USB_HP_CAN1_TX_IRQHandler(void)
#endif /* STM32F10X_CL */
{
  if(CAN_GetITStatus(CAN1, CAN_IT_TME) != RESET)
  {
    CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
    
    CanTxMsg canTxMsg = {0};
    uint8_t  number   = RingBuffer_Out(can1TxBuffer, &canTxMsg, sizeof(canTxMsg)) / sizeof(canTxMsg);
    
    if(number > 0)
    {
      CAN_Transmit(CAN1, &canTxMsg);
    }
    else
    {
      can1TransmitFlag = false;
      
      if(can1TransmitFinishCallback != 0)
      {
        can1TransmitFinishCallback();
      }
    }
  }
}

/**
  * @brief  This function handles CAN1 RX0 Handler.
  * @param  None.
  * @return None.
  */
#ifdef STM32F10X_CL
void CAN1_RX0_IRQHandler(void)
#else
void USB_LP_CAN1_RX0_IRQHandler(void)
#endif /* STM32F10X_CL */
{
  if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
  {
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    
    CanRxMsg canRxMsg = {0};
    CAN_Receive(CAN1, CAN_FIFO0, &canRxMsg);
    
    if(RingBuffer_Avail(can1RxBuffer) / sizeof(CanRxMsg) > 0)
    {
      RingBuffer_In(can1RxBuffer, &canRxMsg, sizeof(canRxMsg));
    }
    
    if(can1ReceiveFinishCallback != 0)
    {
      can1ReceiveFinishCallback();
    }
  }
}

#ifdef STM32F10X_CL
/**
  * @brief  This function handles CAN2 TX Handler.
  * @param  None.
  * @return None.
  */
void CAN2_TX_IRQHandler(void)
{
  if(CAN_GetITStatus(CAN2, CAN_IT_TME) != RESET)
  {
    CAN_ClearITPendingBit(CAN2, CAN_IT_TME);
    
    CanTxMsg canTxMsg = {0};
    uint8_t  number   = RingBuffer_Out(can2TxBuffer, &canTxMsg, sizeof(canTxMsg)) / sizeof(canTxMsg);
    
    if(number > 0)
    {
      CAN_Transmit(CAN2, &canTxMsg);
    }
    else
    {
      can2TransmitFlag = false;
      
      if(can2TransmitFinishCallback != 0)
      {
        can2TransmitFinishCallback();
      }
    }
  }
}

/**
  * @brief  This function handles CAN2 RX0 Handler.
  * @param  None.
  * @return None.
  */
void CAN2_RX0_IRQHandler(void)
{
  if(CAN_GetITStatus(CAN2, CAN_IT_FMP0) != RESET)
  {
    CAN_ClearITPendingBit(CAN2, CAN_IT_FMP0);
    
    CanRxMsg canRxMsg = {0};
    CAN_Receive(CAN2, CAN_FIFO0, &canRxMsg);
    
    if(RingBuffer_Avail(can2RxBuffer) / sizeof(CanRxMsg) > 0)
    {
      RingBuffer_In(can2RxBuffer, &canRxMsg, sizeof(canRxMsg));
    }
    
    if(can2ReceiveFinishCallback != 0)
    {
      can2ReceiveFinishCallback();
    }
  }
}
#endif /* STM32F10X_CL */
