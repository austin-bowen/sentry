import time
from typing import Dict

from sentrybot.config.main import config


def login_checker(creds: Dict[str, str]) -> bool:
    # Sleep to prevent rapid password checking
    time.sleep(1)

    username = creds['username']
    password = creds['password']

    users = config.website.users.value
    return username in users and password == users[username]
