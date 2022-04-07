#pragma once

#include <Filters.h>
#include <LSM6DS3.h>

namespace SentryIMU {
  class SentryIMU {
    public:
      SentryIMU();
      ~SentryIMU();

      int Begin();

      void Calibrate();

      float GetAccelX();
      float GetAccelY();
      float GetAccelZ();

      float GetGyroX();
      float GetGyroY();
      float GetGyroZ();

    private:
      LSM6DS3 *xiao_imu_;

      unsigned long accel_prev_t_us_[3];
      Filter::LPF<float> *accel_filters_[3];

      unsigned long gyro_prev_t_us_[3];
      Filter::HPF<float> *gyro_filters_[3];
      float gyro_offsets_[3] = {0.0f, 0.0f, 0.0f};

      void ResetFilters(const int axis_index);

      float GetAccelRaw(const int axis_index);
      float FilterAccel(float raw_accel, const int axis_index);

      float GetGyroRaw(const int axis_index);
      float FilterGyro(float raw_gyro, const int axis_index);
  };
}
