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

Every known function of the original firmware is supported, with these exceptions, sorted most to least likely to change:

- Not all faults are descriptively named, though generic fault status is still detected.
- Litter hopper configuration is not supported yet, but the hopper itself will likely still function.
- Multiple cats are not tracked, only the latest cat weight measurement is provided.
- Updating mainboard/laserboard firmware is not currently possible.

For some reason the official app/firmware only lets you set sleep schedules on a coarse weekday/weekend basis. This firmware actually allows for independent sleep schedules for every day of the week.

## Is it reversible?

The original firmware can be flashed back to the ESP32 to fully restore the device to its original state. You should make a full backup of the stock firmware prior to flashing the ESPHome firmware.

## How do I install it?

The initial firmware flashing requires connecting an [ESP-Prog-2 development board](https://amzn.to/4vMhlyv) (Affiliate Link) to an unpopulated ESP-Prog header on the mainboard. Since the pads are unpopulated you will either need to solder directly to them or populate it with the intended [1.27mm surface mount header](https://www.ebay.com/itm/336595048935) (Sold pre-cut to size by me) to utilize the ribbon cable included with the ESP-Prog-2. Full instructions can be found in the [Firmware Flashing Guide](docs/Litter%20Robot%204/Flashing%20Instructions.md).

## Docs

See the [Litter Robot 4 UART Reference](docs/Litter%20Robot%204/UART%20Reference.md) for a full breakdown of the mainboard serial protocol.

## Disclaimer

This project is not affiliated with, endorsed by, or associated with Whisker, Litter Robot, or Automated Pet Care Products. Modifying your device involves hardware disassembly and modification which may void your warranty and carries a risk of permanent damage. Proceed at your own risk. Neither the author(s) nor contributors are responsible for any damage to your device, property, or pets resulting from the use of this firmware or instructions.