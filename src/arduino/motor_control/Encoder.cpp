#include <Arduino.h>

#include "Encoder.h"


QuadratureEncoder::QuadratureEncoder(int enc_a_pin, int enc_b_pin, unsigned int ticks_per_rev) {
  enc_a_pin_ = enc_a_pin;
  enc_b_pin_ = enc_b_pin;
  ticks_per_rev_ = ticks_per_rev_;
}


void QuadratureEncoder::SetupEncPinChangeHandlers(void (*enc_a_handler)(), void (*enc_b_handler)()) {
  attachInterrupt(
    digitalPinToInterrupt(enc_a_pin_),
    enc_a_handler,
    CHANGE
  );

  attachInterrupt(
    digitalPinToInterrupt(enc_b_pin_),
    enc_b_handler,
    CHANGE
  );
}


void QuadratureEncoder::HandleEncAChange() {
  bool enc_a = digitalRead(enc_a_pin_);
  bool enc_b = digitalRead(enc_b_pin_);

  if ((enc_a && !enc_b) || (!enc_a && enc_b)) {
    ticks += tick_increment_;
  } else {
    ticks -= tick_increment_;
  }
}


void QuadratureEncoder::HandleEncBChange() {
  bool enc_a = digitalRead(enc_a_pin_);
  bool enc_b = digitalRead(enc_b_pin_);

  if ((enc_a && enc_b) || (!enc_a && !enc_b)) {
    ticks += tick_increment_;
  } else {
    ticks -= tick_increment_;
  }
}


void QuadratureEncoder::Reverse(bool reverse) {
  tick_increment_ = reverse ? -1 : 1;
}
