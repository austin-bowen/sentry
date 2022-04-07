#include "SentryIMU.h"

// https://github.com/RCmags/imuFilter
#include <imuFilter.h>


int RIGHT_MOTOR_PWM_PIN   = 9;
int RIGHT_MOTOR_DIR_PIN   = 10;
int RIGHT_MOTOR_ENC_A_PIN = 0;
int RIGHT_MOTOR_ENC_B_PIN = 1;

int LEFT_MOTOR_PWM_PIN   = 7;
int LEFT_MOTOR_DIR_PIN   = 8;
int LEFT_MOTOR_ENC_A_PIN = 2;
int LEFT_MOTOR_ENC_B_PIN = 3;


SentryIMU::SentryIMU imu;

constexpr float IMU_FILTER_GAIN = 0.1;
imuFilter<&IMU_FILTER_GAIN> imu_filter;


void setup() {
  Serial.begin(115200);

  setup_imu();
  setup_motors();
}


void setup_imu() {
  imu.Begin();
  imu.Calibrate();

  imu_filter.setup(imu.GetAccelX(), imu.GetAccelY(), imu.GetAccelZ());
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
  avoid_objects();
}


void avoid_objects() {
  if (detected_bump()) {
//    avoid();
    drive_forward();
  } else {
    drive_forward();
  }
}


bool detected_bump() {
  get_pitch();

//  Serial.print(imu.GetGyroX());
//  Serial.print(',');
//  Serial.print(imu.GetGyroY());
//  Serial.print(',');
//  Serial.print(imu.GetGyroZ());
//  Serial.println();

  float accel = imu.GetAccelY();

  return accel >= 2.0f;
}


float get_pitch() {
  imu_filter.update(
    imu.GetGyroX() * PI / 180.f,
    imu.GetGyroY() * PI / 180.f,
    imu.GetGyroZ() * PI / 180.f,
    imu.GetAccelX(),
    imu.GetAccelY(),
    imu.GetAccelZ()
  );

  float roll = imu_filter.roll();
  float pitch = imu_filter.pitch();
  float yaw = imu_filter.yaw();

  Serial.print(roll);
  Serial.print(',');
  Serial.print(pitch);
  Serial.print(',');
  Serial.print(yaw);
  Serial.println();

  return pitch;
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
