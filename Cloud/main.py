# -*- coding: utf-8 -*-

# [START app]
import logging

# [START imports]
from flask import Flask, render_template, request
from flask import url_for, send_from_directory, jsonify
import requests

from requests.exceptions import ConnectionError, RequestException, Timeout
# [END imports]

# [START create_app]
app = Flask(__name__)
# [END create_app]

# [Device number]
LED = 0
WIND = 1
FEED = 2

# [Arduino status variables]
LED_STATE = False  # led on/off
WIND_STATE = False # wind on/off

# [SensorData]
TEMP = 0  # 온도
HUMD = 0  # 습도

requests.adapters.DEFAULT_RETRIES = 30


# [KaKao Plus Friends]
@app.route('/keyboard')
def keyboard():
    key_list = ["LED", "LED STATUS", "TEMP", "WIND", "HUMD", "FEED"]
    keyboard = {"type": "buttons", "buttons": key_list}

    resp = jsonify(keyboard)
    resp.headers['Content-type'] = "application/json; charset=utf-8"
    return resp


@app.route('/message', methods=['POST'])
def get_message():
    # Load arduino state variables
    global LED_STATE
    global WIND_STATE
    global FEED_STATE

    key_list = ["LED", "LED STATUS", "TEMP", "WIND", "HUMD", "FEED"]
    keyboard = {"type": "buttons", "buttons": key_list}

    # Get data from kakao
    content = request.get_json()
    text = content['content']

    try:
        if text == "LED":
            LED_STATE = not LED_STATE
            payload = {"sensor": LED}
            r = requests.get("http://211.107.138.41:81", params=payload, timeout=5)
            if LED_STATE:
                MESSAGE = "Now LED ON"
            else:
                MESSAGE = "Now LED OFF"

        elif text == "LED STATUS":
            if LED_STATE:
                MESSAGE = "LED STATE is ON"
            else:
                MESSAGE = "LED STATE is OFF"

        elif text == "TEMP":
            MESSAGE = "temperature is %.2f °C" % TEMP

        elif text == "HUMD":
            MESSAGE = "Humidity is %.2f " % HUMD

        elif text == "WIND":
            WIND_STATE = not WIND_STATE
            payload = {"sensor": WIND}
            r = requests.get("http://211.107.138.41:81", params=payload, timeout=5)
            if WIND_STATE:
                MESSAGE = "Now, Wind sys turn on"
            else:
                MESSAGE = "Now, Wind sys turn off"

        elif text == "FEED":
            payload = {"sensor": FEED}
            r = requests.get("http://211.107.138.41:81", params=payload, timeout=5)
            MESSAGE = "Now, FEED !!"

        else:
            MESSAGE = "WTF"

    except ConnectionError as e:
        MESSAGE = "Please wait a moment and try again."

    except Timeout as e:
        MESSAGE = "Please wait a moment and try again(timeout)"

    finally:
        text_content = {"text": MESSAGE}
        message = {"message": text_content, "keyboard": keyboard}
        return jsonify(message)


@app.route('/sensor')
def led_handler():
    global TEMP
    global HUMD

    if request.args.get('temp', '') is not '':
        TEMP = float(request.args.get('temp'))

    if request.args.get('humd', '') is not '':
        HUMD = float(request.args.get('humd'))

    return "DONE"


@app.errorhandler(500)
def server_error(e):
    # Log the error and stacktrace.
    logging.exception('An error occurred during a request.')
    return 'An internal error occurred.', 500
# [END app]
