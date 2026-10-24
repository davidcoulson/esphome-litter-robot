# Litter Robot 3 - Mainboard UART Protocol Reference

## 1. Architecture

The LR3 Connect mainboard contains two microcontrollers:

- **ESP32** (ESP32-WROOM-32, "ESP") - a thin WiFi/MQTT-to-UART bridge. It contains no Litter Robot hardware logic; it relays commands and telemetry between the cloud (AWS IoT / MQTT) and the mainboard.

- **Mainboard MCU** (PIC18F67K40-I/PT, TQFP-64 package, "PIC") - runs all robot logic: motor control, sensor processing, the cycle state machine, RTC, sleep scheduling, and all safety interlocks (anti-pinch, weight-pause, over-current). The PIC operates independently of the ESP32 and can cut power to the ESP32 completely for power saving on battery.

> Two variants of the LR3 Connect PCB have been identified. They appear functionally equivalent.
>
> - v1: `LR3-2017-COMBO` with a `3/8/19 LMR` top silkscreen marking.
> - v2: newer revision with only the `automated pet care` marking on the top silkscreen.

## 2. Physical Layer

- UART1, 19200 baud, 8n1, no flow control.
- ESP32 pins: TX = GPIO19, RX = GPIO5.
- PIC pins: TODO.
- Programming header: `J8` `ESP PRG`, 2x3 1.27mm pads next to the ESP32.

## 3. Serial Frame Format

All frames are ASCII lines terminated by `\r\n`.

| Direction  | Prefix | Purpose                      |
|------------|--------|------------------------------|
| PIC -> ESP | `)`    | Status and non-status events |
| PIC -> ESP | `(`    | Command responses            |
| ESP -> PIC | `)`    | Commands                     |

The PIC sends a status packet once per second. The packet is 20 bytes when sleep mode is off and 28 bytes when the sleep countdown is present.

The PIC also sends `)F0`/`)F1` when the WiFi provisioning state changes, `)Y####` at the midpoint of a clean cycle, and a `({cmd}AOK`/`({cmd}NOK#` response to each command.

## 4. Cloud Command Format (for reference)

Observed in stock firmware logs: the ESP32 receives AWS IoT/MQTT commands prefixed with `<`, then forwards them to the PIC as `)` command frames with the same command character and value. For example, `<C` becomes `)C\r\n`.

## 5. Commands

### 5.1 Command Format

`){cmd}{optional value}\r\n`

| Field      | Description                           |
|------------|---------------------------------------|
| Prefix     | `)`                                   |
| Command    | Single character from the table below |
| Value      | Optional; depends on the command      |
| Terminator | `\r\n`                                |

### 5.2 Command Table

| Command | Action                         | Value                                                             |
|---------|--------------------------------|-------------------------------------------------------------------|
| `C`     | Start clean cycle              | none                                                              |
| `D`     | Factory reset                  | none                                                              |
| `F`     | WiFi provisioning state        | `0`=not provisioning, `1`=provisioning                            |
| `L`     | Panel lockout                  | `0`=off, `1`=on                                                   |
| `N`     | Night light                    | `0`=off, `1`=on                                                   |
| `P`     | Power                          | `0`=off, `1`=on                                                   |
| `R`     | Reset (simulated button press) | none                                                              |
| `S`     | Sleep mode                     | `0`=off; `1hh:mm:ss`=on, current position relative to sleep start |
| `W`     | Cycle delay                    | `3`=3 min, `7`=7 min, `F`=15 min                                  |

- The PIC sends `)F0`/`)F1` when the WiFi provisioning state changes. The ESP echoes the same frame; the PIC acknowledges with `(FAOK`.
- For `S`, the offset is the current position relative to the sleep start, not a future start time. `00:00:00` starts the sleep window and `08:00:00` ends it; the robot is awake for the rest of the 24-hour cycle. To start sleeping later, send `)S100:00:00` at the desired start time.
- For `W`, values `4`-`E` were tested and rejected by the PIC.
- `R` for the most part acts the same as a physical press of the reset button, with the exception that a press will not stop/cancel a cycle.

### 5.3 Responses

| Response     | Meaning                                |
|--------------|----------------------------------------|
| `({cmd}AOK`  | Command accepted                       |
| `({cmd}NOK#` | Command rejected; `#` is a reason code |

### 5.4 Non-Status Events

| Event    | Meaning                                                                         |
|----------|---------------------------------------------------------------------------------|
| `)F0`    | WiFi provisioning disabled; the ESP echoes `)F0` and the power LED is blue      |
| `)F1`    | WiFi provisioning enabled; the ESP echoes `)F1` and the power LED is turned off |
| `)Y####` | Total clean cycle count, 4 hex digits, sent at the midpoint of a clean cycle    |

Total clean cycle count persists across a factory reset.

## 6. Status Packet

### 6.1 Packet Layout

The packet is a space-delimited ASCII string with 7 fields. Only the sleep field changes width (8 extra characters when sleep is on), giving 20 or 28 bytes.

| Field | Layout                         | Description                                      |
|-------|--------------------------------|--------------------------------------------------|
| 1     | `)` + 1 digit                  | Power supply: `0`=AC, `1`=battery                |
| 2     | 2 digits                       | First digit = power state, second = system state |
| 3     | 1 character                    | Wait time                                        |
| 4     | 1 character                    | Night light                                      |
| 5     | 1 digit, plus `hh:mm:ss` if on | Sleep mode                                       |
| 6     | 1 character                    | Panel lockout                                    |
| 7     | 1 digit + 3 hex characters     | Unknown flag + cat sensor value                  |

### 6.2 Field Details

#### Power State

`0` = normal; `1` = standby/off.

#### System State

| Value | Meaning                       |
|-------|-------------------------------|
| `0`   | Calibrating or provisioning   |
| `1`   | Ready                         |
| `2`   | Timing                        |
| `3`   | Cycling                       |
| `4`   | Unknown (Mid cycle)           |
| `5`   | Cycle interupted              |
| `6`   | Cycle paused                  |
| `7`   | Bonnet removed                |
| `8`   | Pinch sensor                  |
| `9`   | Waste drawer full             |
| `A`   | Waste drawer full (+1 cycle)  |
| `B`   | Waste drawer full (+2 cycles) |

After `9`, the state increments once per clean cycle while the drawer remains full up to B, where cycles are prevented from happening.

#### Wait Time

| Value | Meaning    |
|-------|------------|
| `3`   | 3 minutes  |
| `7`   | 7 minutes  |
| `F`   | 15 minutes |

#### Night Light

Boolean; Turns the globe LED on and off.

#### Sleep Mode

`0` = off. `1hh:mm:ss` = on, where `hh:mm:ss` is the current position relative to the sleep start.

The robot sleeps for the first 8 hours of a 24-hour cycle: `00:00:00` is the start of sleep and `08:00:00` is the end. After `08:00:00` the robot is in its normal awake period for the rest of the cycle.

For example, a value of `1` + `00:25:38` (written as `100:25:38`) means sleep is enabled and the robot is 25m38s into the sleep window.

#### Panel Lockout

Boolean; keypad inputs will be ignored.

#### Cat Sensor

The first digit is an unknown flag; the last 3 hex characters are the cat sensor value.

| First digit | Meaning                                     |
|-------------|---------------------------------------------|
| `0`         | Normal                                      |
| `1`         | Occurs for one packet about every 5 minutes |
| `3`         | Bonnet removed                              |

Cat sensor value: raw 3-hex load/weight reading. Observed values scale with weight; units are unconfirmed. The documented maximum is `0x3FF`.

## 7. Notes

- Panel lockout is enabled for 5 seconds after a clean cycle completes when it was disabled at the start.
