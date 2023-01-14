import time

from flask import Response, Flask, render_template
from picamera import PiCamera

from camera import CameraCapturer
from sentrybot.config.main import config

app = Flask(__name__)

camera = PiCamera()
camera.resolution = config.camera.resolution
camera.framerate = config.camera.framerate
camera.rotation = 180
time.sleep(2)

camera_capturer = CameraCapturer(camera, print_fps=config.camera.print_fps)
camera_capturer.start()


@app.route('/')
def index():
    return render_template('index.html')


@app.route(config.camera.stream.path)
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
    app.run(
        host=config.camera.stream.host,
        port=config.camera.stream.port,
        debug=True,
        threaded=True,
        use_reloader=False,
    )


if __name__ == '__main__':
    main()
