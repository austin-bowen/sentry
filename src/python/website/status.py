import re
import subprocess
from dataclasses import dataclass
from typing import Tuple, Optional

import psutil
from flask_socketio import SocketIO

from sentrybot.config.main import config
from sentrybot.vcgencmd import get_throttled, ThrottlingResult


class StatusEmitter:
    def __init__(
            self,
            status_supplier: 'StatusSupplier',
            socketio: SocketIO,
            emit_period: float = 3.
    ):
        self.status_supplier = status_supplier
        self.socketio = socketio
        self.emit_period = emit_period

    @staticmethod
    def build(socketio: SocketIO) -> 'StatusEmitter':
        supplier = StatusSupplier()
        emitter = StatusEmitter(supplier, socketio)
        socketio.start_background_task(emitter.run)
        return emitter

    def run(self) -> None:
        while True:
            self.socketio.sleep(self.emit_period)

            try:
                status = self.get_status_msg()
                self.socketio.emit('status', status)
            except RuntimeError as e:
                print(e)
                return
            except Exception as e:
                print(e)

    def get_status_msg(self) -> str:
        status = self.status_supplier.get_status()
        return status.to_html()


class StatusSupplier:
    WIFI_BIT_RATE_PATTERN = re.compile(r'.*Bit Rate=(\d+)', flags=re.DOTALL)

    def get_status(self) -> 'Status':
        cpu_usage_avg, cpu_usage_max = self.get_cpu_usage()

        try:
            throttling = get_throttled()
        except FileNotFoundError:
            throttling = None

        return Status(
            loadavg=self.get_loadavg(),
            cpu_usage_avg=cpu_usage_avg,
            cpu_usage_max=cpu_usage_max,
            mem_usage=psutil.virtual_memory().percent,
            wifi_bit_rate_Mbps=self.get_wifi_bit_rate(),
            throttling=throttling,
        )

    def get_loadavg(self) -> str:
        loadavg = psutil.getloadavg()
        loadavg = (f'{it:.3}' for it in loadavg)
        loadavg = ' '.join(loadavg)

        return loadavg

    def get_cpu_usage(self) -> Tuple[float, float]:
        cpu_usages = psutil.cpu_percent(percpu=True)

        avg_usage = sum(cpu_usages) / len(cpu_usages)
        max_usage = max(cpu_usages)

        return avg_usage, max_usage

    def get_wifi_bit_rate(self) -> Optional[float]:
        iface = config.status_report.wifi.interface

        result = subprocess.run(
            ['iwconfig', iface],
            capture_output=True,
            text=True
        )

        if result.returncode != 0:
            print(f'ERROR: Failed to get WiFi bit rate: returncode={result.returncode}')
            print('--- stderr ---')
            print(result.stderr)
            print('------')
            return None

        match = self.WIFI_BIT_RATE_PATTERN.match(result.stdout)
        if not match:
            print('ERROR: Failed to find bit rate in output:')
            print('--- stdout ---')
            print(result.stdout)
            print('------')
            return None

        return float(match.group(1))


@dataclass
class Status:
    loadavg: str
    cpu_usage_avg: float
    cpu_usage_max: float
    mem_usage: float
    wifi_bit_rate_Mbps: Optional[float]
    throttling: Optional[ThrottlingResult]

    def to_html(self) -> str:
        wifi_speed = (
            f'{round(self.wifi_bit_rate_Mbps)} Mb/s'
            if self.wifi_bit_rate_Mbps is not None else
            f'Unknown'
        )

        result = rf'''
            <strong>loadavg:</strong> {self.loadavg}<br>
            <strong>CPU (avg/max):</strong> {round(self.cpu_usage_avg)}/{round(self.cpu_usage_max)}%<br>
            <strong>Memory:</strong> {round(self.mem_usage)}%<br>
            <strong>WiFi Speed:</strong> {wifi_speed}
        '''

        if self.throttling and self.throttling.is_throttled:
            result += '<br><br><strong style="color: red;">THROTTLING IS OCCURRING!'

            if self.throttling.is_under_voltage:
                result += '<br>- Under-voltage detected'

            if self.throttling.is_temp_limit:
                result += '<br>- Temp limit reached'

            result += '</strong>'

        return result.strip()
