from socket import gethostname

from sentrybot.config.config import Config
from sentrybot.config.secrets import secrets
from sentrybot.notification import IFTTTNotifier, ConsoleNotifier

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
    notifier=(IFTTTNotifier.build(secrets.ifttt.webhooks.key.value)
              if IS_SENTRY else ConsoleNotifier()),
    platform=SENTRY if IS_SENTRY else DEV,
    website=Config(
        host='0.0.0.0',
        port=8080,
    ),
    camera=Config(
        framerate=5,
        print_fps=False,
        # resolution=(512, 384),
        resolution=(648, 486),
        # resolution=(800, 600),
        # resolution=(1024, 768),
        stream=Config(
            host='0.0.0.0',
            port=8081,
        )
    ),
    status_report=Config(
        wifi=Config(
            interface='wlan0' if IS_SENTRY else 'wlp0s20f3'
        )
    )
)

config.update(secrets)

if __name__ == '__main__':
    print(f'config={config}')
