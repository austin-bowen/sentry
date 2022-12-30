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


// https://github.com/RCmags/imuFilter
#include <imuFilter.h>

// https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/
#include <LSM6DS3.h>


namespace SentryIMU {
  struct AngularVelocity {
    /** Degrees per second. */
    float degps;

    /** Radians per second. */
    float radps;
  };

  struct Gyroscope {
    AngularVelocity x;
    AngularVelocity y;
    AngularVelocity z;
  };

  struct LinearAcceleration {
    /** G's; 1 G is one Earth gravity at sea level. */
    float gs;

    /** Meters per second squared. */
    float mps2;
  };

  struct Accelerometer {
    LinearAcceleration x;
    LinearAcceleration y;
    LinearAcceleration z;
  };

  /** Orientation in space, calculated from gyro and accel. All units in radians. */
  struct Orientation {
    /** Rotation around the X axis; positive values correspond to left side higher than right. */
    float roll;

    /** Rotation around the Y axis; positive values correspond to front higher than back. */
    float pitch;

    /** Rotation around the Z axis; positive values correspond to counter-clockwise rotation. */
    float yaw;
  };

  struct Temperature {
    /** Celcius. */
    float c;

    /** Fahrenheit. */
    float f;
  };

  struct Offset {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
  };

  /** One sample of data read from the IMU. */
  struct Sample {
    /** Time of sample in microseconds. */
    unsigned long t_us = 0;

    /** Time since previous sample in microseconds (will be 0.0 for first call). */
    unsigned long dt_us;

    Accelerometer accel;
    Gyroscope gyro;
    Orientation orient;
    Temperature temp;
  };

  class SentryIMU {
    public:
      Sample sample;

      SentryIMU();
      ~SentryIMU();

      int Begin();

      void Calibrate(unsigned long time_ms);

      void ReadSample();

    private:
      LSM6DS3 *xiao_imu_;

      Offset gyro_offsets_;

      imuFilter orientation_filter_;

      float FilterGyro(float raw_gyro, const int axis_index);
  };
}
