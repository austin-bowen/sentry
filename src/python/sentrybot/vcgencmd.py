import subprocess
from dataclasses import dataclass

VCGENCMD = 'vcgencmd'
GET_THROTTLED = (VCGENCMD, 'get_throttled')
IS_UNDER_VOLTAGE = 1 << 0
IS_FREQ_CAPPED = 1 << 1
IS_THROTTLED = 1 << 2
IS_TEMP_LIMIT = 1 << 3
UNDER_VOLTAGE_OCCURRED = 1 << 16
FREQ_CAP_OCCURRED = 1 << 17
THROTTLING_OCCURRED = 1 << 18
TEMP_LIMIT_OCCURRED = 1 << 19


def get_throttled() -> 'ThrottlingResult':
    throttling = subprocess.check_output(GET_THROTTLED, text=True)
    throttling = throttling.partition('=')[2]
    throttling = throttling.strip()
    throttling = int(throttling, 0)

    return ThrottlingResult(
        is_under_voltage=bool(throttling & IS_UNDER_VOLTAGE),
        is_freq_capped=bool(throttling & IS_FREQ_CAPPED),
        is_throttled=bool(throttling & IS_THROTTLED),
        is_temp_limit=bool(throttling & IS_TEMP_LIMIT),
        under_voltage_occurred=bool(throttling & UNDER_VOLTAGE_OCCURRED),
        freq_cap_occurred=bool(throttling & FREQ_CAP_OCCURRED),
        throttling_occurred=bool(throttling & THROTTLING_OCCURRED),
        temp_limit_occurred=bool(throttling & TEMP_LIMIT_OCCURRED),
    )


@dataclass
class ThrottlingResult:
    is_under_voltage: bool
    is_freq_capped: bool
    is_throttled: bool
    is_temp_limit: bool
    under_voltage_occurred: bool
    freq_cap_occurred: bool
    throttling_occurred: bool
    temp_limit_occurred: bool
