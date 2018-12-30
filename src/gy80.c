#include "gy80.h"

#include "stdlib.h" // HMC5883_Correction

#define I2C_ADDRESS 0x00
#define I2C_TIMING  0x00D00E28
#define I2C_TIMEOUT 50

#define L3G4200_X_OFFSET 0
// 28
#define L3G4200_Y_OFFSET 0
// -228
#define L3G4200_Z_OFFSET 0
// 104.5
#define L3G4200_ACTIVE 0

#define HMC5883_X_OFFSET 90
#define HMC5883_Y_OFFSET -60
#define HMC5883_Z_OFFSET -116
#define HMC5883_X_SCALE 1
#define HMC5883_Y_SCALE 1
#define HMC5883_Z_SCALE 1.1


I2C_HandleTypeDef I2cHandle;

static HAL_StatusTypeDef I2C_Init(void) {
  I2cHandle.Instance             = I2Cx;
  I2cHandle.Init.Timing          = I2C_TIMING;
  I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.OwnAddress2     = 0xFF;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

  HAL_StatusTypeDef ret = HAL_OK;

  ret |= HAL_I2C_Init(&I2cHandle);
  ret |= HAL_I2CEx_ConfigAnalogFilter(&I2cHandle,I2C_ANALOGFILTER_ENABLE);
  return ret;
}

static HAL_StatusTypeDef I2C_Set_Reg(I2C_HandleTypeDef* handle, uint16_t addr, uint16_t reg, uint8_t val) {
  return HAL_I2C_Mem_Write(handle, addr, reg, 1, &val, 1, I2C_TIMEOUT);
}

static HAL_StatusTypeDef I2C_Get_Reg(I2C_HandleTypeDef* handle, uint16_t addr, uint16_t reg, uint8_t* val) {
  return HAL_I2C_Mem_Read(handle, addr, reg, 1, val, 1, I2C_TIMEOUT);
}

static HAL_StatusTypeDef ADXL345_Init(void) {
  const uint16_t ADXL345_ADDR = 0x53 << 1;
  HAL_StatusTypeDef ret = HAL_OK;
  ret |= I2C_Set_Reg(&I2cHandle, ADXL345_ADDR, 0x2D, 0x00); // normal mode, standby
  ret |= I2C_Set_Reg(&I2cHandle, ADXL345_ADDR, 0x31, 0x0B); // full resolution, +/- 16G
  ret |= I2C_Set_Reg(&I2cHandle, ADXL345_ADDR, 0x38, 0x00); // bypass mode
  ret |= I2C_Set_Reg(&I2cHandle, ADXL345_ADDR, 0x2C, 0x0A); // 100Hz
  ret |= I2C_Set_Reg(&I2cHandle, ADXL345_ADDR, 0x2D, 0x08); // start measure

  return ret;
}

HAL_StatusTypeDef ADXL345_Read(int16_t* data){
  const uint16_t ADXL345_ADDR = 0x53 << 1;
  uint8_t buffer[6];
  if (HAL_I2C_Mem_Read(&I2cHandle, ADXL345_ADDR, 0x32, 1, buffer, 6, I2C_TIMEOUT) != HAL_OK)
    return HAL_ERROR;
  data[0] = ((int16_t)(buffer[1] << 8) | buffer[0]);
  data[1] = ((int16_t)(buffer[3] << 8) | buffer[2]);
  data[2] = ((int16_t)(buffer[5] << 8) | buffer[4]);
  return HAL_OK;
}

static HAL_StatusTypeDef L3G4200_Init(void) {
  const uint16_t L3G4200_ADDR = 0x69 << 1;
  HAL_StatusTypeDef ret = HAL_OK;
  // 200Hz, Cutoff 12.5, enable 3 axis, and start 
  ret |= I2C_Set_Reg(&I2cHandle, L3G4200_ADDR, 0x20, 0x4F);
  // High-pass filter cut-off freq
  ret |= I2C_Set_Reg(&I2cHandle, L3G4200_ADDR, 0x21, 0x08);
  // block data update, 250dps  
  ret |= I2C_Set_Reg(&I2cHandle, L3G4200_ADDR, 0x23, 0x80);
  // Enable high-pass filter
  ret |= I2C_Set_Reg(&I2cHandle, L3G4200_ADDR, 0x24, 0x12);

  return ret;
}

HAL_StatusTypeDef L3G4200_Read(int16_t* data) {
  const uint16_t L3G4200_ADDR = 0x69 << 1;
  const uint16_t MAX_RETRY = 10;
  HAL_StatusTypeDef ret = HAL_OK;
  int retry = 0;
  uint8_t sts;
  do {
    ret |= I2C_Get_Reg(&I2cHandle, L3G4200_ADDR, 0x27, &sts);
    if (ret || ++retry > MAX_RETRY) {
      return HAL_ERROR;
    }
  } while ((sts & 0X80) == 0);
  uint8_t buffer[6];
  // 0x28 | Auto increasement 0x80
  ret |= HAL_I2C_Mem_Read(&I2cHandle, L3G4200_ADDR, 0x28 | 0x80, 1, buffer, 6, I2C_TIMEOUT);
  data[0] = ((int16_t)(buffer[1] << 8) | buffer[0]) - L3G4200_X_OFFSET;
  data[1] = ((int16_t)(buffer[3] << 8) | buffer[2]) - L3G4200_Y_OFFSET;
  data[2] = ((int16_t)(buffer[5] << 8) | buffer[4]) - L3G4200_Z_OFFSET;

  data[0] = abs(data[0]) <= L3G4200_ACTIVE ? 0 : data[0];
  data[1] = abs(data[1]) <= L3G4200_ACTIVE ? 0 : data[1];
  data[2] = abs(data[2]) <= L3G4200_ACTIVE ? 0 : data[2];

  return ret;
}

static HAL_StatusTypeDef HMC5883_Init(void) {
  const uint16_t HMC5883_ADDR = 0x1E << 1;
  HAL_StatusTypeDef ret = HAL_OK;
  ret |= I2C_Set_Reg(&I2cHandle, HMC5883_ADDR, 0, 0x60); // 8 sample average
  ret |= I2C_Set_Reg(&I2cHandle, HMC5883_ADDR, 1, 0x20); // +/-1.3 Ga
  ret |= I2C_Set_Reg(&I2cHandle, HMC5883_ADDR, 2, 0x00); // start

  return ret;
}

HAL_StatusTypeDef HMC5883_Read(int16_t* data) {
  const uint16_t HMC5883_ADDR = 0x1E << 1;
  HAL_StatusTypeDef ret = HAL_OK;
  uint8_t buffer[6];
  ret |= HAL_I2C_Mem_Read(&I2cHandle, HMC5883_ADDR, 0x3, 1, buffer, 6, I2C_TIMEOUT);
  // ret |= HAL_I2C_Master_Receive(&I2cHandle, HMC5883_ADDR, buffer, 6, I2C_TIMEOUT);
  data[0] = ((int16_t)(buffer[0] << 8) | buffer[1]) - HMC5883_X_OFFSET;
  data[1] = ((int16_t)(buffer[4] << 8) | buffer[5]) - HMC5883_Y_OFFSET;
  data[2] = ((int16_t)(buffer[2] << 8) | buffer[3]) - HMC5883_Z_OFFSET;
  data[0] = (int16_t)(data[0] * HMC5883_X_SCALE);
  data[1] = (int16_t)(data[1] * HMC5883_Y_SCALE);
  data[2] = (int16_t)(data[2] * HMC5883_Z_SCALE);

  return ret;
}


static int cmp(void* a, void* b) {
  return *(int16_t*)a - *(int16_t*)b;
}
void HMC5883_Correction(int16_t* data, int16_t* data_offset, int16_t* data_max) {
  static int16_t arr[3][20] = {0};
  for (int i = 0; i < 3; ++i) {
    arr[i][10] = data[i];
    qsort(arr[i], 20, sizeof(int16_t), cmp);
    data_offset[i] = (arr[i][3] + arr[i][16]) / 2;
    data_max[i] = arr[i][16] - data_offset[i];
  }
}

HAL_StatusTypeDef Gy80_Init(void) {
  if (I2C_Init() != HAL_OK)
    return HAL_ERROR;
  if (ADXL345_Init() != HAL_OK)
    return HAL_ERROR;
  if (L3G4200_Init() != HAL_OK)
    return HAL_ERROR;
  if (HMC5883_Init() != HAL_OK)
    return HAL_ERROR;
  return HAL_OK;
}

/**
  * @brief  I2C error callbacks.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle) {
  /** Error_Handler() function is called when error occurs.
    * 1- When Slave don't acknowledge it's address, Master restarts communication.
    * 2- When Master don't acknowledge the last data transferred, Slave don't care in this example.
    */
  if (HAL_I2C_GetError(I2cHandle) != HAL_I2C_ERROR_AF) {
    // Error_Handler();
    while (1);
  }
}
