// pH measure with AS7341 — talks to the board over the Web Serial API using
// the trimmed reflectance-only protocol (see firmware/README.md):
//   -> {"cmd":"get_info"} / {"cmd":"set_gain","idx":N} / {"cmd":"set_led","current":mA}
//      {"cmd":"set_samples","n":N} / {"cmd":"measure"}
//   <- {"evt":"info",...} / {"evt":"ack",...} / {"evt":"progress",...}
//      {"evt":"result",...} / {"evt":"error",...}

let port, reader, writer;
let channels = [];
let sampleNumber = 0;

const el = (id) => document.getElementById(id);

const btnConnect   = el('btn-connect');
const statusLine   = el('status-line');
const setupSection = el('setup');
const gainSelect   = el('gain');
const ledSlider    = el('led');
const ledValue     = el('led-value');
const samplesSlider = el('samples');
const samplesValue  = el('samples-value');
const btnSendSetup = el('btn-send-setup');
const btnMeasure   = el('btn-measure');
const resultsGrid  = el('results-grid');
const sampleNumberLine = el('sample-number');

btnConnect.addEventListener('click', connect);
btnSendSetup.addEventListener('click', sendSetup);
btnMeasure.addEventListener('click', () => sendCommand({ cmd: 'measure' }));

ledSlider.addEventListener('input', () => { ledValue.textContent = ledSlider.value; });
samplesSlider.addEventListener('input', () => { samplesValue.textContent = samplesSlider.value; });

async function connect() {
  if (!('serial' in navigator)) {
    statusLine.textContent = 'Web Serial not supported in this browser (use Chrome or Edge).';
    return;
  }
  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });
    writer = port.writable.getWriter();
    statusLine.textContent = 'Connected — requesting sensor info...';
    readLoop();
    await sendCommand({ cmd: 'get_info' });
  } catch (e) {
    statusLine.textContent = 'Connection failed: ' + e.message;
  }
}

async function sendCommand(obj) {
  if (!writer) return;
  const line = JSON.stringify(obj) + '\n';
  await writer.write(new TextEncoder().encode(line));
}

async function readLoop() {
  const decoder = new TextDecoderStream();
  port.readable.pipeTo(decoder.writable);
  reader = decoder.readable.getReader();
  let buffer = '';
  while (true) {
    const { value, done } = await reader.read();
    if (done) break;
    buffer += value;
    let idx;
    while ((idx = buffer.indexOf('\n')) >= 0) {
      const line = buffer.slice(0, idx).trim();
      buffer = buffer.slice(idx + 1);
      if (line) handleLine(line);
    }
  }
}

function handleLine(line) {
  let msg;
  try { msg = JSON.parse(line); } catch { return; }

  if (msg.evt === 'info') {
    channels = msg.channels;
    buildGainOptions(msg.gain.options, msg.gain.default);
    ledSlider.min = msg.led.minMA;
    ledSlider.max = msg.led.maxMA;
    ledSlider.value = msg.led.default;
    ledValue.textContent = msg.led.default;
    buildResultCells();
    statusLine.textContent = 'Connected — ' + msg.sensor;
    setupSection.classList.remove('hidden');

  } else if (msg.evt === 'result') {
    sampleNumber++;
    sampleNumberLine.textContent = 'Sample number: ' + sampleNumber;
    updateResults(msg.data);

  } else if (msg.evt === 'error') {
    statusLine.textContent = 'Error: ' + msg.msg;
  }
}

function buildGainOptions(options, defaultIdx) {
  gainSelect.innerHTML = '';
  options.forEach((label, i) => {
    const opt = document.createElement('option');
    opt.value = i;
    opt.textContent = label;
    if (i === defaultIdx) opt.selected = true;
    gainSelect.appendChild(opt);
  });
}

function buildResultCells() {
  resultsGrid.innerHTML = '';
  channels.forEach((ch) => {
    const cell = document.createElement('div');
    cell.className = 'result-cell';
    cell.id = 'result-' + ch.id;
    cell.textContent = ch.name + ': 0.00';
    resultsGrid.appendChild(cell);
  });
}

function updateResults(data) {
  channels.forEach((ch) => {
    const cell = el('result-' + ch.id);
    const v = data[ch.id];
    if (cell && v !== undefined) cell.textContent = ch.name + ': ' + Number(v).toFixed(2);
  });
}

async function sendSetup() {
  const gainIdx = Number(gainSelect.value);
  const ledMA = Number(ledSlider.value);
  const n = Number(samplesSlider.value);

  await sendCommand({ cmd: 'set_gain', idx: gainIdx });
  await sendCommand({ cmd: 'set_led', current: ledMA });
  await sendCommand({ cmd: 'set_samples', n });

  el('cfg-samples').textContent = 'Number of samples: ' + n;
  el('cfg-type').textContent = 'Sample type: ' + el('sample-type').selectedOptions[0].textContent;
  el('cfg-gain').textContent = 'Gain: ' + gainSelect.selectedOptions[0].textContent.toLowerCase();
  el('cfg-led').textContent = 'LED current: ' + ledMA + ' mA';
}
