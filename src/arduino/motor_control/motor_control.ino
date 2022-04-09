#include <AsyncTimer.h>

#include "SentryIMU.h"


// Pin assignments
int BATTERY_MONITOR_PIN = 0;

int LEFT_MOTOR_PWM_PIN   = 7;
int LEFT_MOTOR_DIR_PIN   = 8;
int LEFT_MOTOR_ENC_A_PIN = 5;
int LEFT_MOTOR_ENC_B_PIN = 6;

int RIGHT_MOTOR_PWM_PIN   = 9;
int RIGHT_MOTOR_DIR_PIN   = 10;
int RIGHT_MOTOR_ENC_A_PIN = 3;
int RIGHT_MOTOR_ENC_B_PIN = 4;


AsyncTimer async;

// Battery stuff
const float FULL_BATTERY_VOLTAGE = 4.2f * 2;
const float EMPTY_BATTERY_VOLTAGE = 3.2f * 2;
const byte LOW_BATTERY_LEVEL = 20;
float battery_voltage = FULL_BATTERY_VOLTAGE;

// IMU stuff
SentryIMU::SentryIMU imu;
SentryIMU::IMUSample imu_sample;


void setup() {
  setup_async_debug();

  setup_battery_monitor();
  setup_imu();
  setup_motors();
}


void setup_async_debug() {
  Serial.begin(115200);

  async.setInterval([]() {
//    print_imu_sample()

    Serial.println("batt_volt,batt_lvl");
    Serial.print(battery_voltage);
    Serial.print(',');
    Serial.print(get_battery_level());

    Serial.println();
  }, 1000 / 20);
}


void print_imu_sample() {
  Serial.print(imu_sample.roll);
  Serial.print(',');
  Serial.print(imu_sample.pitch);
  Serial.print(',');
  Serial.print(imu_sample.yaw);
}


void setup_battery_monitor() {
  check_battery_level();

  async.setInterval(check_battery_level, 1000 / 1);
}


void check_battery_level() {
  update_battery_voltage();

  byte battery_level = get_battery_level();
  if (battery_level <= LOW_BATTERY_LEVEL) {
    warn_low_battery();
  }

  // Display battery level
  analogWrite(LEDG, 255 * (100 - battery_level) / 100);
  analogWrite(LEDR, 255 * battery_level / 100);
}


void update_battery_voltage() {
  battery_voltage = read_battery_voltage();
}


float read_battery_voltage() {
  // Raw ADC reading [0..1023]
  float voltage = analogRead(BATTERY_MONITOR_PIN);

  // Actual voltage read [0.0V .. 3.3V]
  voltage *= 3.3f / 1023;

  // Actual battery voltage
  static const float voltage_divider_scalar = 10.0f / (21.7f + 10.0f);
  voltage /= voltage_divider_scalar;

  return voltage;
}


byte get_battery_level() {
  // Linearly interpolate battery voltage [full..empty] to level [0..100]
  float level = 100.f * (battery_voltage - EMPTY_BATTERY_VOLTAGE) / (FULL_BATTERY_VOLTAGE - EMPTY_BATTERY_VOLTAGE);

  // Clamp level to [0..100]
  level = min(max(0, level), 100);

  // Round level to nearest int
  return round(level);
}


void warn_low_battery() {
  // TODO
}


void setup_imu() {
  imu.Begin();
  imu.Calibrate();

  // Update the IMU sample periodically
  async.setInterval([]() {
    imu.Sample(&imu_sample);
  }, 1000 / 100);
}


void setup_motors() {
  // Setup left motor
  pinMode(LEFT_MOTOR_ENC_A_PIN, INPUT);
  pinMode(LEFT_MOTOR_ENC_B_PIN, INPUT);
//  left_motor.forward_check_func = can_move_left_forward;

  // Setup right motor
  pinMode(RIGHT_MOTOR_ENC_A_PIN, INPUT);
  pinMode(RIGHT_MOTOR_ENC_B_PIN, INPUT);
//  right_motor.forward_check_func = can_move_right_forward;
//  right_motor.debug = false;

  // Setup interrupts
//  attachInterrupt(
//    digitalPinToInterrupt(RIGHT_MOTOR_ENC_A_PIN),
//    handle_right_motor_enc_a_change,
//    CHANGE
//  );
//  attachInterrupt(
//    digitalPinToInterrupt(RIGHT_MOTOR_ENC_B_PIN),
//    handle_right_motor_enc_b_change,
//    CHANGE
//  );
//  attachInterrupt(
//    digitalPinToInterrupt(LEFT_MOTOR_ENC_A_PIN),
//    handle_left_motor_enc_a_change,
//    CHANGE
//  );
//  attachInterrupt(
//    digitalPinToInterrupt(LEFT_MOTOR_ENC_B_PIN),
//    handle_left_motor_enc_b_change,
//    CHANGE
//  );
}


void loop() {
  async.handle();
}


void avoid_objects() {
  if (detected_bump()) {
    avoid();
  } else {
    drive_forward();
  }
}


bool detected_bump() {
  SentryIMU::IMUSample sample;
  imu.Sample(&sample);

  Serial.print(sample.roll);
  Serial.print(',');
  Serial.print(sample.pitch);
  Serial.print(',');
  Serial.print(sample.yaw);
  Serial.println();

//  return sample.accel_y >= 2.0f;

  float max_angle = PI / 8.f;
  return sample.pitch >= max_angle || abs(sample.roll) >= max_angle;
}


void avoid() {
  drive(0.0, 0.0);
  delay(500);

  drive(-0.5, -0.5);
  delay(1000);

  drive(0.0, 0.0);
  delay(500);

  bool left = random(2);
  if (left) {
    drive(-0.5, 0.5);
  } else {
    drive(0.5, -0.5);
  }
  delay(random(1000, 3000));

  drive(0.0, 0.0);
  delay(500);
}


void drive_forward() {
  drive(0.5, 0.5);
}


void drive(float left, float right) {
  left = -left;
  right = -right;

  if (left >= 0.0) {
    analogWrite(LEFT_MOTOR_PWM_PIN, 255 * left);
    digitalWrite(LEFT_MOTOR_DIR_PIN, 0);
  } else {
    analogWrite(LEFT_MOTOR_PWM_PIN, 255 * (1 + left));
    digitalWrite(LEFT_MOTOR_DIR_PIN, 1);
  }

  if (right >= 0.0) {
    analogWrite(RIGHT_MOTOR_PWM_PIN, 255 * right);
    digitalWrite(RIGHT_MOTOR_DIR_PIN, 0);
  } else {
    analogWrite(RIGHT_MOTOR_PWM_PIN, 255 * (1 + right));
    digitalWrite(RIGHT_MOTOR_DIR_PIN, 1);
  }
}
