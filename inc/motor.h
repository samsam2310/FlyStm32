#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

void Motor_Init();
void Motor_Start();
void Motor_Update(const int16_t* output);

#endif