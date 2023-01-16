from socket import gethostname

from sentrybot.config.config import Config
from sentrybot.config.secrets import secrets

HOSTNAME = gethostname()

DEV = 'dev'
SENTRY = 'sentry'
SENTRY_HOSTNAME = SENTRY

IS_SENTRY = HOSTNAME == SENTRY_HOSTNAME

config = Config(
    hostname=HOSTNAME,
    is_sentry=IS_SENTRY,
    motor_control=Config(
        serial=Config(
            path='/dev/ttyACM0',
        ),
    ),
    platform=SENTRY if IS_SENTRY else DEV,
    website=Config(
        host='0.0.0.0',
        port=8080,
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
        )
    )
)

config.update(secrets)

if __name__ == '__main__':
    print(f'config={config}')
