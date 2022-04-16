#include "Locomotion.h"


void DifferentialDrive::Stop() {
  left_motor_->Stop();
  right_motor_->Stop();
}


void DifferentialDrive::Update() {
  float left_target_velocity = target_linear_;
  float right_target_velocity = target_linear_;

  left_motor_->SetTargetVelocity(left_target_velocity * ticks_per_meter_);
  right_motor_->SetTargetVelocity(right_target_velocity * ticks_per_meter_);

  left_motor_->Update();
  right_motor_->Update();
}
