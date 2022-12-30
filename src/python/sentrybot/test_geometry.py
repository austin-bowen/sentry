import unittest
from math import pi
from unittest import TestCase

from parameterized import parameterized

from sentrybot.geometry import trunc_angle


class GeometryTest(TestCase):
    @parameterized.expand([
        (0., 0.),
        (pi - 0.001, pi - 0.001),
        (pi, -pi),
        (-pi, -pi),
        (-pi - 0.001, pi - 0.001),
        (2 * pi, 0.),
        (9 * pi / 2, pi / 2),
    ])
    def test_trunc_angle(self, angle: float, expected: float):
        result = trunc_angle(angle)
        self.assertAlmostEqual(expected, result)


if __name__ == '__main__':
    unittest.main()
