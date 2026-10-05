# Litter Robot 4 - Mainboard UART Protocol Reference

## 1. Architecture

The LR4 mainboard contains two microcontrollers plus a satellite board:

- **ESP32** (ESP32-WROOM-32E, "ESP") - a thin WiFi/MQTT-to-UART bridge. It contains no Litter Robot hardware logic; it relays register reads/writes between the cloud (AWS IoT / MQTT) and the mainboard.

- **Mainboard MCU** (PIC18F47Q43-I/PT, "PIC") - runs all robot logic: motor control, sensor processing, the cycle state machine, RTC, sleep scheduling, and all safety interlocks (anti-pinch, weight-pause, over-current). This board survives independently of the ESP32 and can even cut power to the ESP32 completely for power saving on battery.

	> Units manufactured after February 2026 (serial number starting with "LR4S") have a different mainboard revision which uses an STM microcontroller instead.

- **Laserboard** - a satellite board handling the curtain (ToF) sensors for detecting cat presence as well as litter and waste drawer level.

## 2. Physical Layer

- UART1, 256000 baud, no flow control.
- ESP32 pins: TX = GPIO19, RX = GPIO5.

## 3. Serial Frame Format

The bus is full-duplex with 7 bytes per frame:

| Field      | Description                                                    |
|------------|----------------------------------------------------------------|
| Direction  | `0x02` = ESP→PIC, `0x01` = PIC→ESP                             |
| Operation  | `0x01`=read, `0x02`=write, `0x03`=read-reply, `0x04`=write-ack |
| Register   | register number, 1 byte                                        |
| Value high | Most significant byte of 16-bit big-endian value               |
| Value low  | Least significant byte of 16-bit big-endian value              |
| Checksum   | sum of bytes 0–4, mod 256                                      |
| Terminator | always `0xFF`                                                  |

The PIC periodically sends unsolicited event notifications (`0x01 0x02` writes) and the ESP acknowledges every one with a `0x02 0x04` echoing the register and value received. The PIC acknowledges writes in the same way, though the value it acknowledges is not guaranteed to be valid/saved. Some registers like those for version/identification have indexed values which are read using the low value byte as the index.

## 4. Cloud Command Format (for reference)

The AWS-side MQTT command payload is:
```json
{"serial": "<device-serial>", "data": ["0xPPRRVVVV"]}
```
where `PP`=op, `RR`=register, `VVVV`=16-bit value. The ESP32 prepends the `0x02` direction byte and computes the checksum before forwarding to the PIC over UART.

## 5. Commands

| Action            | Register | Value    |
|-------------------|----------|----------|
| Start clean cycle | `0x34`   | `0x000A` |
| Empty litter      | `0x34`   | `0x000B` |
| Replace filter    | `0x34`   | `0x000C` |
| Factory reset     | `0x2D`   | `0x0001` |

Register `0x34` doubles as the [Robot Status](#robot-status-0x34) telemetry register. The PIC does not ack the `Factory reset` command, though it writes the command back to the ESP as a new write event before rebooting. Register values are reset internally to the PIC and are not written over UART. The `Day-of-week` value is reset in this process as well, though the time otherwise remains unchanged, likely due to the other values being read directly by the RTC.

### 5.1 Simulated Control Panel Input

| Button  | Register | Value    |
|---------|----------|----------|
| Power   | `0x01`   | `0x0101` |
| Cycle   | `0x01`   | `0x0201` |
| Reset   | `0x01`   | `0x0401` |
| Empty   | `0x01`   | `0x0801` |
| Connect | `0x01`   | `0x1001` |

The PIC reports physical button presses by pushing a value to register `0x01`. The ESP can write the same value to simulate a press. This is actually how the Whisker app handles `Power` and `Reset` commands.

A low byte value of `0x02` is written instead of `0x01` to indicate a long press.

## 6. Configuration Registers

### Debug Mode 0x05

Boolean; when enabled, the PIC emits the debug telemetry registers `0x80`-`0x96` every second. Their values cannot be read manually, even with debug mode enabled.

#### Ambient Light 0x84

Light sensor reading, 0-100. Used to control the [Night Light 0x3B](#night-light-0x3b) and [Control Panel Brightness](#control-panel-led-brightness-0x0e).

#### Current Weight 0x89

Live scale reading in decipounds (divide by 10 for pounds), signed. When a negative value is detected, the scale automatically tares back to 0 after a few seconds.

#### Curtain Sensors 0x8B-0x8D

Time-of-flight readings, mirroring [0x58-0x5A](#curtain-sensors-0x58-0x5a).

### LitterHopper 0x0C

Boolean; Controls the optional LitterHopper automatic litter dispenser. See [LitterHopper Status](#litterhopper-status-0x0c) for the state values reported while it is enabled.

### WiFi State 0x33

Register `0x33` is used to read and write the current WiFi state.

| Value  | Meaning    | Notes               |
|--------|------------|---------------------|
| `0x00` | Off        | White LED           |
| `0x11` | Pairing    | Flashing yellow LED |
| `0x21` | Connecting | Flashing blue LED   |
| `0x22` | Connected  | Solid blue LED      |
| `0x23` | Error      | Flashing red LED    |

The PIC writes `0x11` when WiFi is enabled and `0x00` when disabled via the Connect button on the control panel.

### Date 0x0F-0x15

Value is **local time** spread across the following registers. The PIC's RTC drives the [sleep schedule](#sleep-schedule-0x1d-0x2b); keeping time in sync is required for schedule accuracy.

| Register | Meaning     | Notes          |
|----------|-------------|----------------|
| `0x0F`   | Day-of-week | 0=Sunday       |
| `0x10`   | Hour        | 0-23           |
| `0x11`   | Minute      |                |
| `0x12`   | Second      |                |
| `0x13`   | Day         |                |
| `0x14`   | Month       |                |
| `0x15`   | Year        | two-digit only |

### Clean Cycle Delay 0x16

Determines how long to wait before starting a clean cycle after a cat has no longer been detected. Value in minutes with a minimum of 3. Defaults to 7.

### Control Panel Lockout 0x17

State of the control panel lock. Panel inputs will be ignored, including [simulated ones](#5-1-simulated-control-panel-input).

### Night Light Options 0x18/0x19

Configures the night light inside the globe. See [Night Light 0x3B](#night-light-0x3b) for the current state reading.

| Register | Meaning                | Notes               |
|----------|------------------------|---------------------|
| `0x18`   | Night light mode       | 0=off, 1=on, 2=auto |
| `0x19`   | Night light brightness | 0-100               |

The Whisker app maps brightness low/med/high to 25/50/100, with high as default.

When Night light mode is set to `auto`, the light is driven by the [ambient light sensor](#ambient-light-0x84). Values below 20 turn on the light, and values above 70 turn it off.

### Control Panel LED Brightness 0x0E

Brightness level of the panel LED. Two independent values are packed into this register's high and low bytes. Low/medium/high is mapped to 25/15, 50/40, and 100/90 in the app. Factory resetting the PIC sets these values to 100/50, which is close enough I suppose.

The brightness is driven by the [ambient light sensor](#ambient-light-0x84). Values below 20 set the panel brightness to the low byte value, and values above 70 set it to the high byte value.

### Panel Sleep Time 0x1B

1260 by default. Appears to be minutes from midnight (21:00, 9pm). Unclear purpose. TODO: Need more info.

### Panel Wake Time 0x1C

360 by default. Appears to be minutes from midnight (6:00, 6am). Unclear purpose. TODO: Need more info.

### CAL Flag (Purpose unknown) 0x76

Register `0x76` is the first register read by the ESP on boot. Logs output "Wifi Init because CAL flag is 1" immediately after. TODO: Need more info.

## 7. Telemetry Registers

### Power Type 0x07

0 is AC power. Set to 1 when on battery before powering down the ESP.

### Cat Weight 0x09

Per-visit cat weight. Signed 16 bit integer in centipounds (divide by 100 for lb).

### Heartbeat 0x30

0x63 gets written to ESP on register 0x30 every 5 seconds. If the ESP does not acknowledge these events immediately the PIC makes the "Connect" LED flash white, indicating "WiFi chip is defective" according to Whisker. The stock Whisker firmware filters this register from logs.

### Unit Power Status 0x31

Identified in logs. TODO: Need more info.

### Sleep Status 0x32

Boolean value indicating the current time is within a [sleep schedule](#sleep-schedule-0x1d-0x2b) time period.

### Robot Status 0x34

| Value  | Meaning                         |
|--------|---------------------------------|
| `0x01` | Powered off                     |
| `0x02` | Powering on                     |
| `0x03` | Powering off                    |
| `0x04` | Ready                           |
| `0x05` | Bonnet removed                  |
| `0x06` | ?                               |
| `0x07` | Cat detected                    |
| `0x0A` | Clean cycle in progress         |
| `0x0B` | Emptying litter                 |
| `0x0C` | Motion detected in waste drawer |
| `0x0E` | Filter replacement mode         |
| `0x17` | Starting calibration            |
| `0x18` | Calibration in progress         |

`0x06` is always set just before `0x07`, so its meaning is unclear but related to cat detection.

### Globe Fault Status 0x35

Not all faults are well understood. Names used by the Whisker API are placeholders until a better meaning is confirmed.

| Value  | Meaning                   |
|--------|---------------------------|
| `0x00` | No fault                  |
| `0x01` | "FAULT_TIMEOUT"           |
| `0x02` | Globe motor disconnected  |
| `0x03` | Under-voltage / brown-out |
| `0x04` | Globe motor over-current  |
| `0x05` | Globe motor current spike |
| `0x06` | Pinch detected            |
| `0x07` | "FAULT_ALL_SENSORS"       |

### Detection Event 0x37

Potentially not just for detection events, but these are the only understood published values:

| Value  | Meaning           |
|--------|-------------------|
| `0x10` | Curtain clear     |
| `0x11` | Curtain detected  |
| `0x20` | Weight clear      |
| `0x21` | Weight detected   |

### Bonnet Sensor 0x3A

Boolean, inverted; 0 = OK, 1 = bonnet removed. Tracks the same state as status `0x05` in [Robot Status](#robot-status-0x34).

### Night Light 0x3B

Boolean; current state of the night light inside the globe. Controlled via the [night light mode](#night-light-options-0x18-0x19).

### USB Power 0x04

"isUSBPowerOn" according to logs. Only ever observed a value of 1.

### LitterHopper Status 0x0C

| Value      | Meaning             |
|------------|---------------------|
| `0x0020`   | Enabled/Idle        |
| `0x01XX`   | Motor start         |
| `0x10XX`   | Motor stopped (?)   |
| `0x20XX`   | Motor stopped (?)   |
| `0x0030`   | Fault/Not Connected |

`0x01XX` and `0x10XX`/`0x20XX` are published in order during the hopper motor run but their values and meaning are unclear. The high byte is the only consistent pattern.

With no sensors or means of communication with the Litter Robot, the Hopper not dispensing litter or being empty must be inferred from the [litter level reading](#litter-drawer-level) not recovering after a few cycles.

### Odometers

| Register | Meaning                  |
|----------|--------------------------|
| `0x3D`   | Power cycle count        |
| `0x3E`   | Clean cycle count        |
| `0x3F`   | Empty cycle count        |
| `0x40`   | Filter cycle count       |
| `0x42`   | Waste drawer cycle count |

### Litter / Drawer Level

| Register | Meaning                 | Notes                                    |
|----------|-------------------------|------------------------------------------|
| `0x43`   | Waste drawer percentage |                                          |
| `0x44`   | Waste drawer full       | Boolean                                  |
| `0x45`   | Cycles since full       | Not published consistently               |
| `0x47`   | Litter level            | Raw distance value. 480=empty, 435<=full |
| `0x69`   | Calibrated litter level | Raw distance value. Used by hopper logic |

Waste drawer full (`0x44`) is set to true after 3 consecutive readings of the waste drawer percentage (`0x43`) over 80%. Each subsequent cycle increments cycles since full (`0x45`). Once cycles since full reaches 3, the robot will refuse to detect cats or cycle manually until the reset button is pressed.

The reset command clears `0x44` and `0x42` when the robot is in the [Ready](#robot-status-0x34) state; if the drawer was not actually emptied they will return to their previous states.

### Curtain Sensors 0x58-0x5A

Three time-of-flight curtain sensors on the laser board, reported in millimeters. They are published alongside [detection events](#detection-event-0x37) as a cat enters or exits. Labeled based on position facing into the globe.

| Register | Sensor |
|----------|--------|
| `0x58`   | Left   |
| `0x59`   | Center |
| `0x5A`   | Right  |

### Sleep Schedule 0x1D-0x2B

Sleep/wake times are encoded in minutes-of-day, so 21:00 (9pm) would be 1260 for example. The schedule relies on the PIC's [date/time registers](#date-0x0f-0x15) having accurate local time.

| Register      | Meaning                                         |
|---------------|-------------------------------------------------|
| `0x1D`        | Day-enable bitmask: bit 0=Sunday ... 6=Saturday |
| `0x1E`/`0x1F` | Sunday sleep/wake time                          |
| `0x20`/`0x21` | Monday sleep/wake time                          |
| `0x22`/`0x23` | Tuesday sleep/wake time                         |
| `0x24`/`0x25` | Wednesday sleep/wake time                       |
| `0x26`/`0x27` | Thursday sleep/wake time                        |
| `0x28`/`0x29` | Friday sleep/wake time                          |
| `0x2A`/`0x2B` | Saturday sleep/wake time                        |

Recommended write order: `0x1D=0` (disable) -> write all time pairs -> `0x1D=<mask>` (enable), to avoid the schedule firing against a partially-written set of times.

### Panel Sleep Mode 0x1A

Identified in logs. Likely related to [Panel Sleep/Wake Times](#panel-sleep-time-0x1b) rather than the [cycle sleep schedule](#sleep-schedule-0x1d-0x2b). TODO: Needs testing.

### Version / Identification Registers

| Register    | Meaning                                      |
|-------------|----------------------------------------------|
| `0x79[0/1]` | Mainboard / laserboard revision ID           |
| `0x7A[0/1]` | Mainboard / laserboard device ID             |
| `0x7E[0-3]` | Laserboard hardware / BOM / suite / revision |
| `0x7F[0-3]` | Mainboard hardware / BOM / suite / revision  |
