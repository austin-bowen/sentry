import cv2
import io
import time

from flask import Response, Flask, render_template
from picamera import PiCamera
from threading import Condition, RLock, Thread


app = Flask(__name__)


camera = PiCamera()
#size = (512, 384)
#size = (800, 600)
size = (1024, 768)
camera.resolution = size
camera.framerate = 10
camera.rotation = 180
time.sleep(2)


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

    def __iter__(self):
        return self

    def __next__(self):
        return self.get_next()

    def run(self) -> None:
        message_prefix = (
            b'--frame\r\n'
            b'Content-Type: image/jpeg\r\n'
            b'\r\n'
        )
        message_suffix = b'\r\n'

        with io.BytesIO() as buffer:
            t0 = time.monotonic()

            for frame in camera.capture_continuous(
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


camera_capturer = CameraCapturer(camera)
camera_capturer.start()


def generate_frames():
    message_prefix = (
        b'--frame\r\n'
        b'Content-Type: image/jpeg\r\n'
        b'\r\n'
    )
    message_suffix = b'\r\n'

    for frame in camera_capturer:
        yield message_prefix + frame + message_suffix


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/video_feed')
def video_feed():
    return Response(
        generate_frames(),
        mimetype='multipart/x-mixed-replace; boundary=frame'
    )


def main():
    app.run(
        host='0.0.0.0',
        port=8080,
        debug=True,
        threaded=True,
        use_reloader=False,
    )


if __name__ == '__main__':
    main()
