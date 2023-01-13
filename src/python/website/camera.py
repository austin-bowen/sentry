import io
import time
from threading import Condition, RLock, Thread

from picamera import PiCamera


class CameraCapturer(Thread):
    def __init__(
            self,
            camera,
            resize=None,
            name: str = 'CameraCapturer',
            daemon: bool = True,
            **kwargs
    ):
        super().__init__(name=name, daemon=daemon, **kwargs)

        self.camera = camera
        self.resize = resize

        self._condition = Condition()
        self._latest = None
        self._latest_lock = RLock()

    @staticmethod
    def build() -> 'CameraCapturer':
        camera = PiCamera()
        # size = (512, 384)
        # size = (800, 600)
        size = (1024, 768)
        camera.resolution = size
        camera.framerate = 10
        camera.rotation = 180
        time.sleep(2)

        cap = CameraCapturer(camera)
        cap.start()
        return cap

    def __iter__(self):
        return self

    def __next__(self):
        return self.get_next()

    def run(self) -> None:
        with io.BytesIO() as buffer:
            t0 = time.monotonic()

            for frame in self.camera.capture_continuous(
                    buffer,
                    format='jpeg',
                    use_video_port=True,
                    resize=self.resize,
            ):
                buffer.seek(0)
                latest = buffer.read()

                with self._condition:
                    self._set_latest(latest)
                    self._condition.notify_all()

                buffer.seek(0)
                buffer.truncate()

                t1 = time.monotonic()
                fps = round(1. / (t1 - t0))
                print(f'FPS={fps}')
                t0 = time.monotonic()

    def get_next(self):
        with self._condition:
            self._condition.wait()
            return self._get_latest()

    def _get_latest(self):
        with self._latest_lock:
            return bytes(self._latest)

    def _set_latest(self, latest):
        with self._latest_lock:
            self._latest = latest