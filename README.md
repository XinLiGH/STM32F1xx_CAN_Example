# STM32F1xx_CAN_Example

STM32F1 的 CAN 驱动程序。适用于 STM32F1 全部芯片。

## 开发环境

* 固件库：STM32F10x_StdPeriph_Lib_V3.5.0
* 编译器：ARMCC V5.06
* IDE：Keil uVision5
* 操作系统：Windows 10 专业版

## API

* void CAN_Configure(CAN_TypeDef *CANx, CAN_WorkMode WorkMode, CAN_BaudRate BaudRate, uint32_t StdId, uint32_t ExtId)
* void CAN_Unconfigure(CAN_TypeDef *CANx)
* void CAN_SetTransmitFinishCallback(CAN_TypeDef *CANx, void (*Callback)(void))
* void CAN_SetReceiveFinishCallback(CAN_TypeDef *CANx, void (*Callback)(void))
* uint32_t CAN_SetTransmitMessage(CAN_TypeDef *CANx, CanTxMsg *Message, uint32_t Number)
* uint32_t CAN_GetReceiveMessage(CAN_TypeDef *CANx, CanRxMsg *Message, uint32_t Number)
* uint32_t CAN_GetUsedTransmitBufferSize(CAN_TypeDef *CANx)
* uint32_t CAN_GetUsedReceiveBufferSize(CAN_TypeDef *CANx)
* uint32_t CAN_GetUnusedTransmitBufferSize(CAN_TypeDef *CANx)
* uint32_t CAN_GetUnusedReceiveBufferSize(CAN_TypeDef *CANx)
* bool CAN_IsTransmitBufferEmpty(CAN_TypeDef *CANx)
* bool CAN_IsReceiveBufferEmpty(CAN_TypeDef *CANx)
* bool CAN_IsTransmitBufferFull(CAN_TypeDef *CANx)
* bool CAN_IsReceiveBufferFull(CAN_TypeDef *CANx)
* void CAN_ClearTransmitBuffer(CAN_TypeDef *CANx)
* void CAN_ClearReceiveBuffer(CAN_TypeDef *CANx)
* bool CAN_IsTransmitMessage(CAN_TypeDef *CANx)

## 注意

CAN 消息发送缓冲区和接收缓冲区的大小，可以根据应用的需求进行修改，缓冲区使用的是堆内存，需要根据缓冲区大小和应用程序中堆内存使用情况进行配置。
