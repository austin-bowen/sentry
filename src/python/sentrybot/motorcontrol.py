import time
from dataclasses import dataclass
from math import radians
from struct import Struct
from typing import Optional

from serial import Serial

from sentrybot.geometry import trunc_angle
from serialpackets import SerialPackets


class DriveMotorController:
    def __init__(self, conn: SerialPackets):
        self.conn = conn

        self._last_target_heading = None

    @staticmethod
    def connect(port: str, baudrate: int = 115200, timeout: float = 1.) -> 'DriveMotorController':
        conn = Serial(port=port, baudrate=baudrate, timeout=timeout)
        conn = SerialPackets(conn)
        return DriveMotorController(conn)

    def set_motor_velocities(self, left, right):
        ...

    def set_linear_velocity(self, v: float):
        """
        :param v: Linear velocity in meters per second.
        """

        self._write_then_read_ack(Request.set_linear_velocity(v))

    def set_angular_velocity(self, rad: float = None, deg: float = None):
        """
        :param rad: Angular velocity in radians per second.
        :param deg: Angular velocity in degrees per second.
        """

        w = _as_rad(rad, deg)
        self._write_then_read_ack(Request.set_angular_velocity(w))
        self._last_target_heading = None

    def set_target_heading(self, rad: float = None, deg: float = None):
        """
        :param rad: The heading (yaw) to maintain, in radians.
        :param deg: The heading (yaw) to maintain, in degrees.
        """

        heading = trunc_angle(_as_rad(rad, deg))
        self._write_then_read_ack(Request.set_target_heading(heading))
        self._last_target_heading = heading

    def change_target_heading(self, rad: float = None, deg: float = None):
        """
        :param rad: The change in heading (yaw) to maintain, in radians.
        :param deg: The change in heading (yaw) to maintain, in degrees.
        """

        if self._last_target_heading is None:
            raise ValueError('Cannot change target heading; not currently targeting any heading.')

        change = _as_rad(rad, deg)
        new_heading = trunc_angle(self._last_target_heading + change)
        self.set_target_heading(rad=new_heading)

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


def _as_rad(rad: Optional[float], deg: Optional[float]) -> float:
    if rad is not None and deg is not None:
        raise ValueError('Must give only one of rad or deg, not both')

    if rad is not None:
        return rad
    elif deg is not None:
        return radians(deg)
    else:
        raise ValueError('Must give one of rad or deg')


class Request:
    SET_LINEAR_VELOCITY_STRUCT = Struct('>ch')
    SET_ANGULAR_VELOCITY_STRUCT = Struct('>ch')
    SET_TARGET_HEADING_STRUCT = Struct('>ch')

    @staticmethod
    def heartbeat() -> bytes:
        return b'\x00'

    @staticmethod
    def get_status() -> bytes:
        return b'\x01'

    @staticmethod
    def set_linear_velocity(v: float) -> bytes:
        v_cm = round(v * 100)
        return Request.SET_LINEAR_VELOCITY_STRUCT.pack(b'\x02', v_cm)

    @staticmethod
    def set_angular_velocity(w: float) -> bytes:
        w_centirad = round(w * 100)
        return Request.SET_ANGULAR_VELOCITY_STRUCT.pack(b'\x03', w_centirad)

    @staticmethod
    def set_target_heading(heading: float) -> bytes:
        heading_centirad = round(heading * 100)
        return Request.SET_TARGET_HEADING_STRUCT.pack(b'\x04', heading_centirad)


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
    heading: 'BodyHeading'


@dataclass
class BodyVelocity:
    linear_meters_per_s: float
    angular_rad_per_s: float


@dataclass
class BodyHeading:
    roll: float
    pitch: float
    yaw: float


class MotorControlError(Exception):
    pass


def main():
    from math import pi

    motors = DriveMotorController.connect('/dev/ttyACM0')

    speed = 0.2

    while True:
        motors.set_linear_velocity(speed)
        time.sleep(16)

        motors.set_linear_velocity(0.)
        motors.change_target_heading(-pi / 2)
        time.sleep(3)

        motors.set_linear_velocity(speed)
        time.sleep(12)

        motors.set_linear_velocity(0.)
        motors.change_target_heading(-pi / 2)
        time.sleep(3)


if __name__ == '__main__':
    main()
