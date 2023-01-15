from hashlib import sha512
from socket import gethostname

from sentrybot.config.config import Config
from sentrybot.config.secrets import secrets

HOSTNAME = gethostname()

DEV = 'dev'
SENTRY = 'sentry'
SENTRY_HOSTNAME = SENTRY

config = Config(
    hostname=HOSTNAME,
    motor_control=Config(
        serial=Config(
            path='/dev/ttyACM0',
        ),
    ),
    platform=SENTRY if HOSTNAME == SENTRY_HOSTNAME else DEV,
    website=Config(
        host='0.0.0.0',
        port=8080,
        password_hash=sha512(secrets.website.password).digest(),
    ),
    camera=Config(
        framerate=5,
        print_fps=False,
        # resolution=(512, 384),
        resolution=(800, 600),
        # resolution=(1024, 768),
        stream=Config(
            host='0.0.0.0',
            port=8081,
            path='/video_feed'
        )
    )
)

if __name__ == '__main__':
    print(f'config={config}')
