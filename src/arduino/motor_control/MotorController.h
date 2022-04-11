#pragma once

#include "Encoder.h"
#include "MotorDriver.h"


class MotorController {
  public:
    MotorController(MotorDriver *motor_driver, QuadratureEncoder *encoder);

    MotorDriver *GetMotorDriver() { return motor_driver_; }
    QuadratureEncoder *GetEncoder() { return encoder_; }

  private:
    MotorDriver *motor_driver_;
    QuadratureEncoder *encoder_;
};
