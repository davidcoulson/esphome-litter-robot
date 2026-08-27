# Litter Robot ESPHome Packages

This repository hosts ESPHome packages relating to the Whisker Litter Robot, as well as instructions to flash ESPHome based firmware on it.

## Why?

Flashing the Litter Robot with ESPHome-based firmware eliminates the dependency on Whisker's cloud infrastructure, instead providing easy integration with open self-hosted services like Home Assistant. This means:

- **Total privacy** - all data stays local. No telemetry, usage statistics, or other data is ever sent to Whisker or AWS.
- **No internet required** - the device can connect to your smart home entirely offline, with no cloud outages or future service shutdowns to worry about.
- **Customizable** - Add or modify functionality however you want.

![Home Assistant](docs/Litter%20Robot%204/Images/Home%20Assistant.JPG)

## What devices are supported?

Only the Litter Robot 4 is supported at this time. Though I suspect the Litter Robot 3 is very similar, I do not have one to test on. Contributions are appreciated!

## Is it safe?

The ESP32 in the Litter Robot 3 and 4 serves as a WiFi-to-UART bridge that relays commands between the cloud and the mainboard PIC microcontroller. All hardware control and safety-critical logic comes from this separate microcontroller, so replacing the ESP32 firmware should not compromise safety mechanisms of the device in any way.

## What features are supported?

Every known function of the original firmware is supported, with these exceptions from most to least likely to change:

- Not all faults are descriptively named yet, though generic fault status is still detected.
- LitterHopper control and status are not implemented, but the hopper itself should still work.
- Updating mainboard/laserboard firmware is not possible.

There are actually some improvements compared to the official firmware/app:

- Sleep schedules can be set on a daily basis instead of only one time for weekday/weekend.
- More direct access to the laser and weight detection sensors, for precise enter and exit tracking.

## Is it reversible?

The original firmware can be flashed back to the ESP32 to fully restore the device to its original state. You should make a full backup of the stock firmware prior to flashing the ESPHome firmware.

## How do I install it?

The initial firmware flashing requires connecting an [ESP-Prog-2 development board](https://amzn.to/4vMhlyv) (Affiliate Link) to an unpopulated ESP-Prog header on the mainboard. Since the pads are unpopulated you will need to either solder the appropriate header or solder wires directly to the pads to facilitate connection of the ESP Prog 2. Full instructions and required tools/parts can be found in the [Firmware Flashing Guide](docs/Litter%20Robot%204/Flashing%20Instructions.md).


## Where can I learn more?

See the [Litter Robot 4 UART Reference](docs/Litter%20Robot%204/UART%20Reference.md) for a full breakdown of the mainboard serial protocol.

## Can I help?

This project can almost certainly be ported to the Litter Robot 3 with little effort, but I don't have one to try it myself. If you have a Litter Robot 3 "Connect" with WiFi it would be helpful to capture serial packets between the PIC and ESP32 to understand the LR3 UART protocol.

This [blog post](https://www.elttam.com/blog/re-of-lr3) goes into detail on some of the hardware on the LR3 mainboard and the first section will tell you how to get logs out of the ESP32. If the logs do not contain serial data then the physical TX/RX lines should be the middle 2 pads of the 4 labeled J8 on the mainboard.

If you can't decipher the serial data or write code yourself, just send me any captures you get and I'll do the rest!

## Can I buy you a coffee?

If you find this project useful, donations of any amount are greatly appreciated. Just know I'll most likely be passing it on to other open source projects anyway :)

[![PayPal](https://img.shields.io/badge/PayPal-003087?style=for-the-badge&logo=paypal)](https://www.paypal.com/paypalme/jdigi78)

You can also get $50 off a Litter-Robot using my referral link:

[![Whisker Referral](https://img.shields.io/badge/$50%20off-Litter--Robot-fbb040?style=for-the-badge)](https://share.litter-robot.com/x/x2zGcr)

## Disclaimer

This project is not affiliated with, endorsed by, or associated with Whisker, Litter Robot, or Automated Pet Care Products. Modifying your device involves hardware disassembly and modification which may void your warranty and carries a risk of permanent damage. Proceed at your own risk. Neither the author(s) nor contributors are responsible for any damage to your device, property, or pets resulting from the use of this firmware or instructions.
