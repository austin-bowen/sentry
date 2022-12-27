import time
from dataclasses import dataclass
from typing import Optional

from serial import Serial
from serialpackets import SerialPackets
from struct import Struct


class DriveMotorController:
    def __init__(self, conn: SerialPackets):
        self.conn = conn

    @staticmethod
    def connect(port: str, baudrate: int = 115200, timeout: float = 1.) -> 'DriveMotorController':
        conn = Serial(port=port, baudrate=baudrate, timeout=timeout)
        conn = SerialPackets(conn)
        return DriveMotorController(conn)

    def set_motor_velocity(self, left, right):
        ...

    def set_body_velocity(self, linear, angular):
        """
        :param linear: Linear velocity in meters per second.
        :param angular: Angular velocity in radians per second.
        """

        self._write_then_read_ack(Request.set_body_velocity(linear, angular))

    def change_body_pose(self, linear, linear_speed, angle, angular_speed):
        ...

    def change_motor_pose(self, left, left_speed, right, right_speed):
        ...

    def get_status(self) -> 'DriveMotorControllerStatus':
        ...

    def disable(self):
        ...

    def send_heartbeat(self):
        """
        Sends a heartbeat to the controller. If the controller does not receive a heartbeat after some time,
        it will stop the motors for safety.
        """

        self._write_then_read_ack(Request.heartbeat())

    def _write_then_read(self, data: bytes) -> Optional[bytes]:
        return self.conn.write_then_read(data)

    def _write_then_read_ack(self, data: bytes) -> None:
        print(f'data={data}')
        response = self._write_then_read(data)

        if response != Response.ACK:
            raise MotorControlError(f'Expected to receive ACK; response={response}')


class Request:
    SET_BODY_VELOCITY_STRUCT = Struct('>chh')

    @staticmethod
    def heartbeat() -> bytes:
        return b'\x00'

    @staticmethod
    def set_body_velocity(linear, angular) -> bytes:
        linear_cm = round(linear * 100)
        angular_centirad = round(angular * 100)
        return Request.SET_BODY_VELOCITY_STRUCT.pack(b'\x01', linear_cm, angular_centirad)


class Response:
    ACK = b'\x00'
    NCK = b'\x01'


@dataclass
class DriveMotorControllerStatus:
    left_motor: 'MotorStatus'
    right_motor: 'MotorStatus'
    body: 'BodyStatus'
    battery_voltage: float


@dataclass
class MotorStatus:
    velocity: 'MotorVelocity'


@dataclass
class MotorVelocity:
    ticks_per_s: float
    meters_per_s: float


@dataclass
class BodyStatus:
    velocity: 'BodyVelocity'


@dataclass
class BodyVelocity:
    linear_meters_per_s: float
    angular_rad_per_s: float


class MotorControlError(Exception):
    pass


def main():
    from math import pi

    motors = DriveMotorController.connect('/dev/ttyACM0')

    while True:
        # motors.set_body_velocity(linear=0.2, angular=0.0)
        # time.sleep(16)
        # motors.set_body_velocity(linear=0.0, angular=-pi / 4)
        # time.sleep(2)
        # motors.set_body_velocity(linear=0.2, angular=0.0)
        # time.sleep(10)
        # motors.set_body_velocity(linear=0.0, angular=-pi / 4)
        # time.sleep(2)

        motors.set_body_velocity(linear=0.0, angular=0.0)
        time.sleep(3)
        motors.set_body_velocity(linear=0.2, angular=0.0)
        time.sleep(16)
        motors.set_body_velocity(linear=0.0, angular=-pi / 2)
        time.sleep(3)
        motors.set_body_velocity(linear=0.2, angular=-pi / 2)
        time.sleep(12)
        motors.set_body_velocity(linear=0.0, angular=pi)
        time.sleep(3)
        motors.set_body_velocity(linear=0.2, angular=pi)
        time.sleep(16)
        motors.set_body_velocity(linear=0.0, angular=pi / 2)
        time.sleep(3)
        motors.set_body_velocity(linear=0.2, angular=pi / 2)
        time.sleep(12)

        # break

    motors.set_body_velocity(linear=0, angular=0)


if __name__ == '__main__':
    main()
