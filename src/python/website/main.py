from flask import Flask, render_template
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app)


class MotorController:
    def drive(self, linear, angular):
        print(f'drive: {linear}, {angular}')

    def stop(self):
        print('stop')


motor_controller = MotorController()


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
    motor_controller.drive(linear, angular)


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
