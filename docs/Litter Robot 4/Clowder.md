# Clowder: one device per cat in Home Assistant

`packages/litter-robot4/clowder.yaml` sends every visit to the
[Clowder](https://github.com/davidcoulson/ha-clowder) Home Assistant integration. Clowder adds
up the visits from all your robots, so each cat is one device with one weight and one set of
counts, instead of a copy on every robot.

The robot still decides which cat it was (that part is yours: collar tags, weight, or
anything else). The package only carries the result to Home Assistant, reliably.

## Setup

Needs `packages/litter-robot4/waste-sense.yaml` (for the feces or urine verdict) and the
Clowder integration installed in Home Assistant.

```yaml
substitutions:
  serial: LR4C000000
  clowder_cats: "Ash,Bean,Coco"   # names in slot order; the same on every robot

packages:
  lr4:
    url: https://github.com/davidcoulson/esphome-litter-robot
    ref: <tag>   # a release tag of this repository, e.g. djc-2026.11.24.02
    files:
      - packages/litter-robot4/waste-sense.yaml
      - packages/litter-robot4/clowder.yaml
```

Then, wherever your config accepts a weight report as a visit, call:

```cpp
id(clowder_queue).open(slot, rssi, weight_lb);   // slot 1 = first name; 0 = cat not identified
```

## What it does

- Each visit is stored in flash and sent to Home Assistant as an `esphome.clowder_visit`
  event at once, then again when WasteSense has decided feces or urine.
- The robot keeps the visit until Clowder acknowledges it (it calls the robot's
  `clowder_ack` action), and resends it every minute and whenever Home Assistant connects.
  A visit made while Home Assistant is down is counted, at the right time, when it is back.
- Up to 16 visits wait in the queue. If more pile up, the oldest is dropped and a warning is
  logged.
- A visit with no cat identified is sent with slot 0 and also counted on the robot's own
  **Unidentified Visits** sensor (this robot only; Clowder's **Unidentified visits** sensor
  counts them across all robots).
- When Home Assistant connects, the robot sends `esphome.clowder_hello` with its cat names,
  so the cat devices exist before the first visit.

## Limits

- If the robot restarts while a visit is waiting for its verdict, or no verdict arrives
  within `max_pending` (45 minutes), the visit is sent without one: it counts as a visit but
  as neither poop nor pee.
- Waste weight is not sent, so Clowder's **Poop weight** and **Pee weight** sensors stay empty.
- An identified visit is always sent with `method` `Tag`, whatever your config used to tell
  the cats apart. Clowder does not use the field.
- In Home Assistant, each robot must be allowed to send events: Settings → Devices &
  services → ESPHome → the robot → Configure → turn on "Allow the device to perform Home
  Assistant actions". Without it no visit reaches Clowder.

The event fields and the acknowledgement are listed in the
[Clowder README](https://github.com/davidcoulson/ha-clowder#under-the-hood) under "Under the hood".
