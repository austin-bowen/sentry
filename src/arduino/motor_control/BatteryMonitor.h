#pragma once

#include <Arduino.h>


class BatteryMonitor {
  public:
    BatteryMonitor(
      // The analog input pin to use to measure the battery voltage
      int pin,
      float full_voltage,
      float empty_voltage,
      // Battery level in percent which is considered low, e.g. 20. Range: 0..100
      byte low_battery_level,
      float adc_to_voltage_scalar
    );

    // Update the monitor with the battery's voltage, and calculate its level
    void Update();

    float GetVoltage();
    byte GetLevel();
    bool IsLow();

  private:
    // Constructor args
    int pin_;
    float full_voltage_;
    float empty_voltage_;
    byte low_battery_level_;
    float adc_to_voltage_scalar_;

    float voltage_;
    byte level_;
};
