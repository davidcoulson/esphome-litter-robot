<div align="center">

# 🐈 Litter Robot ESPHome Packages (dc fork)

Local-only ESPHome firmware for the Whisker Litter Robot 3 and 4, with Home Assistant integration and no Whisker cloud.

🍴 **This is a fork** of [Joseph DiGiovanni's esphome-litter-robot](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot). All the hardware work, the UART protocol reverse engineering and the base firmware are his; please support the [upstream project](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot). This fork tracks upstream and adds the changes listed in [What this fork adds](#what-this-fork-adds). Flashing guides and the UART reference are upstream's, with a few safety additions.

</div>

## ✨ What this fork adds

### 🏷️ Cat identification by BLE collar tag

Weight alone cannot tell cats apart when they all weigh about the same. This fork identifies the cat by the Bluetooth tag on its collar instead:

- While a cat is in the robot, the ESP32 keeps the strongest RSSI it hears from each known tag (`esp32_ble_tracker` `on_ble_advertise`).
- When the robot reports the visit weight, the strongest tag wins if it is above a minimum RSSI and beats the runner-up by a margin (both tunable). That cat's visit is counted and its weight updated.
- If no tag is clear enough, the visit is logged as **Unknown** 🤷. There is no weight fallback, so a wrong guess never pollutes a cat's history.
- A reading that is far from the cat's learned weight (for example a partial reading) is counted as a visit but does not change the stored weight.
- Each cat shows up as its own Home Assistant sub-device, with weight, visits, waste counts, a Set Weight number and a Tracker Reset button. Up to six cats.
- With more than one robot, the optional Clowder package (below) sends every visit to the [Clowder](https://github.com/davidcoulson/ha-clowder) Home Assistant integration, so each cat is one device across all your robots instead of a copy on each.
- Combined with WasteSense, poop 💩 and pee 💧 counts are attributed to the cat that was just identified.

Tag addresses, thresholds and cat names live in your device config, not in this repository, because they are specific to your home. The `cat_tracker` component here provides the per-cat entities and visit recording; the package no longer includes upstream's weight-only `cat-tracker/default.yaml` (cats 1-4), so you declare `cat_tracker` and its cats yourself. See `packages/cat-tracker/` for the pieces to build on.

With the tag in hand, the tracker also measures how long each visit lasted (last and 7-day average) and can raise an **In Box Too Long** alert.

### 🔧 Litter Robot 4 fixes and behaviour changes

These change how the standard LR4 firmware behaves, with no extra package needed:

- **Cycles work under Control Panel Lockout.** Cancel and start cycle commands are no longer ignored while the panel is locked.
- **UART replies are matched to the command that was sent**, so a late or unrelated reply from the mainboard can no longer be taken as the answer to a different request.
- **Timed-out writes are retried** (twice), so a lost lockout relock no longer leaves the panel unlocked. Keypad presses are never retried, since a second Cycle press pauses the robot.
- **Odor Control re-checks before cycling** and never leaves the Ready state hidden.
- **Unknown status codes show as `Unknown (0xNN)`** instead of leaving the last known status (often a stale "Ready") on screen.
- **Curtain and weight sensors start as unknown after a reboot**, rather than "clear", so a reboot with a cat inside is not read as an empty globe. Their first report after boot still runs every package's triggers.
- **Sleep schedule fixes.** Sleep Override remembers the weekly schedule and restores it when turned off. A day toggled before the robot's schedule has been read is ignored instead of clearing the other days. The time zone is a `timezone` substitution.
- **No boot clock sync and no Cycle lockout workaround**; both caused more trouble than they solved.

### 📦 Optional packages

Each one is off unless you include it in your device config. Most are untested beyond the author's robots; the linked docs say what is experimental.

| Package | Robot | What it adds |
|---|---|---|
| `packages/cat-tracker/health.yaml` | any | Per-cat 7-day visits per day and average weight, hours since the last visit, and "changed" alerts for weight, visits and no visit. An alert means something changed, not that the cat is ill. |
| `packages/litter-robot4/maintenance.yaml` | LR4 | Drawer fill rate and when it will be full, filter due (by cycles), Litter Low, and one **Needs Attention** summary. |
| `packages/litter-robot3/maintenance.yaml` | LR3 | The same forecast, learned from the cycle count at which the drawer last reached full. |
| `packages/cat-tracker/waste-tracking/waste-mass.yaml` | LR4 | Estimated waste per visit from the globe weight before and after (needs Debug Mode). Diagnostic only. See [Waste mass](docs/Litter%20Robot%204/WasteSense%20Install.md#waste-mass-experimental). |
| `packages/litter-robot4/odor-watch.yaml` | LR4 + WasteSense | Smoothed Odor Level, an opt-in extra Smell Cycle when the smell lingers, and **Change Litter Soon**. See [Odor watch](docs/Litter%20Robot%204/WasteSense%20Install.md#odor-watch-experimental). |
| `packages/litter-robot4/clowder.yaml` | LR4 + WasteSense | Sends every visit to the [Clowder](https://github.com/davidcoulson/ha-clowder) Home Assistant integration, which keeps one device per cat across all your robots. Visits wait in flash until Home Assistant confirms them, so none are lost while it is down. Your config reports each identified visit. See [Clowder](docs/Litter%20Robot%204/Clowder.md). |
| `homeassistant/cat-sync/` | LR3 | **Replaced by Clowder on LR4 robots.** Home Assistant package and blueprint (nothing to flash) that keeps each cat's weight the same on every robot and sums visit, poop and pee counts, using each robot's own per-cat entities. See its [README](homeassistant/cat-sync/README.md). |

### 🔌 Hardware add-ons

Both share the BME688's I2C bus (GPIO13/14 via J5, 3.3 V and GND from J7), so they need no extra mainboard wiring.

| Sensor | Package | What it adds |
|---|---|---|
| Sensirion **SGP41** (0x59) | `packages/litter-robot4/voc-nox.yaml` | VOC Index and NOx Index, compensated with the BME688's temperature and humidity. Daisy-chains off the BME688 with a STEMMA QT / Qwiic cable. [Wiring and photos](docs/Litter%20Robot%204/WasteSense%20Install.md#voc-and-nox-index-optional-sgp41). |
| SGP41 + WasteSense | `packages/litter-robot4/voc-compare.yaml` | Data collection only: per visit, publishes the WasteSense verdict next to how far the SGP41 and BME688 VOC readings rose, to judge whether the SGP41 could confirm or correct the verdict. |
| **VL53L1X** time-of-flight (e.g. TOF400C) | `packages/litter-robot4/litter-depth.yaml` | Litter Distance and litter depth, after a two-point calibration. Untested on a real robot. [Details](docs/Litter%20Robot%204/WasteSense%20Install.md#litter-depth-experimental). |

### 🔒 Safer install and operation

- The flashing guide has you **verify the stock firmware backup before erasing**, and has clearer wiring steps.
- A new **Securing the Device** section in the [LR4 User Guide](docs/Litter%20Robot%204/User%20Guide.md) covers API encryption, an OTA password and replacing the default web and hotspot passwords.

### ✅ Tested with

ESPHome 2026.10 and 2026.11 (compile-checked on 2026.11.0b5 for LR3 and for LR4 with WasteSense).
