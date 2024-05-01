#!/usr/bin/env python3

import sys
import socket
import json
import logging

server_addr = ('localhost', 2300)

def get_state():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    out = None

    try:
        s.connect(server_addr)
        print("Connected to {:s}".format(repr(server_addr)))

        request = '{ "command": "get_sys_state", "param": "all"}'

        buffer = json.loads(request)
        out = str.encode(json.dumps(buffer))
        nsent = s.send(out)
        if nsent > 0:
            buff = s.recv(1024)
            result = json.loads(buff.decode())

            if "error" in result:
                error = result["error"]
                if error == 0:
                    if "is_running" in result and \
                       "is_light_on" in result and \
                       "is_pump_on" in result and \
                       "pH" in result and \
                       "EC" in result :
                        out = result
                    else:
                        logging.error(f"Unexpected answer: {json.dumps(result, indent = 4)}")
                else:
                    logging.error(f"Error: {error}\n {json.dumps(result, indent = 4)}")
            else:
                logging.error(f"Unexpected answer: {json.dumps(result, indent=4)}")
        else:
            logging.error(f"Failed to send data. error: {nsent}")


    except AttributeError as ae:
        logging.error("Error creating the socket: {}".format(ae))
    except socket.error as se:
        logging.error("Exception on socket: {}".format(se))
    finally:
        logging.error("Closing socket")
        s.close()
    return out


def get_config():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    out = None

    try:
        s.connect(server_addr)
        logging.info("Connected to {:s}".format(repr(server_addr)))

        request = '{ "command": "get_config", "param": "all"}'

        buffer = json.loads(request)
        out = str.encode(json.dumps(buffer))
        nsent = s.send(out)
        if nsent > 0:
            buff = s.recv(1024)
            result = json.loads(buff.decode())

            if "error" in result:
                error = result["error"]
                if error == 0:
                    if "light_on" in result and \
                            "light_off" in result and \
                            "pump_on" in result and \
                            "pump_off" in result and \
                            "pump_night" in result:
                        out = result
                    else:
                        logging.error(f"Unexpected answer: {json.dumps(result, indent=4)}")
                else:
                    logging.error(f"Error: {error}\n {json.dumps(result, indent=4)}")
            else:
                logging.error(f"Unexpected answer: {json.dumps(result, indent=4)}")
        else:
            logging.error(f"Failed to send data. error: {nsent}")

    except AttributeError as ae:
        logging.error("Error creating the socket: {}".format(ae))
    except socket.error as se:
        logging.error("Exception on socket: {}".format(se))
    finally:
        logging.error("Closing socket")
        s.close()
    return out


def set_config(data):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    error = -1

    try:
        if "running" in data or \
           "light_on" in data or \
           "light_off" in data or \
           "pump_on" in data or \
           "pump_off" in data or \
           "pump_night" in data:

            s.connect(server_addr)
            logging.info("Connected to {:s}".format(repr(server_addr)))

            request = '{ "command": "set_config", "param": {}}'
            request_json = json.loads(request)
            request_json["param"] = data;

            out = str.encode(json.dumps(request_json))
            nsent = s.send(out)
            if nsent > 0:
                s.settimeout(2)
                buff = s.recv(1024)
                result = json.loads(buff.decode())

                if "error" in result:
                    error = result["error"]
                    if error == 0:
                        logging.error("Successfully configured")
                    else:
                        logging.error(f"Error: {error}\n {json.dumps(result, indent=4)}")
                else:
                    logging.error(f"Unexpected answer: {json.dumps(result, indent=4)}")
            else:
                logging.error(f"Failed to send data. error: {nsent}")
        else:
            logging.error(f"Unexpected erguments: {json.dumps(data, indent = 4)}")

    except AttributeError as ae:
        logging.error("Error creating the socket: {}".format(ae))
    except socket.error as se:
        logging.error("Exception on socket: {}".format(se))
    finally:
        logging.error("Closing socket")
        s.close()
    return error


if __name__ == "__main__":
    argumentList = sys.argv[1:]

    if len(argumentList) > 0:
        if argumentList[0] == "set1":
            data = {'light_on': 19, 'light_off': 8, 'pump_on': 15, 'pump_off': 25, 'pump_night': 2, 'pH': 698, 'EC': 1543}
            res = set_config(data)
            print(f'set_config() res: {res}\n\n')
        
        elif argumentList[0] == "set2":
            data = {'light_on': 10, 'light_off': 20, 'pump_on': 15, 'pump_off': 25, 'pump_night': 2, 'pH': 650, 'EC': 1200}
            res = set_config(data)
            print(f'set_config() res: {res}\n\n')

        elif argumentList[0] == "start":
            data = {'running': True}
            res = set_config(data)
            print(f'set_config() res: {res}\n\n')

        elif argumentList[0] == "stop":
            data = {'running': False}
            res = set_config(data)
            print(f'set_config() res: {res}\n\n')

        else:
            ptint(f"Unknown command '{argumentList[0]}'")

    res = get_config()
    print(f'get_config() res: {json.dumps(res, indent = 4)}\n\n')

    res = get_state()
    print(f'get_state() res: {json.dumps(res, indent = 4)}\n\n')

