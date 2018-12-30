#include "uart.h"

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#define BUF_SIZE 512

UART_HandleTypeDef UartHandle;
__IO ITStatus UartReady = SET;

HAL_StatusTypeDef Uart_Init(void) {
  UartHandle.Instance            = USARTx;

  UartHandle.Init.BaudRate       = 9600;
  UartHandle.Init.WordLength     = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits       = UART_STOPBITS_1;
  UartHandle.Init.Parity         = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl      = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode           = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling   = UART_OVERSAMPLING_16;
  UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
    return HAL_ERROR;
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
    return HAL_ERROR;
  return HAL_OK;
}


HAL_StatusTypeDef Uart_Printf(UartSendMode mode, const char* format, ...) {
  while (UartReady == RESET) {
    if (mode == UART_DROPABLE)
      return HAL_ERROR;
  }
  char buffer[BUF_SIZE];
  va_list args;
  va_start(args, format);
  int n = vsnprintf(buffer, BUF_SIZE, format, args);
  va_end(args);
  if (n < 0)
    return HAL_ERROR;
  n = n > BUF_SIZE ? BUF_SIZE : n;
  UartReady = RESET;
  return HAL_UART_Transmit_IT(&UartHandle, &buffer, n);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) {
  /* Set transmission flag: transfer complete */
  UartReady = SET;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
  /* Set transmission flag: transfer complete */
  uint8_t b;
  HAL_UART_Receive_IT(UartHandle, &b, 1);
  b = 1;
  // UartReady = SET;
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle) {
  // Error_Handler();
  while (1);
}
