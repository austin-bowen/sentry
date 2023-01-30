import requests

IFTTT_WEBHOOK_URL = 'https://maker.ifttt.com/trigger/{}/with/key/{}'


class IFTTTWebhook:
    def __init__(self, key: str):
        self.__key = key

    def trigger(self, event: str, value1: str = None, value2: str = None, value3: str = None) -> None:
        url = IFTTT_WEBHOOK_URL.format(event, self.__key)

        params = dict(value1=value1, value2=value2, value3=value3)
        params = dict((k, v) for k, v in params.items() if v is not None)

        requests.post(url, params=params)


def main():
    from sentrybot.config.main import config

    key = config.ifttt.webhooks.key
    ifttt = IFTTTWebhook(key)

    ifttt.trigger('sentry_message', value1='Hello, world!')


if __name__ == '__main__':
    main()
