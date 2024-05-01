import logging
import cmd
import json
from titles import titles_list, LANG_DEFAULT
from pathlib import Path

CONFIG_DIR  = ".el_garden"
CONFIG_FILE = "telegram_config.json"
FILE_CFG = Path.home() / CONFIG_DIR / CONFIG_FILE

def authentication(message): 
    global user_name
    user_id = message.from_user.id
    logging.info(f"User id = {user_id} {message.from_user.first_name} {message.from_user.last_name}")
    with open(FILE_CFG, 'r') as f:
        config = json.load(f)
        if "user_list" in config:
            for user in config["user_list"]:
                if user_id == user:
                    user_name = message.from_user.first_name
                    return True
    logging.warning(f"Unknown user id.")
    return False

def get_token():
    filepath = Path(FILE_CFG)
    if Path(filepath).exists():
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            if "token" in config:
                return config["token"]
            else:
                print(f'The token is not in the telegram configuration.')
                print(f'You should use config.py to config telegram bot.')
                return None
    else:
        print(f'The telegram configuration is absent.')
        print(f'You should use config.py to config telegram bot.')
        return None


def get_titles():
    filepath = Path(FILE_CFG)
    lang = LANG_DEFAULT
    if Path(filepath).exists():
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            if "lang" in config:
                lang = config["lang"]
    return titles_list[lang]


def set_lang(lang):
    filepath = Path(FILE_CFG)
    if Path(filepath).exists():
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            config["lang"] = lang
            with open(FILE_CFG, 'w') as f:
                json.dump(config, f, indent=4)
                return True
    else:
        logging.error(f"Failed to change the configuration.")
    return False


class TelegramConfigShell(cmd.Cmd):
    intro = 'Welcome to the telegram bot configurator shell.   Type help or ? to list commands.\n'
    prompt = '(telegram) '
    file = None

    def do_show_token(self, arg):
        'Show current token:  SHOW_TOKEN'
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            if "token" in config:
                print(f'Current token: {config["token"]}')
            else:
                print(f'Token is not specified yet.')

    def do_token(self, arg):
        'Set/change token:  TOKEN 1234567890:9m375vmpw9e8trvw47n468y5ht'
        token = parse_str(arg)
        if len(token) != 1:
            print(f'You should enter only one token')
            return
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            config["token"] = str(token[0])
            with open(FILE_CFG, 'w') as f:
                json.dump(config, f, indent=4)

    def do_show_users(self, arg):
        'Show current user IDs:  SHOW_USERS'
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            if "user_list" in config:
                for user in config["user_list"]:
                    print(f'{user}')
            else:
                print(f'No users have been added yet.')

    def do_add_user(self, arg):
        'Set/change token:  ADD_USER 1234567890 1029384756'
        users = parse_int(arg)
        if len(users) == 0:
            print(f'You should enter at least one user ID')
            return
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            if "user_list" not in config:
                config["user_list"] = []
            for user in users:
                print(f'user: {user} {type(user)}')
                config["user_list"].append(user)
            with open(FILE_CFG, 'w') as f:
                json.dump(config, f, indent=4)

    def do_del_user(self, arg):
        'Delete token:  DEL_USER 1234567890 1029384756'
        users = parse_int(arg)
        if len(users) == 0:
            print(f'You should enter at least one user ID')
            return
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            for user in users:
                if user in config["user_list"]:
                    config["user_list"].remove(user)
                else:
                    print(f'User ID {user} is not in the user list.')
            with open(FILE_CFG, 'w') as f:
                json.dump(config, f, indent=4)

    def do_show_chats(self, arg):
        'Show current chat IDs for notifications:  SHOW_CHATS'
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            if "chat_list" in config:
                for chat in config["chat_list"]:
                    print(f'{chat}')
            else:
                print(f'No chat IDs have been added yet.')

    def do_add_chat(self, arg):
        'Set/change token:  ADD_CHAT -109234567890 -121029384756'
        chats = parse_int(arg)
        if len(chats) == 0:
            print(f'You should enter at least one chat id')
            return
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            if "chat_list" not in config:
                config["chat_list"] = []
            for chat in chats:
                if chat < 0:
                    config["chat_list"].append(chat)
                else:
                    print(f'Uncorrect chat ID {chat}: chat ID must be a negative number.')
            with open(FILE_CFG, 'w') as f:
                json.dump(config, f, indent=4)

    def do_del_chat(self, arg):
        'Delete token:  DEL_CHAT 1234567890 1029384756'
        chats = parse_int(arg)
        if len(chats) == 0:
            print(f'You should enter at least one chat')
            return
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            for chat in chats:
                if chat in config["chat_list"]:
                    config["chat_list"].remove(chat)
                else:
                    print(f'Chat ID {chat} is not in the chat list.')
            with open(FILE_CFG, 'w') as f:
                json.dump(config, f, indent=4)

    def do_quit(self, arg):
        'Exit the app'
        exit(0)

    def do_exit(self, arg):
        'Exit the app'
        exit(0)

def parse_int(arg):
    'Convert a series of zero or more numbers to an argument tuple'
    return tuple(map(int, arg.split()))

def parse_str(arg):
    'Convert a series of zero or more numbers to an argument tuple'
    return tuple(map(str, arg.split()))


if __name__ == '__main__':
    filepath = Path(FILE_CFG)
    if not Path(filepath).exists():
        config = {}
        filepath.parent.mkdir(parents=True, exist_ok=True)
        with open(filepath, 'w') as f:
            json.dump(config, f)

    TelegramConfigShell().cmdloop()
