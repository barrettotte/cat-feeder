
class Control {

  constructor(slider, display, units, defaultValue) {
    this.slider = slider;
    this.display = display;
    this.units = units;
    this.defaultValue = defaultValue;

    this.listeners = {};
    this.refreshDisplay();
    this.slider.oninput = () => this.refreshDisplay();
  }

  get value() {
    return this.slider.value;
  }

  set value(value) {
    this.slider.value = value;
    this.refreshDisplay();
  }

  set disabled(disabled) {
    this.slider.disabled = disabled;
  }

  static createFromPrefix(prefix, units, defaultValue) {
    return new Control(
      document.getElementById(`${prefix}-slider`), 
      document.getElementById(`${prefix}-display`), 
      units,
      defaultValue
    );
  }

  refreshDisplay() {
    this.display.innerHTML = `${this.value} ${this.units}`;
    this.emit('refreshDisplay');
  }

  reset = () => this.value = this.defaultValue;
  addEventListener = (method, callback) => this.listeners[method] = callback;
  removeEventListener = (method) => delete this.listeners[method];

  emit(method, payload=null) {
    const callback = this.listeners[method];
    if (typeof callback === 'function') {
      callback(payload);
    }
  }
}

const catFeederMock = 'http://localhost:8080';
const catFeederEsp8266 = 'http://192.168.1.83';
const catFeeder = catFeederMock;

const defaultSpeedPercent = 50;
const defaultDuration = 10;
const maxSpeed = 255;
const backendDelaySecs = 2;

const speedCtrl = Control.createFromPrefix('speed', '%', defaultSpeedPercent);
const durationCtrl = Control.createFromPrefix('duration', 'second(s)', defaultDuration);
const viewButton = document.getElementById('view-button');
const saveButton = document.getElementById('save-button');
const resetButton = document.getElementById('reset-button');
const feedButton = document.getElementById('feed-button');
const curlCmd = document.getElementById('curl-cmd');

/*** functions ***/

const convertSpeedToPercent = (speed) => Math.floor((speed / maxSpeed) * 100);
const convertPercentToSpeed = (percent) => Math.ceil(maxSpeed * (percent / 100));
const failedRequestError = (resp) => new Error(`${resp.status} - ${resp.statusText}`);

function setCurlCmd() {
  const d = durationCtrl.value;
  const s = convertPercentToSpeed(speedCtrl.value);
  curlCmd.innerHTML = `curl -X POST ${catFeeder}/feed?d=${d}&s=${s}`;
}

function resetControls() {
  durationCtrl.reset();
  speedCtrl.reset();
}

function disableControls(disabled) {
  speedCtrl.disabled = disabled;
  durationCtrl.disabled = disabled;
  viewButton.disabled = disabled;
  saveButton.disabled = disabled;
  resetButton.disabled = disabled;
  feedButton.disabled = disabled;
}

async function feedButtonAction() {
  try {
    const d = parseInt(durationCtrl.value);
    const s = convertPercentToSpeed(speedCtrl.value);
    const resp = await fetch(`${catFeeder}/feed?d=${d}&s=${s}`, {method: 'POST'});

    if (!resp.ok) {
      throw failedRequestError(resp);
    } else {
      disableControls(true);
      feedButton.innerText = 'Feeding...';
      await new Promise(r => setTimeout(r, (d + backendDelaySecs) * 1000));
      disableControls(false);
      feedButton.innerText = 'Feed the Cat!';
    }
  } catch (err) {
    console.error('Error occurred while feeding cat on cat feeder', err);
  }
}

async function saveButtonAction() {
  try {
    const resp = await fetch(`${catFeeder}/config`, {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({
        duration: durationCtrl.value, 
        speed: convertPercentToSpeed(speedCtrl.value)
      })
    });
    if (!resp.ok) {
      throw failedRequestError(resp);
    }
  } catch (err) {
    console.error('Error occurred saving config.json on cat feeder', err);
  }
}

async function resetButtonAction() {
  try{
    const resp = await fetch(`${catFeeder}/reset`, {method: 'POST'});
    if (!resp.ok) {
      throw failedRequestError(resp);
    } else {
      resetControls();
    }
  } catch (err) {
    console.error('Error occurred resetting config.json on cat feeder', err);
  }
}

async function loadConfig() {
  try {
    const resp = await fetch(`${catFeeder}/config`, {headers: {'Content-Type': 'application/json'}});
    if (resp.ok) {
      const data = await resp.json();
      durationCtrl.value = data['duration'];
      speedCtrl.value = convertSpeedToPercent(data['speed']);
    } else {
      throw failedRequestError(resp);
    }
  } catch (err) {
    console.warn('Failed to load config from cat feeder. Using client defaults.', err);
    resetControls();
  }
}

/*** event binding ***/

window.addEventListener('load', async (_) => await loadConfig());
durationCtrl.addEventListener('refreshDisplay', () => setCurlCmd());
speedCtrl.addEventListener('refreshDisplay', () => setCurlCmd());

feedButton.onclick = async () => await feedButtonAction();
viewButton.onclick = () => window.open(`${catFeeder}/config.json`, '_blank');
saveButton.onclick = async () => await saveButtonAction();
resetButton.onclick = async () => await resetButtonAction();
