<div align="center">

# Litter Robot ESPHome Packages

This repository hosts ESPHome packages relating to the Whisker Litter Robot, as well as instructions to flash ESPHome-based firmware on it.

[![Join the Discord](https://img.shields.io/badge/Discord-5865f2?style=for-the-badge&logo=discord&logoColor=f5f5f5)](https://discord.gg/NrMdSAkPsx) [![Get $50 off Litter-Robot](https://img.shields.io/badge/Get%20$50%20off%20Litter--Robot-fbb040?style=for-the-badge&logo=pipecat&logoColor=black)](https://share.litter-robot.com/x/x2zGcr) [![Donate via PayPal](https://img.shields.io/badge/Donate%20via%20PayPal-002991?style=for-the-badge&logo=paypal&logoColor=008cff)](https://www.paypal.com/paypalme/jdigi78)

</div>

## Why?

Flashing the Litter Robot with ESPHome-based firmware eliminates the dependency on Whisker's cloud infrastructure, instead providing easy integration with open self-hosted services like Home Assistant. This means:

- **Total privacy** - all data stays local. No telemetry, usage statistics, or other data is ever sent to Whisker or AWS.
- **No internet required** - the device can connect to your smart home entirely offline, with no cloud outages or future service shutdowns to worry about.
- **Customizable** - Add or modify functionality however you want.

<div align="center">

![Home Assistant](docs/Litter%20Robot%204/Images/Home%20Assistant.JPG)

</div>

## Is it safe?

The ESP32 in the Litter Robot serves as a WiFi-to-UART bridge that relays commands between the cloud and the mainboard PIC/STM microcontroller. All hardware control and safety-critical logic comes from this separate microcontroller, so replacing the ESP32 firmware should not compromise safety mechanisms of the device in any way.

## What devices are supported?

Only the Litter Robot 4 is supported at this time. Litter Robot 3 and 5 support is coming soon.

## What features are supported?

Every function of the original firmware/app is supported, with minor exceptions:

- Some device faults are not yet detected or descriptively named. The unit will still respond to them and show a light pattern on the LED panel as usual.
- Updating firmware of other internal components is not possible.

> For serial numbers starting with "LR4C", stock firmware should be updated to 1175.5021.292 (July 2024) or later prior to install, otherwise the litter hopper accessory will not function.

There are actually quite a few improvements over the stock firmware/app.

**Major improvements include:**

- Smart Cat Detection - Fewer false cat detections and wasted clean cycles. Makes sure both weight _and_ curtain sensor detections happen before confirming a cat visit.
- WasteSense™ - Waste identification and odor control, similar to Whisker's "WasteID" found on the Litter Robot 5.
	- Requires soldering a sensor to the mainboard. [Instructions here](docs/Litter%20Robot%204/WasteSense%20Install.md).

**Minor improvements:**

- Litter hopper can be enabled/disabled via automations. (Not possible with existing Home Assistant integration)
- Sleep schedules can be set on a daily basis instead of only one time for weekday/weekend.
    - Sleep can be toggled with a manual "Sleep Override" switch for more advanced non-time-based events.
- More direct access to the curtain and weight detection sensors, for precise enter and exit tracking.

## Is it reversible?

The original firmware can be flashed back to the ESP32 to fully restore the device to its original state. You should make a full backup of the stock firmware prior to flashing the ESPHome firmware.

## How do I install it?

The initial firmware flashing requires connecting an [ESP-Prog-2 development board](https://www.mouser.com/en/ProductDetail/Espressif-Systems/ESP-Prog-2?qs=HMhDvBYWqvPF20t9F8LmIA%3D%3D) to an unpopulated header on the mainboard. Units manufactured after February 2026 (serial numbers starting with "LR4S") can be flashed without soldering, but older units will require soldering. Full instructions and required tools/parts can be found in the [Firmware Flashing Guide](docs/Litter%20Robot%204/Flashing%20Instructions.md).

Additionally, you can [add a BME680 sensor](docs/Litter%20Robot%204/WasteSense%20Install.md) to enable WasteSense™, an alternative to Whisker's "WasteID" found on the Litter Robot 5.

The source code for the component can be found in my [ESPHome fork](https://github.com/Joseph-DiGiovanni/esphome/tree/LR4-dev) until it is potentially merged into ESPHome.

## Where can I learn more?

Feel free to join the [Discord](https://discord.gg/NrMdSAkPsx) for assistance or discussion, before and after install!

See the [Litter Robot 4 User Guide](docs/Litter%20Robot%204/User%20Guide.md) for how to use the firmware's features.

See the [Litter Robot 4 UART Reference](docs/Litter%20Robot%204/UART%20Reference.md) for a full breakdown of the mainboard serial protocol.

## Can I buy you a coffee?

If you find this project useful, donations of any amount are greatly appreciated. Just know I'll most likely be passing it on to other open source projects anyway :)

[![Donate via PayPal](https://img.shields.io/badge/Donate%20via%20PayPal-002991?style=for-the-badge&logo=paypal&logoColor=008cff)](https://www.paypal.com/paypalme/jdigi78)

You can also get $50 off a Litter-Robot using my referral link:

[![Get $50 off Litter-Robot](https://img.shields.io/badge/Get%20$50%20off%20Litter--Robot-fbb040?style=for-the-badge&logo=pipecat&logoColor=black)](https://share.litter-robot.com/x/x2zGcr)

## Disclaimer

This project is not affiliated with, endorsed by, or associated with Whisker, Litter Robot, or Automated Pet Care Products. Modifying your device involves hardware disassembly and modification which may void your warranty and carries a risk of permanent damage. Proceed at your own risk. Neither the author(s) nor contributors are responsible for any damage to your device, property, or pets resulting from the use of this firmware or instructions.
