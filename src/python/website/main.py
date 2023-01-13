from time import monotonic

from flask import Flask, Response, render_template
from flask_socketio import SocketIO

from camera import CameraCapturer
from sentrybot.motorcontrol import DriveMotorController

app = Flask(__name__)
socketio = SocketIO(app)

camera_capturer = CameraCapturer.build()

last_command_t = 0

motor_controller = DriveMotorController.connect('/dev/ttyACM0')


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/video_feed')
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


@socketio.on('connect')
def handle_connect(auth):
    print('Client connected')
    motor_controller.stop()


@socketio.on('disconnect')
def handle_disconnect():
    print('Client disconnected')
    motor_controller.stop()


@socketio.on('motorController.drive')
def handle_motor_controller_drive(linear: float, angular: float):
    global last_command_t

    t = monotonic()
    if (t - last_command_t) < 0.1:
        return

    motor_controller.set_linear_velocity(linear)
    motor_controller.set_angular_velocity(rad=angular)

    last_command_t = monotonic()


@socketio.on('motorController.stop')
def handle_motor_controller_stop():
    motor_controller.stop()


def main():
    socketio.run(
        app,
        host='0.0.0.0',
        port=8080,
        debug=True,
        use_reloader=False,
    )


if __name__ == '__main__':
    main()
