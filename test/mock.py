# mock cat-feeder backend

import json
from flask import Flask, Response, request, send_from_directory

DEFAULT_DURATION = 5
DEFAULT_SPEED = 128
DATA_DIR = '../data'

config = {'duration': DEFAULT_DURATION, 'speed': DEFAULT_SPEED}

app = Flask(__name__)

@app.route('/', methods=['GET'])
def index():
    return send_from_directory(DATA_DIR, 'index.html')

@app.route('/styles.css', methods=['GET'])
def get_styles_css():
    return send_from_directory(DATA_DIR, 'styles.css')

@app.route('/cat-feeder.js', methods=['GET'])
def get_js():
    return send_from_directory(DATA_DIR, 'cat-feeder.js')

@app.route('/config.json', methods=['GET'])
def get_config_json():
    return Response(json.dumps(config), status=200, mimetype='application/json;charset=utf-8')

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
    return Response(json.dumps(config), status=200, mimetype='application/json;charset=utf-8')

@app.route('/config', methods=['POST'])
def set_config():
    new_config = request.get_json()
    config['duration'] = new_config['duration']
    config['speed'] = new_config['speed']
    print(f'set config to {config}')
    return Response('', status=200, mimetype='text/plain;charset=utf-8')

@app.route('/reset', methods=['POST'])
def reset():
    config['duration'] = DEFAULT_DURATION
    config['speed'] = DEFAULT_SPEED
    print('reset config')
    return Response('', status=200, mimetype='text/plain;charset=utf-8')

@app.route('/feed', methods=['POST'])
def feed():
    d = request.args['d'] if 'd' in request.args else config['duration']
    s = request.args['s'] if 's' in request.args else config['speed']
    print(f'feeding with d={d},s={s}')
    return Response('Fed the cat!', status=200, mimetype='text/plain;charset=utf-8')

if __name__ == '__main__':
    app.run(host='localhost', port=8080, debug=True)
