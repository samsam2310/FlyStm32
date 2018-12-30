#ifndef __UART_H
#define __UART_H

#include "main.h"

typedef enum {
	UART_DROPABLE,
	UART_WAIT
} UartSendMode;

HAL_StatusTypeDef Uart_Init(void);
HAL_StatusTypeDef Uart_Printf(UartSendMode mode, const char* format, ...);

#endif