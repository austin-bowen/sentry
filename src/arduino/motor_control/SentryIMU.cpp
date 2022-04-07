#include <Filters.h>
#include <LSM6DS3.h>
#include <Wire.h>

#include "SentryIMU.h"

namespace SentryIMU {
  const int X_INDEX = 0;
  const int Y_INDEX = 1;
  const int Z_INDEX = 2;

  const float ACCEL_FILTER_GAIN = 10.0f;
  const float GYRO_FILTER_GAIN = 0.0f;

  const float GRAVITY_MPS2 = 9.80665f;

  float get_dt_(unsigned long time_array[], int index) {
    unsigned long t_us = micros();
    float dt = (t_us - time_array[index]) / 1000000.f;
    time_array[index] = t_us;
    return dt;
  }

  SentryIMU::SentryIMU() {
    // Setup IMU
    int imu_addr = 0x6A;
    xiao_imu_ = new LSM6DS3(I2C_MODE, imu_addr);

    // Setup accel filters
    accel_filters_[X_INDEX] = new Filter::LPF<float>(ACCEL_FILTER_GAIN);
    accel_filters_[Y_INDEX] = new Filter::LPF<float>(ACCEL_FILTER_GAIN);
    accel_filters_[Z_INDEX] = new Filter::LPF<float>(ACCEL_FILTER_GAIN);

    // Setup gyro filters
    gyro_filters_[X_INDEX] = new Filter::HPF<float>(GYRO_FILTER_GAIN);
    gyro_filters_[Y_INDEX] = new Filter::HPF<float>(GYRO_FILTER_GAIN);
    gyro_filters_[Z_INDEX] = new Filter::HPF<float>(GYRO_FILTER_GAIN);
  }

  SentryIMU::~SentryIMU() {
    // Delete IMU
    delete xiao_imu_;

    // Delete accel filters
    delete accel_filters_[X_INDEX];
    delete accel_filters_[Y_INDEX];
    delete accel_filters_[Z_INDEX];

    // Delete gyro filters
    delete gyro_filters_[X_INDEX];
    delete gyro_filters_[Y_INDEX];
    delete gyro_filters_[Z_INDEX];
  }

  int SentryIMU::Begin() {
    return xiao_imu_->begin();
  }

  void SentryIMU::Calibrate() {
    // 400Hz --> 1 second
    const int samples = 400;

    for (int axis_index = 0; axis_index < 3; axis_index++) {
      // Throw away one batch of samples; first few samples seem to be large and skew the average
      int sample;
      for (sample = 0; sample < samples; sample++) {
        GetGyroRaw(axis_index);
      }

      // Calculate offset
      gyro_offsets_[axis_index] = 0.0f;

      for (sample = 0; sample < samples; sample++) {
        gyro_offsets_[axis_index] += GetGyroRaw(axis_index);
      }

      gyro_offsets_[axis_index] /= samples;

      // Now that offset has been calculated, reset the filters for this axis
      ResetFilters(axis_index);
    }
  }

  void SentryIMU::ResetFilters(const int axis_index) {
    // Reset accel filters
    accel_prev_t_us_[axis_index] = micros();
    accel_filters_[axis_index]->reset(GetAccelRaw(axis_index));

    // Reset gyro filters
    gyro_prev_t_us_[axis_index] = micros();
    gyro_filters_[axis_index]->reset(GetGyroRaw(axis_index));
  }

  float SentryIMU::GetAccelX() {
    float raw_accel = GetAccelRaw(X_INDEX);
    return FilterAccel(raw_accel, X_INDEX);
  }

  float SentryIMU::GetAccelY() {
    float raw_accel = GetAccelRaw(Y_INDEX);
    return FilterAccel(raw_accel, Y_INDEX);
  }

  float SentryIMU::GetAccelZ() {
    float raw_accel = GetAccelRaw(Z_INDEX);
    return FilterAccel(raw_accel, Z_INDEX);
  }

  float SentryIMU::GetAccelRaw(const int axis_index) {
    switch (axis_index) {
      case X_INDEX:
        return xiao_imu_->readFloatAccelY();
      case Y_INDEX:
        return -xiao_imu_->readFloatAccelX();
      case Z_INDEX:
        return xiao_imu_->readFloatAccelZ();
      default:
        return 0.0f;
    }
  }

  float SentryIMU::FilterAccel(float raw_accel, const int axis_index) {
    // Calculate time since last call, dt
    float dt = get_dt_(accel_prev_t_us_, axis_index);

    // Filter accel
//    float accel = accel_filters_[axis_index]->get(raw_accel, dt);

    // Convert accel from G's to m/s^2
//    accel *= GRAVITY_MPS2;

    // TODO: NOT THIS
    float accel = raw_accel * GRAVITY_MPS2;

    return accel;
  }

  float SentryIMU::GetGyroX() {
    float raw_gyro = GetGyroRaw(X_INDEX);
    return FilterGyro(raw_gyro, X_INDEX);
  }

  float SentryIMU::GetGyroY() {
    float raw_gyro = GetGyroRaw(Y_INDEX);
    return FilterGyro(raw_gyro, Y_INDEX);
  }

  float SentryIMU::GetGyroZ() {
    float raw_gyro = GetGyroRaw(Z_INDEX);
    return FilterGyro(raw_gyro, Z_INDEX);
  }

  float SentryIMU::GetGyroRaw(const int axis_index) {
    switch (axis_index) {
      case X_INDEX:
        return xiao_imu_->readFloatGyroY();
      case Y_INDEX:
        return -xiao_imu_->readFloatGyroX();
      case Z_INDEX:
        return xiao_imu_->readFloatGyroZ();
      default:
        return 0.0f;
    }
  }

  float SentryIMU::FilterGyro(float raw_gyro, const int axis_index) {
    // For some reason, the gyros randomly output approx. +/- 18 deg/sec; ignore these spikes
    static float prev_raw_gyros[] = {0.0f, 0.0f, 0.0f};
    float spike = abs(raw_gyro - prev_raw_gyros[axis_index]);
    if (abs(spike - 18.0f) <= 1.0) {
      raw_gyro = prev_raw_gyros[axis_index];
    } else {
      prev_raw_gyros[axis_index] = raw_gyro;
    }

    // Subtract offset
    float gyro = raw_gyro - gyro_offsets_[axis_index];

    // Calculate time since last call, dt
    float dt = get_dt_(gyro_prev_t_us_, axis_index);

    // Filter gyro
//    gyro = gyro_filters_[axis_index]->get(raw_gyro, dt);

    return gyro;
  }
}
