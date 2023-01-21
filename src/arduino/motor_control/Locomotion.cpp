#include "Locomotion.h"


/* Truncates the angle such that it is kept in range (-pi, pi]. */
double trunc_angle(double angle) {
  static double two_pi = 2. * PI;

  angle += PI;
  angle = angle - two_pi * floor(angle / two_pi);
  angle -= PI;

  return angle;
}


void DifferentialDrive::SetTargetLinearVelocity(float linear) {
  Enable();
  Locomotion::SetTargetLinearVelocity(linear);
}


void DifferentialDrive::SetTargetAngularVelocity(float angular) {
  Enable();
  Locomotion::SetTargetAngularVelocity(angular);
}


void DifferentialDrive::Stop() {
  enabled_ = false;
  Locomotion::SetTargetLinearVelocity(0.0);
  Locomotion::SetTargetAngularVelocity(0.0);
  left_motor_->Stop();
  right_motor_->Stop();
}


void DifferentialDrive::Update() {
  if (enabled_) {
    float angular = target_angular_ * track_width_ / 2.0f;
    float left_target_velocity = target_linear_ - angular;
    float right_target_velocity = target_linear_ + angular;

    left_motor_->SetTargetVelocity(left_target_velocity * ticks_per_meter_);
    right_motor_->SetTargetVelocity(right_target_velocity * ticks_per_meter_);
  }

  left_motor_->Update();
  right_motor_->Update();
}


void DifferentialDrive::Enable() {
  enabled_ = true;
}


DifferentialDriveWithImu::DifferentialDriveWithImu(
  MotorController *left_motor,
  MotorController *right_motor,
  unsigned long ticks_per_meter,
  float track_width,
  SentryIMU::SentryIMU *imu
) : DifferentialDrive(left_motor, right_motor, ticks_per_meter, track_width) {
  this->imu = imu;

  ang_pid_ = new PID(
    &ang_pid_input_,
    &ang_pid_output_,
    &ang_pid_setpoint_,
    0.5,
    6.0,
    0.0,
    // 1.5,
    // 0.0,
    // 0.0,
    DIRECT
  );

  ang_pid_->SetOutputLimits(-2 * PI, 2 * PI);
  ang_pid_->SetSampleTime(1000 / 10);
}


DifferentialDriveWithImu::~DifferentialDriveWithImu() {
  delete ang_pid_; ang_pid_ = nullptr;
}


void DifferentialDriveWithImu::SetTargetHeading(float heading) {
  Enable();
  angular_mode_ = AngularMode::HEADING;
  target_heading_ = heading;
}


void DifferentialDriveWithImu::SetTargetAngularVelocity(float angular) {
  angular_mode_ = AngularMode::VELOCITY;
  DifferentialDrive::SetTargetAngularVelocity(angular);
}


void DifferentialDriveWithImu::Stop()  {
  DifferentialDrive::Stop();
  ang_pid_->SetMode(MANUAL);
}


void DifferentialDriveWithImu::Update() {
  if (enabled_) {
    if (angular_mode_ == AngularMode::VELOCITY) {
      ang_pid_input_ = imu->sample.gyro.z.radps;
      ang_pid_setpoint_ = target_angular_;
      ang_pid_->SetMode(AUTOMATIC);
      ang_pid_->Compute();
    } else if (angular_mode_ == AngularMode::HEADING) {
      ang_pid_->SetMode(MANUAL);
      ang_pid_output_ = 4. * trunc_angle(target_heading_ - imu->sample.orient.yaw);
    } else {
      ang_pid_->SetMode(MANUAL);
      ang_pid_output_ = 0;
    }

    float angular = ang_pid_output_ * track_width_ / 2.0f;

    float left_target_velocity = target_linear_ - angular;
    float right_target_velocity = target_linear_ + angular;

    left_motor_->SetTargetVelocity(left_target_velocity * ticks_per_meter_);
    right_motor_->SetTargetVelocity(right_target_velocity * ticks_per_meter_);
  } else {
    ang_pid_output_ = 0;
  }

  left_motor_->Update();
  right_motor_->Update();
}
