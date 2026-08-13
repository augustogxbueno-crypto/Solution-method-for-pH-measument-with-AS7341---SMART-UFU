# SmartBioPH

A portable, miniaturized, and low-cost platform for colorimetric pH
determination using the **AS7341** multi-channel optical sensor and the
acid–base indicator **bromothymol blue (BTB)**, with cuvette-based
(solution) readout and 3D-printable parts for the prototype.

This repository accompanies the paper:

> *From Solution to Paper: Balancing Analytical Precision and
> Point-of-Care Simplicity in Salivary pH Colorimetry* — A. G. Xavier,
> C. C. S. Machado, Y. S. N. da Mota, J. F. S. Petruci, S. G. da Silva.

## Repository structure

```
SmartBioPH/
├── firmware/              PlatformIO firmware (Arduino Uno / ESP32-S3)
│   ├── platformio.ini      build environments + dependencies (lib_deps)
│   └── src/
│       ├── main.cpp         setup/loop, serial line reading
│       ├── core/
│       │   ├── Utils.h        PROGMEM helpers (no heap/String usage)
│       │   ├── SensorManager.h  active-sensor factory (fixed: AS7341)
│       │   ├── Measurement.h/.cpp  sampling and 8-channel averaging
│       │   └── Protocol.h/.cpp    JSON protocol over Serial
│       └── sensors/
│           ├── SensorBase.h     abstract sensor interface
│           └── AS7341.h/.cpp    AS7341 driver (reflectance mode)
├── web/                    web UI (Web Serial API)
│   ├── index.html
│   ├── script.js
│   └── style.css
└── hardware/               3D-printable parts
    ├── SmartpH_Cuvette.stl        reduced-volume cuvette (40 mm)
    ├── SmartpH_Holder.stl         AS7341 sensor holder
    ├── Arduino_UNO_Smart_Box.3mf  Arduino Uno case (full project)
    └── Arduino_UNO_Smart_Box_part_2.stl  second case part
```

## Firmware architecture

The firmware follows a simple three-layer separation:

1. **`sensors/`** — implements `SensorBase`, the minimal interface any
   sensor needs to expose (channel readout, gain, LED). Currently only
   the `AS7341Sensor` driver exists, operating **in reflectance mode
   only** (internal LED on, no absorbance/fluorescence modes and no
   external LEDs).
2. **`core/`** — sensor-independent logic:
   - `SensorManager` returns the active sensor instance (fixed to the
     AS7341, no build-flag selection);
   - `Measurement` takes N readings and averages the 8 channels;
   - `Protocol` is the only file that knows about JSON — it handles
     incoming serial commands and builds the responses;
   - `Utils` provides `PROGMEM` string helpers, avoiding the `String`
     class (heap fragmentation on AVR boards).
3. **`main.cpp`** — brings up Serial/I2C/the sensor and parses the
   incoming serial stream line by line, handing each complete line to
   `Protocol::handleCommand`.

The web UI (`web/`) talks to the board via the **Web Serial API**
(Chrome/Edge), using the JSON protocol described below — it has no
dependency on the Arduino IDE or any backend.

### Protocol (line-delimited JSON over Serial)

**Browser → Board**
```json
{"cmd":"get_info"}
{"cmd":"set_gain","idx":4}
{"cmd":"set_led","current":60}
{"cmd":"set_samples","n":1}
{"cmd":"measure"}
```

**Board → Browser**
```json
{"evt":"info","sensor":"AS7341","channels":[...8...],"gain":{"options":[...],"default":4},"led":{"minMA":4,"maxMA":258,"default":60}}
{"evt":"ack","cmd":"..."}
{"evt":"progress","n":1,"data":{"0":123.4,...}}
{"evt":"result","n":1,"gain":"16X","led_ma":60,"data":{"0":123.4,...}}
{"evt":"error","msg":"..."}
```

## Hardware

- Microcontroller: **Arduino Uno** (tested, ~91% flash / ~40% RAM) or
  **ESP32-S3** (native USB).
- Sensor: **AS7341** (Adafruit breakout), 8 spectral channels (415–680 nm).
- Reduced-volume acrylic cuvette (40 mm height, 900 µL) —
  `hardware/SmartpH_Cuvette.stl`.
- 3D-printed holder to fix sensor/cuvette geometry —
  `hardware/SmartpH_Holder.stl`.
- Arduino Uno case — `hardware/Arduino_UNO_Smart_Box.3mf`
  (full editable project) plus the complementary part
  `Arduino_UNO_Smart_Box_part_2.stl`.

The `.stl` files can be opened in any slicer (Cura, PrusaSlicer, etc.);
the `.3mf` preserves the complete project (multiple parts/placement) and
opens directly in your slicer or the originating CAD software.

## Installing PlatformIO

The firmware uses **PlatformIO**, which automatically downloads and
resolves all libraries (`Adafruit_AS7341`, `Adafruit_BusIO`,
`ArduinoJson`) listed in `firmware/platformio.ini` — no manual library
installation is needed.

**Recommended — VS Code:**
1. Install [VS Code](https://code.visualstudio.com/).
2. Install the **PlatformIO IDE** extension (Extensions tab, search for
   "PlatformIO IDE").
3. Open the `firmware/` folder from this repository via
   *File → Open Folder*. PlatformIO detects `platformio.ini` and
   downloads the dependencies automatically the first time you build.

**Command-line option:**
```bash
# install PlatformIO Core (requires Python 3)
pip install -U platformio

# inside the firmware/ folder
cd firmware
pio run                      # builds both environments and fetches the libs
pio run -e uno -t upload     # build and flash to Arduino Uno
pio run -e as7341_esp32s3 -t upload   # build and flash to ESP32-S3
```

No manual library-installation step is required: PlatformIO reads
`lib_deps` in `platformio.ini` and resolves everything on its own,
whether from VS Code or the CLI.

## Usage

1. Connect the AS7341 to the board via I2C (SDA/SCL) and plug the board
   into the PC over USB.
2. Flash the firmware (`pio run -e uno -t upload` or `-e as7341_esp32s3`).
3. Open `web/index.html` in **Chrome or Edge** (the Web Serial API does
   not work in Firefox/Safari).
4. Click **Connect via USB** and select the board's port.
5. Adjust Gain, LED current, and Number of samples, then click
   **Send setup**.
6. Click **Measure** to trigger a reading and view the results for all
   8 spectral channels.

## License

This project is released under the MIT License — see [LICENSE](LICENSE).
If you'd prefer a different license (e.g. GPL, Apache-2.0) for the
paper's repository, just swap the file.

## Citation

If this code or the parts are useful in your work, please cite the
corresponding paper (full citation details to be updated upon
publication).
