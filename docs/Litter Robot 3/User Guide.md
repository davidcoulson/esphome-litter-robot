# Litter Robot 3 - User Guide

This guide covers provisioning the device and the features this firmware adds on top of the stock Litter Robot 3. Standard operation is mostly unchanged, so you can refer to the official manual and troubleshooting guides for anything not mentioned here.

> This guide assumes the ESPHome firmware is already flashed. If you have not done that yet, start with the [Firmware Flashing Guide](Flashing%20Instructions.md).

Feel free to join the [Discord server](https://discord.gg/NrMdSAkPsx) for assistance or discussion.

---

## Initial Setup

On first boot, the ESPHome firmware will start in **WiFi access point (AP) mode**. Follow these steps to connect it to WiFi:

1. Connect to the WiFi network `litter-robot3-XXXXXX` (password: `Wh1sk3rM30wM30w`).

2. A captive portal should appear. If it doesn't, navigate to `http://192.168.4.1` in a browser.

3. Select your WiFi network and enter its password. The Litter Robot will reboot and connect to your network. The access point will disappear once it connects.

## Adding to Home Assistant

The device should be auto-discovered by the ESPHome integration. If it doesn't appear, add it manually with the device's hostname or IP address.

> The hostname is usually the same name as the access point earlier. You can find the hostname or IP in your router device list.

All entities will now be available in Home Assistant. Many are disabled by default to keep the device page tidy; enable the ones you want via the "disabled entities" button under each section.

---

## Sleep

The **Sleep** switch puts the robot to sleep and keeps it asleep until you turn it off. Control it directly in Home Assistant or from automations.

While the switch is on, the firmware keeps refreshing the robot's sleep timer so it does not wake up after its normal sleep window.

> This is a manual control, not a schedule. The robot cannot be told to start sleeping later; turn the switch on when the sleep window should begin. For example, to sleep from 8pm to 6am, use a Home Assistant automation that turns the **Sleep** switch on at 8pm and off at 6am.

## Weight Detection

The **Weight Detected** sensor turns on when the cat sensor rises 5 or more above the current baseline. It clears and re-tares when the reading drops back below that threshold for 3 seconds, or when the robot reports Ready.

## Waste Drawer and Clean Cycles

The robot does not measure how full the waste drawer is, so the **Waste Drawer Cycles** sensor counts clean cycles since the last time the robot went from Waste drawer full to Ready, giving an estimate of how much waste has been added. The count is stored in flash and survives reboots. It is automatically reset when reset button is pressed after the robot indicates a full waste drawer. The **Reset Waste Drawer** button sets the estimate back to zero manually.

The **Waste Drawer Full** sensor mirrors the robot's drawer-full state.

The **Clean Cycles** sensor is the robot's lifetime clean-cycle total.

## Web Server

The firmware hosts a lightweight web server on the device's IP address for basic control and OTA updates. The default username is `user` and the password is `Wh1sk3rM30wM30w`. You can find the device IP in your router's device list.

## Adopting in ESPHome Device Builder

The configuration can be "adopted" into [ESPHome Device Builder](https://desktop.esphome.io/) for further configuration. It should automatically show up at the top of the dashboard in a "Discovered Devices" tab. The auto-generated config imports the `litter-robot3.yaml` from this repository as a base package, so you can layer your own customizations on top without losing upstream updates.

## Updating the Firmware (OTA)

Once ESPHome is installed, future firmware updates can be applied over the air without reopening the unit. This process is automated in the ESPHome Device Builder dashboard, but you can also update via the web server:

1. Download the latest OTA update file (`.ota`) from the latest [LR3 release](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot/releases?q=LR3%2F).

2. Open the device's web interface using the hostname or IP of the device in a browser. The default username is `user` and the password is `Wh1sk3rM30wM30w`.

3. At the bottom, use the "OTA Update" file selector to select the downloaded OTA update file and click update. Be sure not to use the `.bin` file or the update will fail.

To return the device to the original Whisker firmware, see [Restoring the Stock Firmware](Flashing%20Instructions.md#restoring-the-stock-firmware).
