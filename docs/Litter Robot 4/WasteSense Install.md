# Litter Robot 4 - WasteSense™ Sensor Install Guide

This small modification makes your Litter Robot 4 capable of identifying the type of waste, similar to Whisker's "WasteID". This information is used to better control odors by cycling much sooner after a feces visit.

> Disclaimer: This project is not affiliated with, endorsed by, or associated with Whisker, Litter Robot, or Automated Pet Care Products. Modifying your device involves hardware disassembly and modification which may void your warranty and carries a risk of permanent damage. Proceed at your own risk.

Before starting, you should already be comfortable with the mainboard removal in the [Firmware Flashing Guide](Flashing%20Instructions.md). You must flash the **`litter-robot4-waste-sense.yaml`** firmware rather than the standard `litter-robot4.yaml`.

**This guide is only for serial numbers starting with LR4C**. Other revisions have a different layout, though the principle is the same and the firmware is compatible.

## How It Works

A **Bosch BME688** environmental sensor is connected to the mainboard so the firmware can classify waste by the VOC signature of a visit. GPIO 13 and 14 are used to communicate with the sensor, and are most easily accessible via the **unpopulated JTAG pads** (J5). The BME688 also needs 3.3V and ground, which are available nearby.

> Note: Feces is confirmed by the sensor, but urine is only assumed by the lack of feces. False urine identifications are likely if your cat tends to enter the Litter Robot without urinating.

![Completed install](Images/Waste%20Sense/LR4C/Completed.JPG)

There are two ways to attach the sensor:

1. **Direct solder wires** - Solder wires directly from the sensor breakout board to the pads on the mainboard. Gets the job done.
2. **Using JTAG ribbon cable** - Install the JTAG header and use the **10 pin JTAG ribbon cable included with the ESP-Prog-2**. Cleaner, sensor can be added and removed easily.

## What You Need

> All Amazon links in this guide are affiliate links.

- **[BME688 breakout board](https://amzn.to/4imVx8R)** with `VCC`, `GND`, `SCL`, `SDA` pads/pins.

- **Soldering iron** (fine/curved tip) and solder.

- **[Fine fiberglass brush](https://amzn.to/3TKakku)** or razor blade to remove conformal coating.

- **Wire**.

**For the JTAG ribbon cable option:**

  - **1.27mm (0.05") pitch dual row surface-mount header** - You technically only need 2x2, but the full header is 2x5.

  - **1.27mm ribbon cable** - I recommend using the 10 pin JTAG cable that comes with the [ESP-Prog-2](https://www.mouser.com/en/ProductDetail/Espressif-Systems/ESP-Prog-2?qs=HMhDvBYWqvPF20t9F8LmIA%3D%3D) if you already got one for flashing and don't plan on using it.

## Modification

> The directions below assume the mainboard is oriented with the control panel facing up and the LED indicators facing you, as they are in normal use.

<details>
<summary>Direct solder</summary>
<br>

1. Locate the J5 (JTAG) and J7 pads next to the ESP32.

![Mainboard close up](Images/Waste%20Sense/LR4C/Mainboard%20close%20up.JPG)

2. Scrape the conformal coating off the 2 top left J5 pads with the fiberglass brush or a razor blade. Be careful not to cut any traces. J7's through-hole pads should not have any conformal coating.

3. Solder 4 wires from the BME688 to the matching pads on the mainboard labelled below.

   ![I2C pads](Images/Waste%20Sense/LR4C/I2C.JPG)

   - The top left through-hole pad of J7 is `VCC`, the bottom right of J7 is `GND`.
   - The top left J5 pads are `SCL` and `SDA` from left to right.

</details>

<details>
<summary>Using JTAG ribbon cable</summary>
<br>

1. Scrape the conformal coating off the J5 pads with the fiberglass brush or a razor blade. Be careful not to cut any traces. J7's through-hole pads should not have any conformal coating.

2. Populate the unpopulated JTAG pads with a 1.27mm dual row header. Only the first 2x2 pins on the left are needed, but I recommend using a 2x5 header to populate the whole thing.

3. Reroute VCC. The JTAG VCC pin (pin 1) is not connected on the PCB, so you need to run a short wire from the **top-left pad of J7** to **the bottom left pad of J5**.

   ![JTAG header VCC jumper](Images/Waste%20Sense/LR4C/JTAG%20header.JPG)

   > The left photo shows the VCC jumper installed; the right photo shows the same header with the ribbon cable attached.

4. Plug the **1.27mm JTAG ribbon cable** into the JTAG header with the pink wire on the left.

5. Cut the other end off of the **JTAG ribbon cable** and solder it to the **BME688 breakout board**. Allow at least 3 inches or 80mm of length.

   ![BME688 breakout](Images/Waste%20Sense/LR4C/BME680.JPG)

   - Counting from the pink wire, wires 1-4 carry `VCC`, `SCL`, `GND`, and `SDA` in that order. Solder each to the matching pin labeled on your breakout board. My breakout board (pictured above) swaps the order of `SCL` and `GND`.

![Completed install](Images/Waste%20Sense/LR4C/Completed.JPG)

The complete setup should look something like the above image. The remaining ribbon cable wires should be cut cleanly and taped off to prevent shorts.

</details>
<br>

The WasteSense™ firmware uses `0x77` as the default BME688 I2C Address. If your breakout board defaults to `0x76` you will need to change it. This is usually done by connecting `SDO` to `VDDIO` (3.3V). Bridging `SDO` to `GND` selects `0x76`.

## Mounting

![Sensor installed](Images/Waste%20Sense/LR4C/Installed.JPG)

The BME688 should be mounted near the laser board as shown. I used VHB tape to mount it, though any means will work as long as you don't cover the sensor itself.

I left most of the full length of the ribbon cable, but you can cut it shorter for a cleaner install.

## Firmware and Verification

1. Download the precompiled WasteSense firmware from the latest [LR4 release](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot/releases?q=LR4%2F), or compile **`litter-robot4-waste-sense.yaml`** yourself, then flash it following the [Firmware Flashing Guide](Flashing%20Instructions.md). As with the standard firmware, use the `.bin` for the initial wired flash and the matching `.ota` for later over-the-air updates (see [Updating the Firmware](User%20Guide.md#updating-the-firmware-ota)).

2. After it boots and is added to Home Assistant, open the device page and you will see "Litter Robot 4 with WasteSense™" under device info and `Odor Control` toggle under configuration.

3. If `Odor Control` cannot be enabled, the BME688 was not detected. Check your wiring and make sure your breakout board is set to I2C address `0x77`.

> The BME688 sensor needs a few minutes to warm up after power on before its readings stabilize. Allow a few minutes after boot before relying on `Odor Control`.

The following sensors are also available, but disabled by default:

   - **Temperature**
   - **Humidity**
   - **Indoor Air Quality**
   - **IAQ Accuracy**
   - **Pressure**
   - **CO2 Equivalent**
   - **Breath VOC Equivalent**

There is also a disabled slider called `Odor Control Delay` to configure the specific cycle delay for feces when `Odor Control` is enabled.

The `Odor Control Sensitivity` slider under configuration scales the VOC rise needed to classify a visit as feces on a `1`-`10` scale. `5` matches the stock threshold, lower values require a larger rise (fewer feces detections), and higher values detect smaller rises. The stock setting is already quite sensitive, so lower it first if urine is being recorded as feces.

Feces is confirmed by the sensor, but urine is only assumed. If your cat does both at the same time frequently, increase `Odor Control Delay` so urine has some time to clump before cycling.

### Replacing the Sensor

The BSEC2 library learns the sensor's gas baseline over time and saves it to flash. A replacement BME688 would inherit the old sensor's calibration. If it is ever replaced, press the `WasteSense Calibration Reset` button in the diagnostic category. It clears only the saved BSEC2 calibration and restarts the device.

The new sensor then calibrates from scratch: `IAQ Accuracy` sits low for a while and feces detection is unreliable until it reaches 3. Bosch recommends letting a new sensor run for about 24 hours before trusting its readings.

## Waste mass (experimental)

An optional package estimates how much waste a visit left, from the globe weight before the cat enters versus after it leaves. It is **unproven** and for diagnosis only: it does not change how poop and pee are classified. The scale resolves 0.1 lb (about 45 g), so a pee clump is only one or two steps, and the scale's automatic tare can hide small changes.

To include it, add this to your config next to the `waste_sense` package:

```yaml
packages:
  waste_mass: !include packages/cat-tracker/waste-tracking/waste-mass.yaml
```

It needs **Debug Mode** switched on, since the globe weight is only reported then (it adds a UART message per second). It adds `Last Waste Mass` (g), `Waste Mass Detected`, the `Waste Mass Threshold` and `Waste Mass Settle Time` settings, and a disabled `Globe Weight` diagnostic sensor. Each visit is logged at INFO level under `waste_mass` with the before and after weights, so you can compare them with what was really in the drawer. Visits are skipped, with a warning, if the weight did not settle or no readings arrived. Keep `Waste Mass Settle Time` shorter than the cycle delay.

## Odor watch (experimental)

An optional package turns the BME688 VOC reading into a smoothed odor level, can run an extra clean cycle when the smell lingers, and warns when the litter probably needs changing. **All thresholds are untested on real hardware.** The defaults are guesses; watch `Odor Level` for a few days and adjust them.

To include it, add this to your config next to the `waste_sense` and `odor_control` packages:

```yaml
packages:
  odor_watch: !include packages/litter-robot4/odor-watch.yaml
```

It needs Home Assistant time (included by default) to count cycles per day.

- **`Odor Level`** (ppm): the `Breath VOC Equivalent` reading averaged over about 10 minutes, minus a slow floor. The floor drops quickly when the air gets cleaner and creeps up over about 16 hours, so a lasting smell fades from this value and shows up as drift instead. Nothing is reported for the first 10 minutes after boot.
- **`Smell Cycle`** switch (off by default): when `Odor Level` stays above `Smell Cycle Odor Level` for `Smell Cycle Minutes`, press Cycle once. At most `Smell Cycle Max Per Day` (default 2) run per day. It only runs when the robot is `Ready`, no cat or weight is detected, it is not in sleep mode, `Auto Cycle` is on and no `Odor Control` cycle is pending. After a smell cycle the timer starts over.
- **`Change Litter Soon`**: on when either the days-long average of `Odor Level` (about a 3 day time constant, saved every 30 minutes) is above `Change Litter Odor Level`, or the floor has risen more than `Change Litter Baseline Drift` above where it was when the litter was fresh. The first is odor that stays even after cycles; the second is the sensor's clean-air baseline creeping up. Press `Litter Changed` after changing the litter to restart both. The very first floor seen is taken as "fresh", so install it when the litter is fresh.

Assumptions: the sensor's own BSEC2 drift or a humid room can also move the baseline, and a cat that rarely uses the box leaves little odor to measure. Poop visits cause large spikes (hundreds of ppm), so `Odor Level` is dominated by them for the first minutes after a visit; set `Smell Cycle Minutes` longer than your `Odor Control Delay` plus a cycle so the normal cycle gets the first chance.

Tuning: log `Odor Level` in Home Assistant, note where it settles with clean litter and after a poop visit, then set `Smell Cycle Odor Level` a little above the normal after-visit tail and the two `Change Litter` thresholds above what a fresh box shows over a week.
