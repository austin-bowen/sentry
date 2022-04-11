// https://github.com/Aasim-A/AsyncTimer
#include <AsyncTimer.h>

#include "BatteryMonitor.h"
#include "Encoder.h"
#include "MotorDriver.h"
#include "SentryIMU.h"


// Pin assignments
const int BATTERY_MONITOR_PIN = 0;

const int LEFT_MOTOR_PWM_PIN   = 7;
const int LEFT_MOTOR_DIR_PIN   = 8;
const int LEFT_MOTOR_ENC_A_PIN = 3;
const int LEFT_MOTOR_ENC_B_PIN = 4;

const int RIGHT_MOTOR_PWM_PIN   = 9;
const int RIGHT_MOTOR_DIR_PIN   = 10;
const int RIGHT_MOTOR_ENC_A_PIN = 5;
const int RIGHT_MOTOR_ENC_B_PIN = 6;


AsyncTimer async;


BatteryMonitor battery_monitor(
  BATTERY_MONITOR_PIN,
  4.2f * 2,   // Full voltage
  3.2f * 2,   // Empty voltage
  20,         // Low level (percent)
  // ADC to voltage scalar
  (3.30f / 1023)              // Convert raw ADC to actual voltage read
  * ((21.7f + 10.0f) / 10.0f) // Convert actual voltage to battery voltage, which
                              // is stepped down by a resistor voltage divider
);

// IMU stuff
SentryIMU::SentryIMU imu;
SentryIMU::IMUSample imu_sample;


// Motor stuff
// - Drivers
MotorDriver left_motor_driver(LEFT_MOTOR_PWM_PIN, LEFT_MOTOR_DIR_PIN);
MotorDriver right_motor_driver(RIGHT_MOTOR_PWM_PIN, RIGHT_MOTOR_DIR_PIN);
// - Encoders
const int TICKS_PER_ROTATION = 900;
QuadratureEncoder left_motor_encoder(LEFT_MOTOR_ENC_A_PIN, LEFT_MOTOR_ENC_B_PIN, TICKS_PER_ROTATION);
QuadratureEncoder right_motor_encoder(RIGHT_MOTOR_ENC_A_PIN, RIGHT_MOTOR_ENC_B_PIN, TICKS_PER_ROTATION);


void setup() {
  setup_async_debug();

  setup_battery_monitor();
  setup_imu();
  setup_motors();

//  setup_avoid_objects();
//  setup_track_heading();
//  setup_track_motor_position();
}


void setup_async_debug() {
  Serial.begin(115200);

  async.setInterval([]() {
//    print_imu_sample();
//    print_battery_stats();
    print_encoders();
  }, 1000 / 20);
}


void print_imu_sample() {
  Serial.println("roll,pitch,yaw");
  Serial.print(imu_sample.roll);
  Serial.print(',');
  Serial.print(imu_sample.pitch);
  Serial.print(',');
  Serial.print(imu_sample.yaw);
  Serial.println();
}


void print_battery_stats() {
  Serial.println("batt_volt,batt_lvl");
  Serial.print(battery_monitor.GetVoltage());
  Serial.print(',');
  Serial.print(battery_monitor.GetLevel());
  Serial.println();
}


void print_encoders() {
  Serial.println("left_motor_pos,right_motor_pos");
  Serial.print(left_motor_encoder.ticks);
  Serial.print(',');
  Serial.print(right_motor_encoder.ticks);
  Serial.println();
}


void setup_battery_monitor() {
  check_battery_level();

  // Check battery level once per second
  async.setInterval(check_battery_level, 1000);
}


void check_battery_level() {
  battery_monitor.Update();

  if (battery_monitor.IsLow()) {
    warn_low_battery();
  }

  // Display battery level on LED
  byte battery_level = battery_monitor.GetLevel();
  analogWrite(LEDG, 255 * (100 - battery_level) / 100);
  analogWrite(LEDR, 255 * battery_level / 100);
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
  // Setup motor drivers
  left_motor_driver.Reverse(true);
  right_motor_driver.Reverse(true);

  // Setup encoders
  left_motor_encoder.Reverse(true);
  left_motor_encoder.SetupEncPinChangeHandlers(
    handle_left_motor_enc_a_change,
    handle_left_motor_enc_b_change
  );
  right_motor_encoder.SetupEncPinChangeHandlers(
    handle_right_motor_enc_a_change,
    handle_right_motor_enc_b_change
  );
}


void handle_left_motor_enc_a_change() { left_motor_encoder.HandleEncAChange(); }

void handle_left_motor_enc_b_change() { left_motor_encoder.HandleEncBChange(); }

void handle_right_motor_enc_a_change() { right_motor_encoder.HandleEncAChange(); }

void handle_right_motor_enc_b_change() { right_motor_encoder.HandleEncBChange(); }


void setup_avoid_objects() {
  // Run this at 10Hz
  async.setTimeout([]() {
    avoid_objects();
  }, 1000 / 10);
}


void avoid_objects() {
  static float speed = 0.75;

  if (detected_bump()) {
    avoid(speed);
  } else {
    drive_forward(speed);
    setup_avoid_objects();
  }
}


bool detected_bump() {
  if (imu_sample.accel_x <= -3.0f) {
    return true;
  }

  static const float max_angle = PI / 8.f;
  return imu_sample.pitch >= max_angle || abs(imu_sample.roll) >= max_angle;
}


void avoid(const float speed_) {
  static float speed = speed_;
  speed = speed_;

  stop_driving();

  // Reverse
  async.setTimeout([]() {
    drive_backward(speed);

    // Stop
    async.setTimeout([]() {
      stop_driving();

      // Turn
      async.setTimeout([]() {
        const bool left = random(2);
        left ? turn_left(speed) : turn_right(speed);

        // Stop
        async.setTimeout([]() {
          stop_driving();

          // Resume avoiding objects
          async.setTimeout([]() {
            setup_avoid_objects();
          }, 250);
        }, random(1000, 3000));
      }, 250);
    }, 2000);
  }, 250);
}


void setup_track_heading() {
  static float target_yaw = 0.0f;

  async.setInterval([]() {
    target_yaw = PI * sin(2 * PI * (millis() / 1000.f) / 30);

    float error = (imu_sample.yaw - target_yaw) / PI;

    if (abs(error) < 0.05) {
      error = 0;
    }

    float speed = 5.0f * error;
//    speed = min(max(-0.5, speed), 0.5);

    drive(speed, -speed);
  }, 1000 / 10);
}


void setup_track_motor_position() {
  static long target_pos = TICKS_PER_ROTATION * 10;

  async.setInterval([]() {
    long error = target_pos - left_motor_encoder.ticks;
//    long error = target_pos - right_motor_position;

    float speed = 0.01f * error;
    speed = min(max(-0.5, speed), 0.5);

    drive(speed, 0);
//    drive(0, speed);
  }, 1000 / 10);
}


void drive_forward(const float speed) {
  drive(speed, speed);
}


void drive_backward(const float speed) {
  drive(-speed, -speed);
}


void turn_left(const float speed) {
  drive(-speed, speed);
}


void turn_right(const float speed) {
  drive(speed, -speed);
}


void stop_driving() {
  drive(0.0, 0.0);
}


void drive(float left, float right) {
  left_motor_driver.SetPower(left);
  right_motor_driver.SetPower(right);
}


void loop() {
  async.handle();
}
