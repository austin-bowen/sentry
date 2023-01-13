from flask import Flask, render_template
from flask_socketio import SocketIO
from time import monotonic

from sentrybot.motorcontrol import DriveMotorController

app = Flask(__name__)
socketio = SocketIO(app)

last_command_t = 0


class DummyMotorController:
    def set_linear_velocity(self, linear: float):
        print(f'linear={linear}')

    def set_angular_velocity(self, rad: float):
        print(f'angular={rad}')

    def stop(self):
        print('stop')


# motor_controller = DummyMotorController()
motor_controller = DriveMotorController.connect('/dev/ttyACM0')


@app.route('/')
def index():
    return render_template('index.html')


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
