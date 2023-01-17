from dataclasses import dataclass
from typing import Tuple

import psutil
from flask_socketio import SocketIO


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
    def get_status(self) -> 'Status':
        cpu_usage_avg, cpu_usage_max = self.get_cpu_usage()

        return Status(
            loadavg=self.get_loadavg(),
            cpu_usage_avg=cpu_usage_avg,
            cpu_usage_max=cpu_usage_max,
            mem_usage=psutil.virtual_memory().percent,
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


@dataclass
class Status:
    loadavg: str
    cpu_usage_avg: float
    cpu_usage_max: float
    mem_usage: float

    def to_html(self) -> str:
        return rf'''
            <strong>loadavg:</strong> {self.loadavg}<br>
            <strong>CPU (avg/max):</strong> {round(self.cpu_usage_avg)}/{round(self.cpu_usage_max)}%<br>
            <strong>Memory:</strong> {round(self.mem_usage)}%
        '''.strip()
