# mock cat-feeder backend

import json
import time
from flask import Flask, Response, request, send_from_directory

DEFAULT_DURATION = 5
DEFAULT_SPEED = 128
DATA_DIR = '../data'

app = Flask(__name__)

@app.route('/', methods=['GET'])
def index():
    return send_from_directory(DATA_DIR, 'index.html')

@app.route('/styles.css', methods=['GET'])
def get_styles_css():
    return send_from_directory(DATA_DIR, 'styles.css')

@app.route('/config.json', methods=['GET'])
def get_config_json():
    return send_from_directory(DATA_DIR, 'config.json')

@app.route('/servo', methods=['POST'])
def set_manual_servo():
    print(f'set servo: a={request.args.get("a")}')
    return Response('', status=200, mimetype='text/plain;charset=utf-8')

@app.route('/motor', methods=['POST'])
def set_manual_motor():
    print(f'set motor: en={request.args.get("en")},s={request.args.get("s")}')
    return Response('', status=200, mimetype='text/plain;charset=utf-8')

@app.route('/config', methods=['GET'])
def get_config():
    return Response(json.dumps({'duration': DEFAULT_DURATION, 'speed': DEFAULT_SPEED}), 
        status=200, mimetype='application/json;charset=utf-8')

@app.route('/config', methods=['POST'])
def set_config():
    print(f'set config to {request.get_json()}')
    return Response('', status=200, mimetype='text/plain;charset=utf-8')

@app.route('/reset', methods=['POST'])
def reset():
    print('reset config')
    return Response('', status=200, mimetype='text/plain;charset=utf-8')

@app.route('/feed', methods=['POST'])
def feed():
    d = request.args['d'] if 'd' in request.args else DEFAULT_DURATION
    s = request.args['s'] if 's' in request.args else DEFAULT_SPEED
    print(f'feeding d={d},s={s}')
    time.sleep(int(d))
    return Response('Fed the cat!', status=200, mimetype='text/plain;charset=utf-8')

if __name__ == '__main__':
    app.run(host='localhost', port=8080, debug=True)
