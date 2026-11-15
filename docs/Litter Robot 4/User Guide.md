# Litter Robot 4 - User Guide

This guide covers provisioning the device and the features this firmware adds on top of the stock Litter Robot 4. Standard operation is mostly unchanged, so you can refer to the official manual and troubleshooting guides for anything not mentioned here.

> This guide assumes the ESPHome firmware is already flashed. If you have not done that yet, start with the [Firmware Flashing Guide](Flashing%20Instructions.md).

Feel free to join the [Discord server](https://discord.gg/NrMdSAkPsx) for assistance or discussion.

---

## Initial Setup

On first boot, the ESPHome firmware will start in **WiFi access point (AP) mode** indicated by a flashing yellow LED under the connect button. Follow these steps to connect it to WiFi:

1. Connect to the WiFi network `litter-robot4-XXXXXX` (password: `Wh1sk3rM30wM30w`).

2. A captive portal should appear. If it doesn't, navigate to `http://192.168.4.1` in a browser.

3. Select your WiFi network and enter its password. The Litter Robot will reboot and connect to your network. A solid blue LED under the connect button indicates it connected successfully.

## Adding to Home Assistant

The device should be auto-discovered by the ESPHome integration. If it doesn't appear, add it manually with the device's hostname or IP address.

> The hostname is usually the same name as the access point earlier. You can find the hostname or IP in your router device list.

All entities will now be available in Home Assistant. Many are disabled by default to keep the device page tidy; enable the ones you want via the "disabled entities" button under each section.

![Home Assistant](Images/Home%20Assistant.JPG)

---

## Smart Cat Detection

Enabled by default. This switch requires **both** the weight and curtain sensors to agree before a visit is confirmed. This greatly reduces false detections and wasted clean cycles. You may want to disable this feature if you have kittens that are very small, or another condition causing the sensors not to activate reliably.

## Auto Cycle

When turned **off**, the robot will not automatically start a clean cycle after a cat visit. Instead the firmware clears the pending cycle, so you can decide when to run a manual cycle. This is helpful for jumpy cats that may be frightened by the automatic cycle, or small kittens that may not be reliably detected by the device's safety mechanisms.

Hidden in Home Assistant by default, enable the entity to use it.

## Multi-Cat Tracking

The precompiled firmware tracks weight and visit counts for four cats. These instructions assume you are using this default layout. To start tracking your cat:

1. Enable the desired cat tracking entities for the numbered cat in Home Assistant under "disabled entities" for the various categories.
2. Rename the numbered cat's entities with the desired cat name.
3. To set a starting weight, enable and set the `Set Cat N Weight` number input to your best guess.

Be sure to enable the matching `Cat N Weight` sensor. While the number input holds the same value, it is not tracked long term by Home Assistant.

The Cat Tracker component matches each new weight reading to the nearest known cat weight and increments that cat's visit counter, replacing the input weight with a real measurement. There is a 0.6 pound tolerance by default to avoid obviously false readings. Use a cat's reset button (e.g. `Cat N Tracker Reset`) to clear that cat's stored weight and counters. The Litter Robot's `Factory Reset` does not clear the tracker data.

If you have WasteSense™ installed, each cat also gets `Feces Count` and `Urine Count` sensors.

### Syncing Across Multiple Robots

Each robot keeps its own tracker state, so with more than one unit the learned weights and counters will drift apart. Home Assistant helpers can act as a shared source of truth: an `input_number` per cat's weight, kept in sync with every robot's `Set Cat N Weight`, and a summed helper for the count sensors.

A specific implementation is beyond the scope of this guide - if you build a blueprint or similar, contributions are welcome!

## WasteSense™ and Odor Control

[WasteSense™](WasteSense%20Install.md) utilizes an optional BME688 sensor to classify waste as feces or urine, similar to Whisker's "WasteID". It requires flashing the **`litter-robot4-waste-sense.yaml`** firmware and soldering the sensor to the mainboard.

- **Odor Control** - when enabled, the robot cycles much sooner after a feces visit, rather than waiting for the full clean cycle delay.
- **WasteSense Timeout** - how long to keep watching for feces after the cat leaves before recording the visit as urine (0.5-5 minutes, default 2 minutes).
- **Odor Control Delay** - how long the cat must be gone before a feces cycle starts (0-10 minutes, default 1 minute). If feces is detected after this has passed, the robot cycles right away.
- **WasteSense Calibration Reset** - clears the sensor's saved calibration and restarts the device. Use it after replacing the BME688; see [Replacing the Sensor](WasteSense%20Install.md#replacing-the-sensor).

Feces is confirmed by the sensor, but urine is only assumed when no feces is detected within `WasteSense Timeout`. You will get false urine identifications if a cat enters without urinating. If feces visits are being recorded as urine because the odor rises slowly, increase `WasteSense Timeout`. If your cat regularly does both in one visit, increase `Odor Control Delay` so urine has some time to clump before cycling.

> The BME688 sensor needs a few minutes to warm up after power on before its readings stabilize. Allow a few minutes after boot before relying on `Odor Control`.

## Web Server

The firmware hosts a lightweight web server on the device's IP address for basic control and OTA updates. The default username is `user` and the password is `Wh1sk3rM30wM30w`. You can find the device IP in your router's device list.

## Adopting in ESPHome Device Builder

The configuration can be "adopted" into [ESPHome Device Builder](https://desktop.esphome.io/) for further configuration. It should automatically show up at the top of the dashboard in a "Discovered Devices" tab. The auto-generated config imports the `litter-robot4.yaml` from this repository as a base package, so you can layer your own customizations on top without losing upstream updates.

## Updating the Firmware (OTA)

Once ESPHome is installed, future firmware updates can be applied over the air without reopening the unit. This process is automated in the ESPHome Device Builder dashboard, but you can also update via the web server:

1. Download the latest OTA update file (`.ota`) from the latest [LR4 release](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot/releases?q=LR4%2F).

2. Open the device's web interface using the hostname or IP of the device in a browser. The default username is `user` and the password is `Wh1sk3rM30wM30w`.

3. At the bottom, use the "OTA Update" file selector to select the downloaded OTA update file and click update. Be sure to not use the `.bin` file or the update will fail.

To return the device to the original Whisker firmware, see [Restoring the Stock Firmware](Flashing%20Instructions.md#restoring-the-stock-firmware).
