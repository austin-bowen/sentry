from functools import cache

from flask import Flask, render_template
from flask_socketio import SocketIO

from sentrybot.config.main import config
from sentrybot.motorcontrol import DriveMotorController

app = Flask(__name__)
socketio = SocketIO(app)


@cache
def motor_controller():
    DriveMotorController.connect(config.motor_control.serial.path)


@app.route('/')
def index():
    return render_template(
        'index.html',
        video_stream_port=config.camera.stream.port,
        video_stream_path=config.camera.stream.path,
    )


@socketio.on('connect')
def handle_connect(auth):
    print('Client connected')
    motor_controller().stop()


@socketio.on('disconnect')
def handle_disconnect():
    print('Client disconnected')
    motor_controller().stop()


@socketio.on('motorController.drive')
def handle_motor_controller_drive(linear: float, angular: float):
    motor_controller().set_linear_velocity(linear)
    motor_controller().set_angular_velocity(rad=angular)


@socketio.on('motorController.stop')
def handle_motor_controller_stop():
    motor_controller().stop()


def main():
    print(f'config={config}\n')

    socketio.run(
        app,
        host=config.website.host,
        port=config.website.port,
        debug=True,
        use_reloader=False,
    )


if __name__ == '__main__':
    main()
