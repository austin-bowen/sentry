from math import floor, pi


def trunc_angle(angle: float) -> float:
    """Truncates the angle (in radians) to the range [-pi, pi)."""

    two_pi = 2 * pi

    angle += pi
    angle -= two_pi * floor(angle // two_pi)
    angle -= pi

    return angle
