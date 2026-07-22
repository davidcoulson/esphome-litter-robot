# Litter Robot ESPHome Packages

This repository serves as a home for ESPHome packages relating to the Whisker Litter Robot, as well as instructions to flash the custom ESPHome firmware.

![Home Assistant](docs/Litter%20Robot%204/Images/Home%20Assistant.JPG)

## What devices are supported?
Only the Litter Robot 4 is supported at this time. Though I suspect the Litter Robot 3 is very similar, I do not have one to test on. Contributions are appreciated!

## Is it safe?

The ESP32 in the Litter Robot 3 and 4 serves as a WiFi-to-UART bridge that relays commands between the cloud and the mainboard PIC microcontroller. All hardware control and safety-critical logic comes from this separate microcontroller, so replacing the ESP32 firmware should not compromise safety mechanisms of the device in any way.

## What features are supported?

Every known function of the original firmware is supported, with these exceptions:

- Not all faults are descriptively named, though generic fault status is still detected.
- Litter hopper configuration is not supported yet, but the hopper itself may still function.
- Updating mainboard/laserboard firmware.

## Is it reversible?

The original firmware can be flashed back to the ESP32 to fully restore the device to its original state. You should make a full backup of the stock firmware prior to flashing the ESPHome firmware.

## How do I install it?

The initial firmware flashing requires connecting an [ESP-Prog-2 development board](https://amzn.to/4vMhlyv) (Affiliate Link) to an unpopulated ESP-Prog header on the mainboard. Since the pads are unpopulated you will either need to solder directly to them or populate it with the intended [1.27mm surface mount header](https://www.ebay.com/itm/336595048935) (Sold pre-cut to size by me) to utilize the ribbon cable included with the ESP-Prog-2. Full instructions can be found in the [Firmware Flashing Guide](docs/Litter%20Robot%204/Flashing%20Instructions.md).

## Docs

See the [Litter Robot 4 UART Reference](docs/Litter%20Robot%204/UART%20Reference.md) for a full breakdown of the mainboard serial protocol.
