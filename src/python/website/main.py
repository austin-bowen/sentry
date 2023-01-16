import os
import time

from flask import Flask, render_template
from flask_socketio import SocketIO
from serial import SerialException

from sentrybot.config.main import config
from sentrybot.motorcontrol import DriveMotorController

app = Flask(__name__)
socketio = SocketIO(app, logger=True, engineio_logger=True)

if config.is_sentry:
    while True:
        try:
            motor_controller = DriveMotorController.connect(config.motor_control.serial.path)
            break
        except SerialException:
            time.sleep(1)
else:
    class DummyMotorController:
        def stop(self):
            pass

        def set_linear_velocity(self, *args, **kwargs):
            pass

        def set_angular_velocity(self, *args, **kwargs):
            pass


    motor_controller = DummyMotorController()


@app.route('/sentry/')
def index():
    return render_template(
        'index.html',
    )


@app.route('/main.js')
def main_js():
    return render_template(
        'main.js',
        video_stream_port=config.camera.stream.port,
        video_stream_path=config.camera.stream.path,
    )


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
    motor_controller.set_linear_velocity(linear)
    motor_controller.set_angular_velocity(rad=angular)


@socketio.on('motorController.stop')
def handle_motor_controller_stop():
    motor_controller.stop()


@socketio.on('shutdown')
def handle_shutdown():
    print('Shutting down!')

    if config.is_sentry:
        time.sleep(1)
        os.system('sudo shutdown -h now')


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
