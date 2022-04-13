#pragma once

#include <PID_v1.h>

#include "Encoder.h"
#include "MotorDriver.h"


class MotorController {
  public:
    MotorController(
      MotorDriver *motor_driver,
      Encoder *encoder,
      double k_p,
      double k_i,
      double k_d,
      unsigned int sample_time_ms
    );

    ~MotorController();

    MotorDriver *GetMotorDriver() { return motor_driver_; }
    Encoder *GetEncoder() { return encoder_; }
    PID * GetPid() { return pid_; }

    float GetTargetVelocity() { return pid_setpoint_; }

    void SetTargetVelocity(float ticks_per_second);

    /* Sets target velocity to 0 and allows the motor to rotate freely. */
    void Stop();

    /* Sets target velocity to 0 (controller will actively try to keep velocity at 0). */
    void Brake();

    float GetActualVelocity() { return pid_input_; }

    /* Call this every loop to ensure the motor controller runs. */
    void Update();

  private:
    MotorDriver *motor_driver_;
    Encoder *encoder_;

    PID *pid_;
    double pid_input_;
    double pid_output_;
    double pid_setpoint_ = 0.0;

    unsigned long prev_t_us_;
    long prev_ticks_;

    void UpdateActualVelocity();
};
