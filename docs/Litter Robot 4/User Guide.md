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

Each robot keeps its own tracker state, so with more than one unit the learned weights and counters will drift apart. Use Clowder to bring them together:

- **Clowder** (LR4 with WasteSense): `packages/litter-robot4/clowder.yaml` sends every visit to the [Clowder](https://github.com/davidcoulson/ha-clowder) Home Assistant integration, which keeps one device per cat across all your robots. Your config reports each identified visit (for example from a collar tag). See [Clowder](Clowder.md).
- **Cat sync** is replaced by Clowder on LR4 robots and is kept for the LR3, which has no Clowder package. See [`homeassistant/cat-sync`](../../homeassistant/cat-sync/README.md).

## Maintenance

Optional package that forecasts chores from the entities the robot already reports. Add it to your device config after the LR4 package (and, for the Hopper Empty entity, `hopper.yaml`, which is included by default):

```yaml
packages:
  maintenance: !include packages/litter-robot4/maintenance.yaml
```

- **Cycles Per Percent** - how many clean cycles it takes to add 1% to the waste drawer. It is fitted each time the drawer is emptied (the level drops by 20 or more points) and averaged with the previous fit. It is empty until the first empty after install.
- **Drawer Cycles Until Full** / **Days Until Drawer Full** - extrapolated from the current level and the cycle rate since the last empty. Days appear after 24 hours of data and ignore time the robot is powered off.
- **Filter Cycles Until Due** / **Filter Due** - `filter_life_cycles` (default 500) minus the Filter Cycles odometer. It is a cycle count, not a measurement of the filter; press **Replace Filter** to reset the odometer.
- **Litter Low** - litter level below `litter_low_percent` (default 25).
- **Needs Attention** - `OK`, or a comma-separated list of: Waste drawer full, Waste drawer nearly full (from `drawer_warn_percent`, default 80), Filter due, Litter low, Hopper empty. Use it as an automation trigger for notifications.

Override the thresholds with `substitutions:` in your device config. The fitted values are stored in flash.

## WasteSense™ and Odor Control

[WasteSense™](WasteSense%20Install.md) utilizes an optional BME688 sensor to classify waste as feces or urine, similar to Whisker's "WasteID". It requires flashing the **`litter-robot4-waste-sense.yaml`** firmware and soldering the sensor to the mainboard.

- **Odor Control** - when enabled, the robot cycles much sooner after a feces visit, rather than waiting for the full clean cycle delay.
- **WasteSense Timeout** - how long to keep watching for feces after the cat leaves before recording the visit as urine (0.5-5 minutes, default 2 minutes).
- **Odor Control Delay** - how long the cat must be gone before a feces cycle starts (0-10 minutes, default 1 minute). If feces is detected after this has passed, the robot cycles right away.
- **WasteSense Calibration Reset** - clears the sensor's saved calibration and restarts the device. Use it after replacing the BME688; see [Replacing the Sensor](WasteSense%20Install.md#replacing-the-sensor).

Feces is confirmed by the sensor, but urine is only assumed when no feces is detected within `WasteSense Timeout`. You will get false urine identifications if a cat enters without urinating. If feces visits are being recorded as urine because the odor rises slowly, increase `WasteSense Timeout`. If your cat regularly does both in one visit, increase `Odor Control Delay` so urine has some time to clump before cycling.

> The BME688 sensor needs a few minutes to warm up after power on before its readings stabilize. Allow a few minutes after boot before relying on `Odor Control`.

## Sleep Schedule

Sleep schedules run on the robot's clock, which is set from Home Assistant in the `America/New_York` timezone by default. If you live elsewhere, set your timezone in the [adopted config](#adopting-in-esphome-device-builder):

```yaml
substitutions:
  timezone: Europe/London
```

Turning on `Sleep Override` keeps the robot asleep until it is turned off, which restores the previous schedule.

## Web Server

The firmware hosts a lightweight web server on the device's IP address for basic control and OTA updates. The default username is `user` and the password is `Wh1sk3rM30wM30w`. You can find the device IP in your router's device list.

## Adopting in ESPHome Device Builder

The configuration can be "adopted" into [ESPHome Device Builder](https://desktop.esphome.io/) for further configuration. It should automatically show up at the top of the dashboard in a "Discovered Devices" tab. The auto-generated config imports the `litter-robot4.yaml` from this repository as a base package, so you can layer your own customizations on top without losing upstream updates.

## Securing the Device

Out of the box the native API is unencrypted, ESPHome OTA has no password, and the web server and setup hotspot use the shared default password above. Anyone on your network can then press any button (including `Factory Reset`) or flash new firmware. After adopting the device, add the following to its config in ESPHome Device Builder and the matching values to your `secrets.yaml`:

```yaml
api:
  encryption:
    key: !secret litter_robot4_api_key # generate one at https://esphome.io/components/api/

ota:
  - id: !extend esphome_ota
    password: !secret litter_robot4_ota_password

wifi:
  ap:
    password: !secret litter_robot4_ap_password

web_server:
  auth:
    password: !secret litter_robot4_web_password
```

Install the update once over the existing unencrypted connection, then re-add the device in Home Assistant with the new encryption key.

## Updating the Firmware (OTA)

Once ESPHome is installed, future firmware updates can be applied over the air without reopening the unit. This process is automated in the ESPHome Device Builder dashboard, but you can also update via the web server:

1. Download the latest OTA update file (`.ota`) from the latest [LR4 release](https://codeberg.org/Joseph-DiGiovanni/esphome-litter-robot/releases?q=LR4%2F).

2. Open the device's web interface using the hostname or IP of the device in a browser. The default username is `user` and the password is `Wh1sk3rM30wM30w`.

3. At the bottom, use the "OTA Update" file selector to select the downloaded OTA update file and click update. Be sure to not use the `.bin` file or the update will fail.

To return the device to the original Whisker firmware, see [Restoring the Stock Firmware](Flashing%20Instructions.md#restoring-the-stock-firmware).

## Cat Health History

The cat tracker keeps 15 days of per-cat history in flash (it survives reboots and Home Assistant outages) and needs the robot's clock to be set. Include `packages/cat-tracker/health.yaml` (cats 1-4) or add the entities to your own cats:

```yaml
sensor:
  - platform: cat_tracker
    cat: cat1
    visits_per_day_7d: {name: "Cat 1 Visits per Day (7d)"}   # last 7 full days
    avg_weight_7d: {name: "Cat 1 Average Weight (7d)"}
    hours_since_visit: {name: "Cat 1 Hours Since Last Visit"}
binary_sensor:
  - platform: cat_tracker
    cat: cat1
    type: weight_changed   # last 3 days vs the 10 before, percent, default 5
    name: "Cat 1 Weight Changed"
  - platform: cat_tracker
    cat: cat1
    type: visits_changed   # last 7 days vs the 7 before, percent, default 40
    name: "Cat 1 Visits Changed"
  - platform: cat_tracker
    cat: cat1
    type: no_visit         # hours since the last identified visit, default 24
    name: "Cat 1 No Recent Visit"
    threshold: 18
```

Only visits the tracker attributed to a cat count, so a cat that skips its collar looks like it is visiting less. The alerts say something changed, not that your cat is sick. A change in weight or in how often a cat uses the box is worth a call to the vet, but it is not a diagnosis. Weight alerts need 3 weighed visits in each period and visit alerts need two full weeks of history since the tracker started (or was reset), so they stay off for the first two weeks. Visits heard before the clock is set are counted once it is. "Hours Since Last Visit" counts from the start of monitoring until a cat's first visit, so a cat that is never identified eventually raises "No Recent Visit". Visits per Day (7d) shows unknown until 7 full days of history exist. Days follow the local calendar, including across daylight saving changes.

### Visit duration and "in the box a while"

If your cats wear BLE tags, the tracker can time each visit. It needs a sighting from your tag-matching code, so the cat is still identified the way you do it today; add one call to your existing `on_ble_advertise`, for the cat whose tag was heard:

```yaml
esp32_ble_tracker:
  on_ble_advertise:
    - then:
        - lambda: |-
            // your existing matching, then for the matched cat:
            if (x.address_uint64() == 0xAABBCCDDEEFF)
              id(cat1).tag_seen(x.get_rssi());
        # or, without a lambda:
        # - cat_tracker.tag_seen: {cat: cat1, rssi: !lambda "return x.get_rssi();"}

cat_tracker:
  cats:
    - id: cat1
      min_rssi: -80   # ignore sightings weaker than this (default -85)
      gap: 90s        # tag silent this long = visit over (default 60s)

sensor:
  - platform: cat_tracker
    cat: cat1
    last_visit_duration: {name: "Cat 1 Last Visit Duration"}          # seconds
    avg_visit_duration_7d: {name: "Cat 1 Average Visit Duration (7d)"} # seconds, includes today
binary_sensor:
  - platform: cat_tracker
    cat: cat1
    type: in_box_too_long   # minutes of continuous tag presence, default 10
    name: "Cat 1 In Box a While"
    threshold: 15
```

A visit is the time from the first sighting until the tag has been silent for the gap, so it includes the time the cat sits near the box with the tag in range; set `min_rssi` so only a tag inside or at the box counts. You can also end a visit sooner, for example when the robot reports the cat has left, with `cat_tracker.visit_ended` (add `cat: cat1` for one cat, or leave it out for all). The alert turns on while a tag has been heard continuously for longer than the threshold and off when the tag is gone. It says a cat has been in the box a while, nothing more, and is not a safety feature; the robot's own sensors and safety logic are unchanged. Durations use their own small flash record, are only stored while the clock is set, and the averages need at least one timed visit.
