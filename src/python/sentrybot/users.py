import time
from typing import Dict

from sentrybot.config.main import config


def login_checker(creds: Dict[str, str]) -> bool:
    # Sleep to prevent rapid password checking
    time.sleep(1)

    username = creds['username']
    password = creds['password']

    users = config.website.users.value

    if username in users and password == users[username]:
        print(f'AUTH: User logged in: {username}')
        return True
    else:
        print(f'AUTH: User failed to log in: {username}')
        return False
