import sys
from abc import abstractmethod

from requests.exceptions import ConnectionError

from sentrybot.ifttt import IFTTTWebhook


class Notifier:
    @abstractmethod
    def notify(self, message: str) -> None:
        ...


class ConsoleNotifier(Notifier):
    def notify(self, message: str) -> None:
        print(f'NOTIFICATION: {message}')


class IFTTTNotifier(ConsoleNotifier):
    def __init__(self, ifttt: IFTTTWebhook):
        self.ifttt = ifttt

    @staticmethod
    def build(ifttt_key: str) -> 'IFTTTNotifier':
        ifttt = IFTTTWebhook(ifttt_key)
        return IFTTTNotifier(ifttt)

    def notify(self, message: str) -> None:
        super().notify(message)

        try:
            self.ifttt.trigger('sentry_message', value1=message)
        except ConnectionError as e:
            print(e, file=sys.stderr)
