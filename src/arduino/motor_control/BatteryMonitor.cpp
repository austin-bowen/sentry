#include <Arduino.h>

#include "BatteryMonitor.h"


BatteryMonitor::BatteryMonitor(
  int pin,
  float full_voltage,
  float empty_voltage,
  byte low_battery_level,
  float adc_to_voltage_scalar
) {
  pin_ = pin;
  full_voltage_ = full_voltage;
  empty_voltage_ = empty_voltage;
  low_battery_level_ = low_battery_level;
  adc_to_voltage_scalar_ = adc_to_voltage_scalar;
}


void BatteryMonitor::Update() {
  // Update voltage
  voltage_ = analogRead(pin_) * adc_to_voltage_scalar_;

  // Update level
  float level = 100.f * (voltage_ - empty_voltage_) / (full_voltage_ - empty_voltage_);
  level = min(max(0, level), 100);
  level_ = round(level);
}


float BatteryMonitor::GetVoltage() {
  return voltage_;
}


byte BatteryMonitor::GetLevel() {
  return level_;
}


bool BatteryMonitor::IsLow() {
  return GetLevel() <= low_battery_level_;
}
