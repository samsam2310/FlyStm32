#ifndef __PID_H
#define __PID_H

#include "stdint.h"

typedef struct {
  float quat[4];
  float eInt[3];
} Attitude;

void Attitude_Update(Attitude* attitude, int16_t accele_data[], int16_t gyro_data[], int16_t magn_data[]);

typedef struct {
  float pitch;
  float yaw;
  float roll;
} EulerAngle;

void EulerAngle_From_Attitude(const Attitude* attitude, EulerAngle* angle);

#endif