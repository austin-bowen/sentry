import time

from flask import Response, Flask
from flask_simplelogin import SimpleLogin, login_required

from camera import CameraCapturer
from sentrybot.config.main import config
from sentrybot.users import login_checker

app = Flask(__name__)
app.config['SECRET_KEY'] = config.website.secret_key.value

if config.is_sentry:
    from picamera import PiCamera
else:
    print('Using dummy PiCamera')

    # Dummy PiCamera for non-Raspberry Pi platforms
    class PiCamera:
        framerate = 10

        def capture_continuous(self, *args, **kwargs):
            while True:
                yield b''
                time.sleep(1 / self.framerate)


def build_camera():
    camera = PiCamera()
    camera.resolution = config.camera.resolution
    camera.framerate = config.camera.framerate
    camera.rotation = 180
    time.sleep(2)
    return camera


def build_camera_capturer():
    camera_capturer = CameraCapturer(build_camera(), print_fps=config.camera.print_fps)
    camera_capturer.start()
    return camera_capturer


camera_capturer = build_camera_capturer()


@app.route('/')
@login_required
def video_feed():
    return Response(
        generate_frames(),
        mimetype='multipart/x-mixed-replace; boundary=frame'
    )


def generate_frames():
    message_prefix = (
        b'--frame\r\n'
        b'Content-Type: image/jpeg\r\n'
        b'\r\n'
    )
    message_suffix = b'\r\n'

    for frame in camera_capturer:
        yield message_prefix + frame + message_suffix


def main():
    SimpleLogin(app, login_checker=login_checker)

    app.run(
        host=config.camera.stream.host,
        port=config.camera.stream.port,
        debug=True,
        threaded=True,
        use_reloader=False,
    )


if __name__ == '__main__':
    main()
