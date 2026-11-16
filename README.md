<div align="center">

# Litter Robot ESPHome Packages (dc fork)

Local-only ESPHome firmware for the Whisker Litter Robot 3 and 4, with Home Assistant integration and no Whisker cloud.

**This is a fork** of [Joseph DiGiovanni's esphome-litter-robot](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot). All the hardware work, the UART protocol reverse engineering and the base firmware are his; please support the [upstream project](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot). This fork tracks upstream and adds the changes listed in [What this fork adds](#what-this-fork-adds). Flashing guides and the UART reference are upstream's, with a few safety additions.

</div>

## What this fork adds

### Cat identification by BLE collar tag

Weight alone cannot tell cats apart when they all weigh about the same. This fork identifies the cat by the Bluetooth tag on its collar instead:

- While a cat is in the robot, the ESP32 keeps the strongest RSSI it hears from each known tag (`esp32_ble_tracker` `on_ble_advertise`).
- When the robot reports the visit weight, the strongest tag wins if it is above a minimum RSSI and beats the runner-up by a margin (both tunable). That cat's visit is counted and its weight updated.
- If no tag is clear enough, the visit is logged as **Unknown**. There is no weight fallback, so a wrong guess never pollutes a cat's history.
- A reading that is far from the cat's learned weight (for example a partial reading) is counted as a visit but does not change the stored weight.
- Each cat shows up as its own Home Assistant sub-device, with weight, visits, waste counts, a Set Weight number and a Tracker Reset button. Up to six cats.
- Combined with WasteSense, feces and urine counts are attributed to the cat that was just identified.

Tag addresses, thresholds and cat names live in your device config, not in this repository, because they are specific to your home. The `cat_tracker` component here provides the per-cat entities and visit recording; the package no longer includes upstream's weight-only `cat-tracker/default.yaml` (cats 1-4), so you declare `cat_tracker` and its cats yourself. See `packages/cat-tracker/` for the pieces to build on.

### Litter Robot 4 fixes and behaviour changes

- **Cycles work under Control Panel Lockout.** Cancel and start cycle commands are no longer ignored while the panel is locked.
- **UART replies are matched to the command that was sent**, so a late or unrelated reply from the mainboard can no longer be taken as the answer to a different request.
- **Odor Control re-checks before cycling** and never leaves the Ready state hidden.
- **Sleep Override keeps your schedule.** Turning the override on remembers the weekly schedule and turning it off restores it.
- **No boot clock sync and no Cycle lockout workaround**; both caused more trouble than they solved.

### Safer install and operation

- The flashing guide has you **verify the stock firmware backup before erasing**, and has clearer wiring steps.
- A new **Securing the Device** section in the [LR4 User Guide](docs/Litter%20Robot%204/User%20Guide.md) covers API encryption, an OTA password and replacing the default web and hotspot passwords.

### Tested with

ESPHome 2026.10 and 2026.11 (compile-checked on 2026.11.0b5 for LR3 and for LR4 with WasteSense).
