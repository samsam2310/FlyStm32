#ifndef __GY80_H
#define __GY80_H

#include "main.h"

HAL_StatusTypeDef ADXL345_Read(int16_t* data);
HAL_StatusTypeDef L3G4200_Read(int16_t* data);
HAL_StatusTypeDef HMC5883_Read(int16_t* data);
void HMC5883_Correction(int16_t* data, int16_t* data_offset, int16_t* data_max);
HAL_StatusTypeDef BMP085_Read(int32_t* data, int32_t* temp);
HAL_StatusTypeDef Gy80_Init(void);

#endif