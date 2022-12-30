#include <Arduino.h>
#include <SerialPackets.h>

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
const int IMU_UPDATE_PERIOD = 1000 / 100;
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
const int MOTOR_CONTROLLER_UPDATE_PERIOD = 1000 / 10;
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


// Serial packets
const uint8_t serial_buffer_size = 64 - 1;
const uint8_t read_buffer_len = serial_buffer_size - SerialPackets::kPacketHeaderLen;
uint8_t read_buffer[read_buffer_len];
SerialPackets serial_packets;


void setup() {
  // setup_async_debug();

  setup_battery_monitor();
  setup_imu();
  setup_motors();
}


void setup_async_debug() {
  Serial.begin(115200);

  Async::FuncId id = async.RunForever(100, []() {
    // print_async_stats();
    print_imu_sample();
    // print_battery_stats();
    // print_encoders();
    // print_motor_controllers();
    // print_locomotion();
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
  // Serial.print(imu.sample.gyro.x.radps);
  // Serial.print(',');
  // Serial.print(imu.sample.gyro.y.radps);
  // Serial.print(',');
  // Serial.print(imu.sample.gyro.z.radps);
  // Serial.println();

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


void setup_battery_monitor() {
  check_battery_level();

  // Check battery level once per second
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
  delay(1000);
  imu.Calibrate(2000);

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
    locomotion.Update();
  });
  async.GetFunc(id)->name = "motors";
}


void handle_left_motor_enc_a_change() { left_motor_encoder.HandleEncAChange(); }

void handle_left_motor_enc_b_change() { left_motor_encoder.HandleEncBChange(); }

void handle_right_motor_enc_a_change() { right_motor_encoder.HandleEncAChange(); }

void handle_right_motor_enc_b_change() { right_motor_encoder.HandleEncBChange(); }


void handle_commands() {
  static const byte COMMAND_HEARTBEAT = 0x00;
  static const byte COMMAND_GET_STATUS = 0x01;
  static const byte COMMAND_SET_LINEAR_VELOCITY = 0x02;
  static const byte COMMAND_SET_ANGULAR_VELOCITY = 0x03;
  static const byte COMMAND_SET_TARGET_HEADING = 0x04;
  static const byte ACK = 0x00;
  static const byte NCK = 0x01;

  int packet_len = serial_packets.ReadNonblocking(read_buffer, read_buffer_len);
  if (packet_len < 1) {
    return;
  }

  byte response[1] = {ACK};
  int response_len = 1;

  byte command = read_buffer[0];
  if (command == COMMAND_HEARTBEAT) {
    // TODO
  } else if (command == COMMAND_GET_STATUS) {
    // TODO
  } else if (command == COMMAND_SET_LINEAR_VELOCITY) {
    short linear_cm        = ((short)read_buffer[1] << 8) | (short)read_buffer[2];
    float linear = linear_cm / 100.f;
    locomotion.SetTargetLinearVelocity(linear);
  } else if (command == COMMAND_SET_ANGULAR_VELOCITY) {
    short angular_centirad = ((short)read_buffer[1] << 8) | (short)read_buffer[2];
    float angular = angular_centirad / 100.f;
    locomotion.SetTargetAngularVelocity(angular);
  } else if (command == COMMAND_SET_TARGET_HEADING) {
    short heading_centirad = ((short)read_buffer[1] << 8) | (short)read_buffer[2];
    float heading = heading_centirad / 100.f;
    locomotion.SetTargetHeading(heading);
  } else {
    response[0] = NCK;
  }

  serial_packets.Write(response, response_len);
}


void loop() {
  handle_commands();
  async.Handle();
}
