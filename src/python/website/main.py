import os
import time

from flask import Flask, render_template
from flask_simplelogin import SimpleLogin, login_required
from flask_socketio import SocketIO
from serial import SerialException

from sentrybot.config.main import config
from sentrybot.motorcontrol import DriveMotorController
from sentrybot.users import login_checker
from status import StatusEmitter

app = Flask(__name__)
app.config['SECRET_KEY'] = config.website.secret_key.value

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


@app.route('/')
@login_required
def index():
    return render_template(
        'index.html',
    )


@app.route('/main.js')
@login_required
def main_js():
    return render_template(
        'main.js',
        video_stream_port=config.camera.stream.port,
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
        socketio.sleep(1)
        os.system('sudo shutdown -h now')


@socketio.on('reboot')
def handle_reboot():
    print('Rebooting!')

    if config.is_sentry:
        socketio.sleep(1)
        os.system('sudo reboot')


@socketio.on('restart_service')
def handle_restart_service():
    print('Restarting service!')

    if config.is_sentry:
        socketio.sleep(1)
        os.system('sudo systemctl restart sentry.service')


def main():
    print(f'config={config}\n')

    SimpleLogin(app, login_checker=login_checker)

    StatusEmitter.build(socketio)

    socketio.run(
        app,
        host=config.website.host,
        port=config.website.port,
        debug=True,
        use_reloader=False,
    )


if __name__ == '__main__':
    main()
