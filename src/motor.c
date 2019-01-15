#include "motor.h"


TIM_HandleTypeDef PWMTimer;
static HAL_StatusTypeDef Timer_Init(void) {
  PWM_TIMx_Enable();
  PWMTimer.Instance = PWM_TIMx;
  PWMTimer.Init.Prescaler = 500;
  PWMTimer.Init.CounterMode = TIM_COUNTERMODE_UP;
  PWMTimer.Init.Period = 500;
  PWMTimer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  PWMTimer.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&PWMTimer);
  HAL_TIM_Base_Start_IT(&PWMTimer);

  return HAL_OK;
}


static HAL_StatusTypeDef GPIO_Init(void) {
	PWM_GPIOx_Enable();

	GPIO_InitTypeDef   GPIO_InitStruct;
	/* Common configuration for all channels */
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;

	GPIO_InitStruct.Pin = PWM_C1_PIN;
	HAL_GPIO_Init(PWM_GPIOx, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = PWM_C2_PIN;
	HAL_GPIO_Init(PWM_GPIOx, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = PWM_C3_PIN;
	HAL_GPIO_Init(PWM_GPIOx, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = PWM_C4_PIN;
	HAL_GPIO_Init(PWM_GPIOx, &GPIO_InitStruct);

	return HAL_OK;
}


static HAL_StatusTypeDef Update_PWM_Pulses(const int16_t* pluses) {
	TIM_OC_InitTypeDef pwm_oc_config;

	pwm_oc_config.OCMode = PWMx;
	pwm_oc_config.OCPolarity = TIM_OCPOLARITY_HIGH;
	pwm_oc_config.OCFastMode = TIM_OCFAST_DISABLE;

	pwm_oc_config.Pulse = pluses[0];
	HAL_TIM_PWM_ConfigChannel(&PWMTimer, &pwm_oc_config, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&PWMTimer, TIM_CHANNEL_1);

	pwm_oc_config.Pulse = pluses[1];
	HAL_TIM_PWM_ConfigChannel(&PWMTimer, &pwm_oc_config, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&PWMTimer, TIM_CHANNEL_2);

	pwm_oc_config.Pulse = pluses[2];
	HAL_TIM_PWM_ConfigChannel(&PWMTimer, &pwm_oc_config, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&PWMTimer, TIM_CHANNEL_3);

	pwm_oc_config.Pulse = pluses[3];
	HAL_TIM_PWM_ConfigChannel(&PWMTimer, &pwm_oc_config, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&PWMTimer, TIM_CHANNEL_4);

	return HAL_OK;
}

void Motor_Init() {
	Timer_Init();
	GPIO_Init();
}


void Motor_Start() {
	int16_t init_pluses[4] = {100, 100, 100, 100};
	Update_PWM_Pulses(init_pluses);
	HAL_Delay(5000);
}


/**
 * output from 0 ~ 1000
 **/
void Motor_Update(const int16_t* output) {
	int16_t pulses[4];
	for (int i = 0; i < 4; ++i) {
		pulses[i] = output[i] / 3 + 100;
		if (pulses[i] > 399) {
			pulses[i] = 399;
		}
		if (pulses[i] < 100) {
			pulses[i] = 100;
		}
	}
	Update_PWM_Pulses(pulses);
}