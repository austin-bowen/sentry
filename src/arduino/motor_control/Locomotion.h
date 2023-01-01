#pragma once

#include <PID_v1.h>

#include "MotorController.h"
#include "SentryIMU.h"


class Locomotion {
  public:
    void SetTargetVelocities(float linear, float angular) {
      SetTargetLinearVelocity(linear);
      SetTargetAngularVelocity(angular);
    }

    float GetTargetLinearVelocity() { return target_linear_; }

    void SetTargetLinearVelocity(float linear) { target_linear_ = linear; }

    float GetTargetAngularVelocity() { return target_angular_; }

    void SetTargetAngularVelocity(float angular) { target_angular_ = angular; }

    void Brake() { SetTargetVelocities(0, 0); }

    virtual void Update() = 0;

  protected:
    float target_linear_ = 0.0f;
    float target_angular_ = 0.0f;
};


class DifferentialDrive : public Locomotion {
  public:
    DifferentialDrive(
      MotorController *left_motor,
      MotorController *right_motor,
      unsigned long ticks_per_meter,
      float track_width
    ) {
      left_motor_ = left_motor;
      right_motor_ = right_motor;
      ticks_per_meter_ = ticks_per_meter;
      track_width_ = track_width;
    }

    void Stop();
  
    void Update();

  protected:
    MotorController *left_motor_;
    MotorController *right_motor_;
    unsigned long ticks_per_meter_;
    float track_width_;

    bool enabled_ = true;

    void Enable();
};


enum class AngularMode { VELOCITY, HEADING };


class DifferentialDriveWithImu : public DifferentialDrive {
  public:
    DifferentialDriveWithImu(
      MotorController *left_motor,
      MotorController *right_motor,
      unsigned long ticks_per_meter,
      float track_width,
      SentryIMU::SentryIMU *imu
    );

    ~DifferentialDriveWithImu();

    void SetTargetHeading(float heading);

    void SetTargetAngularVelocity(float angular);

    /* Sets target velocities to 0 and allows the motor to rotate freely. */
    void Stop();

    void Update();

  public:
    SentryIMU::SentryIMU *imu;

    AngularMode angular_mode_ = AngularMode::VELOCITY;
    float target_heading_ = 0.0f;

    PID *ang_pid_;
    double ang_pid_input_;
    double ang_pid_output_;
    double ang_pid_setpoint_;
};
