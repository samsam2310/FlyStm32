/**
  ******************************************************************************
  * @file    I2C/I2C_TwoBoards_AdvComIT/Src/main.c
  * @author  MCD Application Team
  * @brief   This sample code shows how to use STM32L4xx I2C HAL API to transmit
  *          and receive a data buffer with a communication process based on
  *          IT transfer.
  *          The communication is done using 2 Boards.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "maintimer.h"
#include "math.h"
#include "motor.h"
#include "gy80.h"
#include "pid.h"
#include "uart.h"


extern TIM_HandleTypeDef MainTimer;

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void) {
  /* LED2 is slowly blinking (1 sec. period) */
  while(1) {
    BSP_LED_Toggle(LED2);
    HAL_Delay(300);
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    /* Initialization Error */
    while(1);
  }
}

void Init(void) {
  HAL_StatusTypeDef ret = HAL_OK;
  /* STM32L4xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 80 MHz */
  SystemClock_Config();

  /* Configure LED2 */
  BSP_LED_Init(LED2);

  ret |= Gy80_Init();
  ret |= MainTimer_Init();
  ret |= Uart_Init();
  Motor_Init();

  if (ret != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {
  Init();

  Attitude attitude;
  attitude.quat[0] = 1;
  attitude.quat[1] = 0;
  attitude.quat[2] = 0;
  attitude.quat[3] = 0;
  attitude.eInt[0] = 0;
  attitude.eInt[1] = 0;
  attitude.eInt[2] = 0;

  int16_t accele[3], gyro[3], magn[3];
  // int32_t pressure, temp;
  int16_t throttle = 0;
  int16_t enable = 0;
  
  // int16_t magn_max[3], magn_offset[3];

  Uart_Printf(UART_DROPABLE, "Init;        \r\n");
  Motor_Start();
  UART_Get_Char(); // init
  Uart_Printf(UART_DROPABLE, "Start;        \r\n");

  int16_t logcnt = 0;
  while(1) {
    if (__HAL_TIM_GET_FLAG(&MainTimer, TIM_FLAG_UPDATE) != RESET) {
      __HAL_TIM_CLEAR_IT(&MainTimer, TIM_IT_UPDATE);

      if (ADXL345_Read(accele) != HAL_OK) {
        Error_Handler();
      }
      if (L3G4200_Read(gyro) != HAL_OK) {
        Error_Handler();
      }
      if (HMC5883_Read(magn) != HAL_OK) {
        Error_Handler();
      }
      // if (BMP085_Read(&pressure, &temp) != HAL_OK) {
      //   Error_Handler();
      // }
      // HMC5883_Correction(magn, magn_offset, magn_max);

      Attitude_Update(&attitude, accele, gyro, magn);
      EulerAngle angle;
      EulerAngle_From_Attitude(&attitude, &angle);
      int16_t output[4];
      Motor_Output_From_EulerAngle(&angle, gyro, throttle, output);
      if (enable) {
        Motor_Update(output);
      }

      char command = UART_Get_Char();
      switch(command) {
        case '+':
          throttle += 10;
          if (throttle > 1000) {
            throttle = 1000;
          }
          break;
        case '-':
          throttle -= 10;
          if (throttle < 0) {
            throttle = 0;
          }
          break;
        case 'p':
          Uart_Printf(UART_DROPABLE, "T:%d        \r\n", throttle);
          break;
        case 'a':
          Set_Except_Angle(&angle);
          enable = 1;
          throttle = 0;
          break;
        case 's':
          output[0] = output[1] = output[2] = output[3] = 0;
          Motor_Update(output);
          enable = 0;
          throttle = 0;
          break;
      }

      if(__HAL_TIM_GET_FLAG(&MainTimer, TIM_FLAG_UPDATE) != RESET) {
        Uart_Printf(UART_DROPABLE, "TLE;        \r\n");
      }
      if(++ logcnt >= 100) {
        logcnt = 0;
        // Uart_Printf(UART_DROPABLE, "P[%5.2f]R[%5.2f]Y[%5.2f];G:[%d;%d;%d];        \r",
        //   angle.pitch, angle.roll, angle.yaw, gyro[0], gyro[1], gyro[2]);
        Uart_Printf(UART_DROPABLE, "%d;%d;%d;%d;        \r",
          output[0],output[1],output[2],output[3]);
      }
    }
  }
}
