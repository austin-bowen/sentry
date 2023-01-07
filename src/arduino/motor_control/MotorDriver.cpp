#include <Arduino.h>

#include "MotorDriver.h"


MotorDriver::MotorDriver(int pwm_pin, int dir_pin) {
  pwm_pin_ = pwm_pin;
  dir_pin_ = dir_pin;
}


void MotorDriver::SetPower(float power) {
  // Reverse power if told to do so
  if (reverse_) {
    power *= -1;
  }

  // Limit power to [-1, 1]
  power = min(max(-1, power), 1);

  if (abs(power) < 0.2) {
    power = 0.0;
  }

  // Scale power to PWM max and round to nearest int
  int pwm = round(MAX_PWM * power);

  if (pwm >= 0) {
    analogWrite(pwm_pin_, pwm);
    digitalWrite(dir_pin_, LOW);
  } else {
    pwm *= -1;

    if (invert_power_on_reverse_) {
      pwm = MAX_PWM - pwm;
    }

    analogWrite(pwm_pin_, pwm);
    digitalWrite(dir_pin_, HIGH);
  }
}


void MotorDriver::Stop() {
  SetPower(0.0);
}


void MotorDriver::Reverse(bool reverse) {
  reverse_ = reverse;
}


void MotorDriver::InvertPowerOnReverse(bool invert) {
  invert_power_on_reverse_ = invert;
}
