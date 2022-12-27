#include <imuFilter.h>
#include <LSM6DS3.h>

#include "SentryIMU.h"

namespace SentryIMU {
  float c_to_f(float c) {
    return (c * 9 / 5) + 32;
  }

  float deg_to_rad(float deg) {
    static const float rad_per_deg = PI / 180.f;
    return deg * rad_per_deg;
  }

  float gs_to_mps2(float gs) {
    static const float mps2_per_g = 9.80665f;
    return gs * mps2_per_g;
  }

  SentryIMU::SentryIMU() {
    // Setup IMU
    int imu_addr = 0x6A;
    xiao_imu_ = new LSM6DS3(I2C_MODE, imu_addr);
  }

  SentryIMU::~SentryIMU() {
    // Delete IMU
    delete xiao_imu_;
  }

  int SentryIMU::Begin() {
    return xiao_imu_->begin();
  }

  void SentryIMU::Calibrate() {
    // 400Hz --> 1 second
    const int samples = 400;

    // Reset gyro offsets to all 0s
    Offset new_gyro_offsets;
    gyro_offsets_ = new_gyro_offsets;

    // Throw away one batch of samples; first few samples seem to be large and skew the average
    int s;
    for (s = 0; s < samples; s++) {
      ReadSample();
    }

    // Aggregate the samples
    for (s = 0; s < samples; s++) {
      ReadSample();

      new_gyro_offsets.x += sample.gyro.x.degps;
      new_gyro_offsets.y += sample.gyro.y.degps;
      new_gyro_offsets.z += sample.gyro.z.degps;
    }

    // Average samples to get the offsets
    new_gyro_offsets.x /= samples;
    new_gyro_offsets.y /= samples;
    new_gyro_offsets.z /= samples;

    // Update gyro offsets
    gyro_offsets_ = new_gyro_offsets;

    // Setup the orientation filter
    ReadSample();
    orientation_filter_.setup(
      sample.accel.x.mps2,
      sample.accel.y.mps2,
      sample.accel.z.mps2
    );
  }

  void SentryIMU::ReadSample() {
    // Read raw sensor data from IMU
    unsigned long sample_t = micros();
    uint8_t data[2 * 7];
    xiao_imu_->readRegisterRegion(data, LSM6DS3_ACC_GYRO_OUT_TEMP_L, sizeof(data));

    // Populate times
    bool is_first_run = sample.t_us == 0L;
    sample.dt_us = is_first_run ? 0L : (sample_t - sample.t_us);
    sample.t_us = sample_t;

    // Parse raw sensor values
    uint16_t raw_temp    = (data[1]  << 8) | data[0];
    uint16_t raw_gyro_x  = (data[3]  << 8) | data[2];
    uint16_t raw_gyro_y  = (data[5]  << 8) | data[4];
    uint16_t raw_gyro_z  = (data[7]  << 8) | data[6];
    uint16_t raw_accel_x = (data[9]  << 8) | data[8];
    uint16_t raw_accel_y = (data[11] << 8) | data[10];
    uint16_t raw_accel_z = (data[13] << 8) | data[12];

    // Populate temp
    sample.temp.c = ((float)raw_temp / xiao_imu_->settings.tempSensitivity) + 25;
    sample.temp.f = c_to_f(sample.temp.c);

    // Populate gyro values
    // NOTE: Axes are re-mapped here to align with the robot
    sample.gyro.x.degps = xiao_imu_->calcGyro(-raw_gyro_y) - gyro_offsets_.x;
    sample.gyro.y.degps = xiao_imu_->calcGyro(raw_gyro_x)  - gyro_offsets_.y;
    sample.gyro.z.degps = xiao_imu_->calcGyro(raw_gyro_z)  - gyro_offsets_.z;
    sample.gyro.x.radps = deg_to_rad(sample.gyro.x.degps);
    sample.gyro.y.radps = deg_to_rad(sample.gyro.y.degps);
    sample.gyro.z.radps = deg_to_rad(sample.gyro.z.degps);
    
    // Populate accel values
    // NOTE: Axes are re-mapped here to align with the robot
    sample.accel.x.gs   = xiao_imu_->calcAccel(-raw_accel_y);
    sample.accel.y.gs   = xiao_imu_->calcAccel(raw_accel_x);
    sample.accel.z.gs   = xiao_imu_->calcAccel(raw_accel_z);
    sample.accel.x.mps2 = gs_to_mps2(sample.accel.x.gs);
    sample.accel.y.mps2 = gs_to_mps2(sample.accel.y.gs);
    sample.accel.z.mps2 = gs_to_mps2(sample.accel.z.gs);

    // Update orientation filter
    orientation_filter_.update(
      sample.gyro.x.radps,
      sample.gyro.y.radps,
      sample.gyro.z.radps,
      sample.accel.x.mps2,
      sample.accel.y.mps2,
      sample.accel.z.mps2,
      // Gain (default = 0.5)
      0.5
    );

    // Populate orientation
    // Note: These follow the right-hand rule, except for the pitch,
    // so that positive pitch values correspond to the front of the
    // robot tilting up.
    sample.orient.roll  =  orientation_filter_.roll();
    sample.orient.pitch = -orientation_filter_.pitch();
    sample.orient.yaw   =  orientation_filter_.yaw();
  }
}
