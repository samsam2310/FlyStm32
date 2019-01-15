#include "maintimer.h"


TIM_HandleTypeDef MainTimer;

HAL_StatusTypeDef MainTimer_Init(void) {
  __TIM2_CLK_ENABLE();
  MainTimer.Instance = TIM2;
  MainTimer.Init.Prescaler = 4000;
  MainTimer.Init.CounterMode = TIM_COUNTERMODE_UP;
  MainTimer.Init.Period = 200;
  MainTimer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  MainTimer.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&MainTimer);
  HAL_TIM_Base_Start_IT(&MainTimer);

  return HAL_OK;
}
