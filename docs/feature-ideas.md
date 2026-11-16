# Feature ideas

Research notes, not a roadmap. What the firmware already does, what the commercial boxes offer that it does not, and five additions that look buildable here. Competitor facts come from vendor pages and review sites found by web search (June 2026 snapshot); prices and tiers change, so treat them as indicative.

## 1. What this repo already has

**LR4** (`packages/litter-robot4/`, `esphome/components/litter_robot4/`)
- Full mainboard control over UART: manual cycle, empty, reset, factory reset, filter reset, power, panel lockout (cycles still work while locked), panel/night-light brightness and mode, clean-cycle delay.
- Telemetry: robot status and faults, waste-drawer %, litter level, last cat weight, curtain/weight detection sensors, odometers (clean, empty, filter, power cycles).
- Smart Cat Detection (weight AND curtain must agree), Auto Cycle on/off, per-day sleep schedule, Sleep Override that restores the schedule.
- LitterHopper enable/disable and hopper-empty inference (`packages/common/hopper-empty-detection.yaml`).
- WasteSense (BME688 over I2C on GPIO13/14, BSEC2): feces vs urine classification, Odor Control (fast cycle after feces), sensitivity slider, calibration reset.
- Hardening: reply-to-command matching, security guide (API key, OTA and web passwords), backup-verify flashing guide.

**LR3** (`packages/litter-robot3/`): full sleep-mode control, weight sensor access, waste-drawer cycle counting.

**Cat tracker** (`esphome/components/cat_tracker/`, `packages/cat-tracker/`): up to six cats, BLE collar-tag identification by RSSI with margin, per-cat weight, visit count, feces/urine counts, set-weight and reset entities, "Unknown" when unsure.

**Not present:** any history or trend logic (everything is a counter or last value), health alerts, maintenance forecasts, waste mass, camera, cross-robot sync, LR5 support.

## 2. What the others offer

| Feature | LR4 (stock) | LR5 / 5 Pro | LR EVO | Others |
|---|---|---|---|---|
| Waste type (urine/feces) | no | WasteID (camera) | no | Petkit Purobot Max Pro 2 (camera, pH) |
| Cat ID | weight only | face recognition, up to 5 cats, Whisker+ | weight | Petkit camera models; CatGenie A.I. camera |
| Health insights, trends, vet report | basic weight/visits | 7 days free, history and recap reports with Whisker+ | basic | Petkit/Neakasa apps: weight, frequency, abnormality alerts (some subscription) |
| Live view / clips | no | 2 night-vision 1080p cameras (Pro) | no | Petkit camera models |
| Drawer/litter level, maintenance | % and level, app alerts | same | "full / nearly full" only | Neakasa/Petkit: waste-bin and litter-low alerts, filter reminders |
| Odor control | none | WasteID-driven | none | Petkit air spray/deodoriser accessory |
| Waste mass | no | no | no | Petkit/Neakasa weigh waste per visit on weight-sensor models |
| Multi-robot per cat | app only | app | app | app |

Sources: [Whisker LR4 vs LR5](https://www.whisker.com/blog/litter-robot-4-vs-5), [LR5 lineup explained](https://technomeow.com/litter-robot-5-lineup-explained-lr5-vs-pro-vs-evo/), [Whisker LR4 vs EVO](https://www.whisker.com/blog/litter-robot-4-vs-evo), [LR5 Pro on litter-robot.com](https://www.litter-robot.com/litter-robot-5-pro.html), [Pura Max 2 vs Neakasa M1](https://frontdeskreview.com/gear/self-cleaning-litter-boxes/petkit-pura-max-2-vs-neakasa-m1/). Search coverage of ScoopFree, Tuya and Zigbee boxes was thin; they are mostly timer-and-raking designs with fewer sensors than the LR4, so nothing there beats what is listed. The EVO is real (compact, 1-2 cats, 3-19 lb) and has fewer sensors than the LR4, so it adds nothing to copy.

## 3. Proposed features

### A. Per-cat health trends and alerts: S/M, software only

- **What:** rolling 7-day averages per cat for visits per day, visit duration and weight; binary sensors for "weight changed more than X% in 7 days", "visits per day up or down", "no visit in N hours" and "cat in globe longer than N minutes". Comparable to the part of Whisker+ that is paywalled.
- **Why:** the highest-value health signal for cats (urinary issues, kidney disease, diabetes) is a change in visit frequency or weight. The tracker already knows who visited; it just forgets history.
- **How:** extend `cat_tracker` (`cat.cpp`/`cat.h`): keep a small ring of daily buckets per cat in flash (7-14 floats/ints, restored via the existing preference mechanism), publish the derived sensors from the existing sub-device. Visit duration comes from the detection events (`0x37`, curtain `0x58-0x5A`, weight `0x09`) already used by Smart Cat Detection. Clock is available (`sync_time` from HA). The "stuck in globe" alert is a timer on weight-present, with the robot itself still doing its safety logic.
- **Hardware:** none.
- **Caveats:** only as good as BLE identification (Unknown visits are skipped, which biases counts when a collar is off). Alerting thresholds are not medical advice; frame them as "changed" not "sick". HA statistics can do some of this off the existing counters, so the firmware part is mainly visit duration and the flash-backed history that survives HA outages.

### B. Maintenance forecast and notifications: S, software only

- **What:** "days until drawer full", "drawer full in N cycles", "filter due in N cycles", "litter low", "hopper empty", plus a "needs attention" text sensor.
- **Why:** LR4 only shows a percentage and a blinking light; cheaper rivals send "bin full" and "filter due" pushes. A forecast from the last few empties turns that into a planned chore.
- **How:** new `packages/litter-robot4/maintenance.yaml` template sensors over existing entities: waste drawer % (`0x43`), drawer full (`0x44`), clean/empty/filter/waste-drawer odometers (`0x3E`, `0x3F`, `0x40`, `0x42`), litter level (`0x47`), hopper inference. Fit cycles-per-percent at each Empty press, extrapolate with the per-day cycle rate. Add the same for LR3 from `waste-drawer-cycles.yaml`. If WasteSense is installed, VOC baseline drift is an optional "box smells, change litter" signal.
- **Hardware:** none.
- **Caveats:** `0x45` (cycles since full) is "not published consistently" per the UART reference, so do not depend on it. Filter life is a cycle count, not a measurement.

### C. Waste mass from the scale, to firm up urine detection: M, software only, experimental

- **What:** estimate the mass of a visit's waste from the weight difference before and after the cat, and use it as a second vote for "urine" (clump of roughly 50-100 g) versus "nothing" and feces. Optional extras: per-cat waste grams per day (a hydration and kidney signal).
- **Why:** WasteSense confirms feces, but urine is only assumed when no feces is seen, so a cat that enters and leaves without urinating produces false urine counts. Petkit and Neakasa already weigh waste.
- **How:** in `packages/cat-tracker/waste-tracking/` add a weight-delta path next to `waste-sense.yaml`: read the globe weight on `0x89` (decipounds, needs Debug Mode, emitted every second) at entry and after exit during the cycle delay. Combine with the VOC result.
- **Hardware:** none.
- **Caveats, honestly:** resolution is 0.1 lb (about 45 g), a urine clump is only 1-2 steps, and the scale auto-tares on negative readings and may zero out slow drift, which could erase the signal. Needs a logging experiment with real visits before committing to it. Debug mode adds UART traffic. May turn out not to be reliable; the output should be a diagnostic sensor first, and only feed the classifier once it has been shown to separate visits.

### D. Camera companion for visit snapshots and clips: L, extra hardware

- **What:** a still or short clip per visit, attached in HA to the cat that was identified, for people who want the "see your cat" part of LR5 Pro. The cat ID itself stays on BLE, so no on-device face recognition is needed.
- **Why:** the visible differentiator of the LR5 Pro, and useful for checking odd visits. Local only, so no subscription.
- **How:** a separate device, not the existing ESP32. The LR4 ESP32 is a plain `esp32dev` and is busy with UART, BLE scanning and BSEC2, and has no camera pins or PSRAM. Use an ESP32-S3 camera module running ESPHome `esp32_camera`, triggered from HA by the robot's visit event, with snapshots or an MJPEG stream into HA or Frigate.
- **Hardware:** Seeed XIAO ESP32S3 Sense (OV2640, PSRAM, USB-C, about $14) or an ESP32-S3 board with an OV5640; mount it on the bonnet or above the entrance looking at the globe opening, with an IR-capable lens or small IR LED board for night. Power it from its own USB-C supply. This repo does not document a spare 5 V or 3.3 V pad with enough current on the LR4 mainboard, so do not tap the mainboard until that is measured. No connection to the mainboard UART is needed.
- **Caveats:** physical mounting and a power cable are the real work; the firmware is a stock ESPHome config plus an HA automation, so this may be best as documentation and an example package rather than code in this repo.

### E. Cross-robot cat sync as a Home Assistant package: S/M, software only

- **What:** a shipped HA blueprint or package that keeps `Set Cat N Weight` and the count sensors consistent across several robots (the User Guide asks for it explicitly).
- **Why:** multi-box homes lose a cat's history whenever the cat uses the other box; LR5 handles this in its cloud.
- **How:** `input_number` per cat as the source of truth, a blueprint automation that writes it to each robot's `Set Cat N Weight` after a visit and sums the visit/waste counters with template sensors. Needs a way to push a weight without it counting as a visit, which `Set Cat N Weight` already provides.
- **Hardware:** none.
- **Caveats:** mostly HA YAML, not firmware, and only helps people with more than one robot.

## 4. Not worth pursuing

- **On-device face or waste image recognition (WasteID clone):** needs a camera and a model that does not fit in this ESP32; BLE plus BME688 already cover the use case.
- **New cleaning-cycle behaviours (rake speed, cycle patterns):** would depend on UART commands that nobody has reversed (the reference lists no such register), and the PIC owns motion safety.
- **LR5 support:** already announced as coming; protocol not covered here.
