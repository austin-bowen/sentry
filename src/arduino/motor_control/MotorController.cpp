#include "MotorController.h"
#include "MotorDriver.h"


MotorController::MotorController(MotorDriver *motor_driver, QuadratureEncoder *encoder) {
  motor_driver_ = motor_driver;
  encoder_ = encoder;
}
