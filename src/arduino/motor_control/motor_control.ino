#include "Async.h"
#include "BatteryMonitor.h"
#include "Encoder.h"
#include "Locomotion.h"
#include "MotorController.h"
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


Async::Async async;


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
// TODO
//const int IMU_UPDATE_PERIOD = 1000 / 100;
const int IMU_UPDATE_PERIOD = 10;
SentryIMU::SentryIMU imu;


// Motor stuff
// - Drivers
MotorDriver left_motor_driver(LEFT_MOTOR_PWM_PIN, LEFT_MOTOR_DIR_PIN);
MotorDriver right_motor_driver(RIGHT_MOTOR_PWM_PIN, RIGHT_MOTOR_DIR_PIN);
// - Encoders
const int TICKS_PER_ROTATION = 900;
QuadratureEncoder left_motor_encoder(LEFT_MOTOR_ENC_A_PIN, LEFT_MOTOR_ENC_B_PIN, TICKS_PER_ROTATION);
QuadratureEncoder right_motor_encoder(RIGHT_MOTOR_ENC_A_PIN, RIGHT_MOTOR_ENC_B_PIN, TICKS_PER_ROTATION);
// - Controllers
// TODO
//const int MOTOR_CONTROLLER_UPDATE_PERIOD = 1000 / 10;
const int MOTOR_CONTROLLER_UPDATE_PERIOD = 101;
float k_p = 0.0002, k_i = 0.004, k_d = 0.0;
MotorController left_motor_controller(
  &left_motor_driver,
  &left_motor_encoder,
  k_p, k_i, k_d,
  // Update period (ms)
  MOTOR_CONTROLLER_UPDATE_PERIOD
);
MotorController right_motor_controller(
  &right_motor_driver,
  &right_motor_encoder,
  k_p, k_i, k_d,
  // Update period (ms)
  MOTOR_CONTROLLER_UPDATE_PERIOD
);

// Locomotion
DifferentialDriveWithImu locomotion(
  &left_motor_controller,
  &right_motor_controller,
  // Ticks per meter
  6200,
  // Track width [m]
  0.22,
  &imu
);


void setup() {
  setup_async_debug();

  setup_battery_monitor();
  setup_imu();
  setup_motors();

//  setup_avoid_objects();
//  setup_track_heading();
//  setup_track_motor_position();
//  setup_test_locomotion();
//  setup_drive_in_square();
}


void setup_async_debug() {
  Serial.begin(115200);

//  Async::FuncId id = async.RunForever(1000 / 1, []() {
  Async::FuncId id = async.RunForever(1000, []() {
    print_async_stats();
//    print_imu_sample();
//    print_battery_stats();
//    print_encoders();
//    print_motor_controllers();
//    print_locomotion();
//    print_falling();
  });
  async.GetFunc(id)->name = "debug";
}


void print_async_stats() {
  async.PrintStats(Serial);
//  Serial.println("async_load");
//  Serial.print(async.GetLoad());
//  Serial.println();
}


void print_imu_sample() {
//  Serial.println("gyro_x,gyro_y,gyro_z");
//  Serial.print(imu.sample.gyro.x.radps);
//  Serial.print(',');
//  Serial.print(imu.sample.gyro.y.radps);
//  Serial.print(',');
//  Serial.print(imu.sample.gyro.z.radps);

  Serial.println("roll,pitch,yaw");
  Serial.print(imu.sample.orient.roll);
  Serial.print(',');
  Serial.print(imu.sample.orient.pitch);
  Serial.print(',');
  Serial.print(imu.sample.orient.yaw);

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


void print_motor_controllers() {
  Serial.println("left_vel_target,left_vel_actual,right_vel_target,right_vel_actual");
  Serial.print(left_motor_controller.GetTargetVelocity());
  Serial.print(',');
  Serial.print(left_motor_controller.GetActualVelocity());
  Serial.print(',');
  Serial.print(right_motor_controller.GetTargetVelocity());
  Serial.print(',');
  Serial.print(right_motor_controller.GetActualVelocity());
  Serial.println();
}


void print_locomotion() {
  Serial.println("lin_vel_target,lin_vel_actual,ang_vel_target,ang_vel_actual");
  Serial.print(locomotion.GetTargetLinearVelocity());
  Serial.print(',');
  Serial.print((left_motor_controller.GetActualVelocity() + right_motor_controller.GetActualVelocity()) / 2.0 / 6200.);
  Serial.print(',');
  Serial.print(locomotion.GetTargetAngularVelocity());
  Serial.print(',');
  Serial.print(imu.sample.gyro.z.radps);
  Serial.println();
}


void print_falling() {
  // TODO
//  bool falling = imu.sample.accel_net_squared < pow(5, 2);
  bool falling = false;

  Serial.println("net_accel,falling");
//  Serial.print(sqrt(imu.sample.accel_net_squared));
  Serial.print("TODO");
  Serial.print(',');
  Serial.print(falling ? 5 : 0);
  Serial.println();
}


void setup_battery_monitor() {
  check_battery_level();

  // Check battery level once per second
  // TODO
//  Async::FuncId id = async.RunForever(1000, check_battery_level);
  Async::FuncId id = async.RunForever(1000, check_battery_level);
  async.GetFunc(id)->name = "batt";
}


void check_battery_level() {
  battery_monitor.Update();

  if (battery_monitor.IsLow()) {
    warn_low_battery();
  }

  // Display battery level on LED; green is 100%, red is 0%.
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
  Async::FuncId id = async.RunForever(IMU_UPDATE_PERIOD, []() {
    imu.ReadSample();
  });
  async.GetFunc(id)->name = "imu";
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

  // Setup controllers
  Async::FuncId id = async.RunForever(MOTOR_CONTROLLER_UPDATE_PERIOD, []() {
    // TODO: CLEAN THIS UP
//    left_motor_controller.Update();
//    right_motor_controller.Update();
//    locomotion.Update();
  });
  async.GetFunc(id)->name = "motors";
}


void handle_left_motor_enc_a_change() { left_motor_encoder.HandleEncAChange(); }

void handle_left_motor_enc_b_change() { left_motor_encoder.HandleEncBChange(); }

void handle_right_motor_enc_a_change() { right_motor_encoder.HandleEncAChange(); }

void handle_right_motor_enc_b_change() { right_motor_encoder.HandleEncBChange(); }


void setup_avoid_objects() {
  // Run this at 10Hz
  Async::FuncId id = async.RunForever(1000 / 10, []() {
    avoid_objects();
  });
  async.GetFunc(id)->name = "avoid";
}


void avoid_objects() {
  static float speed = 0.2;

  if (detected_bump()) {
    avoid(speed);
  } else {
    drive_around(speed);
  }
}


bool detected_bump() {
  if (imu.sample.accel.x.mps2 <= -3.0f) {
    return true;
  }

  static const float max_angle = PI / 8.f;
  return imu.sample.orient.pitch >= max_angle || abs(imu.sample.orient.roll) >= max_angle;
}


void avoid(const float speed) {
  locomotion.SetTargetVelocities(0, 0);
  async.Delay(250);

  locomotion.SetTargetVelocities(-speed, 0);
  async.Delay(2000);

  locomotion.SetTargetVelocities(0, 0);
  async.Delay(250);

  const bool left = random(2);
  locomotion.SetTargetVelocities(0, left ? 1 : -1);
  async.Delay(random(1000, 3000));

  locomotion.SetTargetVelocities(0, 0);
  async.Delay(250);
}


void drive_around(const float speed) {
  float angular_velocity = 0.25 * sin(2. * PI * (millis() / 1000.) / 10);
  locomotion.SetTargetVelocities(speed, angular_velocity);
}


void setup_track_heading() {
  static float target_yaw = 0.0f;

  async.RunForever(1000 / 10, []() {
    target_yaw = PI * sin(2 * PI * (millis() / 1000.f) / 30);

    float error = (imu.sample.orient.yaw - target_yaw) / PI;

    if (abs(error) < 0.05) {
      error = 0;
    }

    float speed = 5.0f * error;
//    speed = min(max(-0.5, speed), 0.5);

    drive(speed, -speed);
  });
}


void setup_track_motor_position() {
  static long target_pos = TICKS_PER_ROTATION * 10;

  async.RunForever(1000 / 10, []() {
    long error = target_pos - left_motor_encoder.ticks;
//    long error = target_pos - right_motor_position;

    float speed = 0.01f * error;
    speed = min(max(-0.5, speed), 0.5);

    drive(speed, 0);
//    drive(0, speed);
  });
}


void setup_test_locomotion() {
  async.RunForever(1000 / 10, []() {
    float angular = 0.1 * sin(2. * PI * millis() / 1000. / 10.) * 2. * PI;

//    locomotion.SetTargetVelocities(0.05, 0.05 * 2 / 0.22);
    locomotion.SetTargetVelocities(0.1, angular);
  });
}


void setup_drive_in_square() {
  async.RunForever(1, []() {
    locomotion.SetTargetLinearVelocity(0.2);

    locomotion.SetTargetHeading(0.0);
    async.Delay(8000);

    locomotion.SetTargetHeading(- PI / 2);
    async.Delay(8000);

    locomotion.SetTargetHeading(PI);
    async.Delay(8000);

    locomotion.SetTargetHeading(PI / 2);
    async.Delay(8000);
  });
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
  left_motor_controller.SetTargetVelocity(left);
  right_motor_controller.SetTargetVelocity(right);
}


void loop() {
  async.Handle();
}
