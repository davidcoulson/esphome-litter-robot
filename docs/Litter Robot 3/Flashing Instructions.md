# Litter Robot 3 Connect - Firmware Flashing Guide

This guide covers replacing the stock ESP32 firmware on the Litter Robot 3 mainboard with custom ESPHome firmware. The process is fully reversible and you can restore the original firmware at any time. After ESPHome is on the device, future updates can be delivered wirelessly ([OTA](User%20Guide.md#updating-the-firmware-ota)).

Feel free to join the [Discord server](https://discord.gg/NrMdSAkPsx) for assistance or discussion, before and after install!

> Disclaimer: This project is not affiliated with, endorsed by, or associated with Whisker, Litter Robot, or Automated Pet Care Products. Modifying your device involves hardware disassembly and modification which may void your warranty and carries a risk of permanent damage. Proceed at your own risk. Neither the author(s) nor contributors are responsible for any damage to your device, property, or pets resulting from the use of this firmware or instructions.

## Mainboard Revision Notice

Only the Litter Robot 3 "Connect" model has an ESP32. Non-Connect units do not, so they cannot run this firmware. Connect units have a serial number starting with "LR3C". A [Connect Upgrade Kit](https://www.litter-robot.com/litter-robot-iii-connect-upgrade-kit.html) can be purchased officially from Whisker if needed.

Some earlier Connect units have the WiFi chip on a separate secondary board. This guide is only for units with the ESP32 on the main circuit board, though the backup and flashing steps should be the same once you figure out how to connect a programmer. Please provide photos/info if you successfully flash a model with this configuration.

---

## Recommended Hardware

> All Amazon links in this guide are affiliate links.

- [**ESP-Prog-2 development board**](https://amzn.to/4vMhlyv)

- **[Fine fiberglass brush](https://amzn.to/3TKakku)** or **razor blade** - Any sharp object to scratch off conformal coating

- **Soldering iron** (fine/curved tip) and solder

- [**Female to female jumper wire**](https://www.digikey.com/en/products/detail/adafruit-industries-llc/1950/6827084)

- **Phillips-head screwdriver**

- **USB-C cable**

## Software

- **esptool** - Install instructions can be found [here](https://docs.espressif.com/projects/esptool/en/latest/esp32/installation.html). If you're allergic to the terminal or command prompt you may be able to find a GUI alternative for your OS.

- **Pre-compiled firmware binary** - download the `.bin` from the latest [LR3 release](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot/releases?q=LR3%2F).

  - *Alternatively*, you can compile it from source using `litter-robot3.yaml`:

  ```bash
  esphome compile /path/to/litter-robot3.yaml
  ```

  The compiled binary will be in `./.esphome/build/litter-robot3/build/firmware.factory.bin` relative to the `.yaml` file.

---

## Accessing the Mainboard

You can follow [Whisker's official video guide for replacing the circuit board](https://www.youtube.com/watch?v=Axe9soTNzHU) up to the part where the circuit board is removed (stop at 1:15), otherwise expand the steps below:

<details>
<summary>Step by step instructions</summary>
<br>

1. **Prepare the unit.** Make sure the globe is in the home position. Press the Power button to turn the unit off, then unplug it from the wall. Remove any accessories.

2. **Remove the bonnet.** Press the bonnet tabs on both sides of the unit to release the bonnet, rotate it backward, and lift it off.

3. **Remove the globe.** Lift the globe off the base and set it aside.

4. **Remove the waste drawer.** Pull it out and set it aside.

5. **Remove the control panel cover.** Unscrew the 5 screws and take care not to lose them.

6. **Separate the control panel cover.** Push the buttons through the cover and lift it up. If necessary, use needle-nose pliers to lift the cover out of place.

7. **Remove the circuit board.** Turn the keypad and circuit board over and unplug the three connectors. Remove the rubber keypad membrane.

> The circuit board and keypad may be held in place by a piece of tape (assembly aid). If present, remove it; it is not required for reassembly.

</details>

## Preparing the Mainboard

> The following directions assume the mainboard is oriented with the three connector sockets on the left, and the "ESP PRG" silkscreen text upright in the lower middle of the board.

![ESP-Prog Header Location](Images/ESP-Prog%20Header%20Location.JPG)

The ESP PRG pads may or may not be populated with a 2x3 6 pin header. The pins **do not** match the ESP-Prog-2 pin order so we can't use them as-is.

You will need to solder directly to the bare pads, or carefully use the female jumper wires on the small pins if they are already present. Mine was unpopulated, so I opted to cut some female servo connectors and glue them together as a makeshift 6 pin connector. You can do something similar with the jumper cables if you'd like to leave it for future use, just don't forget to mark pin 1!

There may also be conformal coating on the board that will need to be scratched off prior to soldering.

In any case, you will have to follow this diagram when connecting the wires:

![ESP-Prog-2 Connected](Images/ESP-Prog-2%20Connected.JPG)

**Make sure the ESP-Prog-2 "VPROG" jumper is set to 3.3v first. When flashing the mainboard outside the unit, the ESP-Prog provides power; place it on a non-conductive surface before plugging in the USB-C cable. Do not connect pin 2 while installed in the Litter Robot.**

Be sure to triple check the wiring is correct and there are no shorts. The pins of the large and small 6 pin connectors on the ESP-Prog-2 are electrically identical, so they can be used for continuity testing.

---

## Backing Up the Stock Firmware

Before overwriting anything, make a full backup of the stock firmware. With the mainboard connected to the ESP-Prog-2 and the programmer plugged into your computer, follow along in terminal or command prompt:

Determine the correct serial port:

> check /dev/ttyACM* on Linux, COM* on Windows, /dev/cu.usbserial-* on macOS

```bash
ls /dev/ttyACM*
```

We will assume /dev/ttyACM0 for the rest of the instruction.

Make sure the computer can communicate with the ESP32:

```bash
esptool --port /dev/ttyACM0 flash-id
```

If you get permission errors you may need to add your user to the `dialout` group and log out, or just take ownership with:

```bash
sudo chown $USER /dev/ttyACM0
```

You should see information about the ESP32, including the flash size. Mine is 4MB, so we'll be sure to back up all of it:

```bash
esptool --port /dev/ttyACM0 --baud 921600 read_flash 0x0 0x400000 LR3_stock_firmware_backup.bin
```

> If you encounter communication errors, try lowering the baud rate (e.g., `--baud 460800` or `--baud 115200`)

If all went well you should have a ~4MB file called `LR3_stock_firmware_backup.bin` in the current directory.

**Keep this backup file safe.** You'll need it if you ever want to restore the original firmware.

---

## Flashing the ESPHome Firmware

Now that the backup is complete, erase the full flash:

```bash
esptool --port /dev/ttyACM0 --baud 921600 erase_flash
```

Write the ESPHome firmware starting at address 0x0:

```bash
esptool --port /dev/ttyACM0 --baud 921600 write_flash 0x0 /path/to/firmware.bin
```

> Be sure to use the `.bin` file and not `.ota`.

A successful flash will end with output similar to:

```
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
```

---

## Reassembly

Now that the new firmware is flashed, you can once again follow [Whisker's official video guide](https://youtu.be/Axe9soTNzHU?t=77) from 1:17 to reassemble, or just follow the disassembly instructions in reverse.

## Next Steps

Power the unit back on and continue with the [User Guide](User%20Guide.md) to provision the device and add it to Home Assistant.

---

## Restoring the Stock Firmware

To return the LR3 to its original state, flash the backup you made earlier:

```bash
esptool --port /dev/ttyACM0 --baud 921600 write_flash 0x0 /path/to/LR3_stock_firmware_backup.bin
```

The unit will reboot with the original firmware and reconnect to the WiFi and Whisker cloud if it was previously.
