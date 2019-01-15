#include "pid.h"

#define PI 3.14159265358979323846264

#define ATTI_KP 10000
#define ATTI_KI 6000
#define ATTI_HALFT 0.05
#define L3G4200_DPI 0.00875
#define ATTI_CONST L3G4200_DPI * ATTI_HALFT * PI / 180

#define EXP_PITCH 0.0
#define EXP_ROLL 0.0
#define MOTOR_KP 1
#define MOTOR_KD 0
#include "uart.h" // debug
#include <math.h>


static float reciprocal_sqrt(float number) {
  int32_t i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y = number;
  i = *(long*)&y;  // evil floating point bit level hacking
  i = 0x5f3759df - (i >> 1);  // what the fuck?
  y = *(float*) &i;
  y = y * (threehalfs - (x2 * y * y));  // 1st iteration
  // y = y * (threehalfs - (x2 * y * y));  // 2nd iteration, this can be removed
  return y;
}

static void normalize(float* w, float* x, float* y, float* z) {
  float norm = reciprocal_sqrt((*w)*(*w) + (*x)*(*x) + (*y)*(*y) + (*z)*(*z));
  *w *= norm;
  *x *= norm;
  *y *= norm;
  *z *= norm;
}

void Attitude_Update(Attitude* attitude, int16_t accele_data[], int16_t gyro_data[], int16_t magn_data[]) {
  float q0 = attitude -> quat[0];
  float q1 = attitude -> quat[1];
  float q2 = attitude -> quat[2];
  float q3 = attitude -> quat[3];
  float ax = (float)accele_data[0];
  float ay = (float)accele_data[1];
  float az = (float)accele_data[2];
  float tmpw = 0;

  normalize(&tmpw, &ax, &ay, &az);

  float mx = (float)magn_data[0];
  float my = (float)magn_data[1];
  float mz = (float)magn_data[2];
  float cos = mx*ax + my*ay + mz*az;
  mx -= ax * cos;
  my -= ay * cos;
  mz -= az * cos;

  normalize(&tmpw, &mx, &my, &mz);

  // Gravity (0,0,1)
  float vax = (2 * (q1 * q3 - q0 * q2));
  float vay = (2 * (q0 * q1 + q2 * q3));
  float vaz = (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);

  float ex = vay * az - ay * vaz;
  float ey = vaz * ax - az * vax;
  float ez = vax * ay - ax * vay;

  // North (1,0,0)
  float vmx = q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3;
  float vmy = 2 * (q1 * q2 - q0 * q3);
  float vmz = 2 * (q1 * q3 + q0 * q2);

  ex += vmy * mz - my * vmz;
  ey += vmz * mx - mz * vmx;
  ez += vmx * my - mx * vmy;

  attitude -> eInt[0] += ex * ATTI_KI;
  attitude -> eInt[1] += ey * ATTI_KI;
  attitude -> eInt[2] += ez * ATTI_KI;

  float gx = (float)gyro_data[0] + ex * ATTI_KP + attitude -> eInt[0];
  float gy = (float)gyro_data[1] + ey * ATTI_KP + attitude -> eInt[1];
  float gz = (float)gyro_data[2] + ez * ATTI_KP + attitude -> eInt[2];

  q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * ATTI_CONST;
  q1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * ATTI_CONST;
  q2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * ATTI_CONST;
  q3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * ATTI_CONST;

  normalize(&q0, &q1, &q2, &q3);

  attitude -> quat[0] = q0;
  attitude -> quat[1] = q1;
  attitude -> quat[2] = q2;
  attitude -> quat[3] = q3;

  // debug
  static int logcnt = 0;
  // static char log[100];
  // if (ex > 0.05)log[logcnt] = '1';
  // else if (ex < -0.05)log[logcnt] = '2';
  // else log[logcnt] = '0';
  if (++logcnt == 50) {
    logcnt = 0;
    // Uart_Printf(UART_DROPABLE, "(%.2f,%.2f,%.2f)        \r",
    //       gx, gy, gz);
    // EulerAngle angle;
    // EulerAngle_From_Attitude(attitude, &angle);
    // Uart_Printf(UART_DROPABLE, "[%5.2f;%5.2f;%5.2f];        \r",
    //       angle.pitch, angle.yaw, angle.roll);
    // log[logcnt] = '\r';
    // Uart_Printf(UART_DROPABLE, log);
  }
}


void EulerAngle_From_Attitude(const Attitude* attitude, EulerAngle* angle) {
  float q0 = attitude -> quat[0];
  float q1 = attitude -> quat[1];
  float q2 = attitude -> quat[2];
  float q3 = attitude -> quat[3];

  angle -> pitch  = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.2957795131;
  angle -> yaw = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2*q2 - 2 * q3* q3 + 1)* 57.2957795131;
  angle -> roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.2957795131;
}


static EulerAngle exp_angle;
void Set_Except_Angle(const EulerAngle* angle) {
  exp_angle.pitch = EXP_PITCH;
  exp_angle.roll = EXP_ROLL;
  exp_angle.yaw = angle -> yaw;
}


/**
 * Motor output value 0~1000
 */
void Motor_Output_From_EulerAngle(const EulerAngle* angle, const int16_t* gyro_data, int16_t throttle, int16_t* output) {
  int16_t roll, yaw, pitch;
  float dif;
  dif = exp_angle.roll - angle->roll;
  roll = dif * MOTOR_KP + gyro_data[0] * MOTOR_KD;

  dif = exp_angle.pitch - angle->pitch;
  pitch = dif * MOTOR_KP + gyro_data[1] * MOTOR_KD;

  dif = exp_angle.yaw - angle->yaw;
  yaw = dif * MOTOR_KP + gyro_data[2] * MOTOR_KD;

  output[2] = (throttle + pitch - roll - yaw);
  output[0] = (throttle - pitch + roll - yaw);
  output[3] = (throttle + pitch + roll + yaw);
  output[1] = (throttle - pitch - roll + yaw);
}