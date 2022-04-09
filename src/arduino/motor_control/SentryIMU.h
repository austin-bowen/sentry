#pragma once

/* The Seeeduino XIAO BLE Sense is situated on the robot such that: 
 * - the IMU X axis is pointing to the left of the robot,
 * - the IMU Y axis is pointing to the back of the robot, and
 * - the IMU Z axis is pointing up.
 *
 * Some transformations are performed in SentryIMU to adjust these such that:
 * - the SentryIMU X axis is pointing to the front of the robot,
 * - the SentryIMU Y axis is pointing to the left of the robot, and
 * - the SentryIMU Z axis is pointing up (as before).
 *
 * In addition, orientation calculations are performed such that:
 * - roll (rotation around the X axis) is positive when the left side is higher than the right,
 * - pitch (rotation around the Y axis) is positive when the front is higher than the back, and
 * - yaw (rotation around the Z axis) is positive when the robot is rotated counter-clockwise.
 *
 * Note: The orientations for roll (X axis) and yaw (Z axis) follow the right-hand rule,
 *       but the pitch (Y axis) follows the left-hand rule. This is purely because it is
 *       easier to think of positive pitch as the front of the robot pointing up.
 */


// https://github.com/hideakitai/Filters
#include <Filters.h>

// https://github.com/RCmags/imuFilter
#include <imuFilter.h>

// https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/
#include <LSM6DS3.h>


namespace SentryIMU {
  constexpr float IMU_FILTER_GAIN = 0.1;

  struct IMUSample {
    // Time of sample in microseconds
    unsigned long t_us;

    // Time since previous sample in seconds (will be 0.0 for first call)
    float dt_s;

    // Acceleration in m/s^2
    float accel_x;
    float accel_y;
    float accel_z;

    // Gyro in radians / s
    float gyro_x;
    float gyro_y;
    float gyro_z;

    // Orientation in space in radians
    float roll;   // Rotation around the X axis; positive values correspond to left side higher than right
    float pitch;  // Rotation around the Y axis; positive values correspond to front higher than back
    float yaw;    // Rotation around the Z axis; positive values correspond to counter-clockwise rotation
  };

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

      void Sample(IMUSample *imu_sample);

    private:
      LSM6DS3 *xiao_imu_;

      unsigned long accel_prev_t_us_[3];
      Filter::LPF<float> *accel_filters_[3];

      unsigned long gyro_prev_t_us_[3];
      Filter::HPF<float> *gyro_filters_[3];
      float gyro_offsets_[3] = {0.0f, 0.0f, 0.0f};

      imuFilter<&IMU_FILTER_GAIN> orientation_filter_;

      void ResetFilters(const int axis_index);

      float GetAccelRaw(const int axis_index);
      float FilterAccel(float raw_accel, const int axis_index);

      float GetGyroRaw(const int axis_index);
      float FilterGyro(float raw_gyro, const int axis_index);
  };
}
