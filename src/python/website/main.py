from flask import Flask, render_template
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app)


@app.route('/')
def index():
    return render_template('index.html')


@socketio.on('disconnect')
def test_disconnect():
    handle_motor_controller_stop()


@socketio.on('motorController.drive')
def handle_motor_controller_drive(args):
    print(f'drive: {args}')


@socketio.on('motorController.stop')
def handle_motor_controller_stop():
    print('stop')


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
