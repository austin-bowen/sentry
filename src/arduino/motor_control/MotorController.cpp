#include <Arduino.h>

#include "MotorController.h"
#include "MotorDriver.h"


MotorController::MotorController(
  MotorDriver *motor_driver,
  QuadratureEncoder *encoder,
  double k_p,
  double k_i,
  double k_d,
  unsigned int sample_time_ms
) {
  motor_driver_ = motor_driver;
  encoder_ = encoder;

  pid_ = new PID(
    &pid_input_,
    &pid_output_,
    &pid_setpoint_,
    k_p,
    k_i,
    k_d,
    DIRECT
  );

  // Limit output to the range accepted by motor_driver
  pid_->SetOutputLimits(-1, 1);

  pid_->SetSampleTime(sample_time_ms);

  // Setup velocity variables
  prev_t_us_ = micros();
  prev_ticks_ = encoder->GetTicks();
}


MotorController::~MotorController() {
  delete pid_; pid_ = nullptr;
}


void MotorController::SetTargetVelocity(float ticks_per_second) {
  pid_setpoint_ = ticks_per_second;
  pid_->SetMode(AUTOMATIC);
}


void MotorController::Stop() {
  pid_setpoint_ = 0;
  pid_->SetMode(MANUAL);
}


void MotorController::Brake() {
  SetTargetVelocity(0);
}


void MotorController::UpdateActualVelocity() {
  unsigned long t_us = micros();
  long ticks = encoder_->GetTicks();

  // TODO: Low-pass filter this
  const float dt = (float)(t_us - prev_t_us_) / 1e6;
  pid_input_ = (float)(ticks - prev_ticks_) / dt;

  prev_t_us_ = t_us;
  prev_ticks_ = ticks;
}


void MotorController::Update() {
  UpdateActualVelocity();

  pid_->Compute();

  motor_driver_->SetPower(pid_output_);
}
