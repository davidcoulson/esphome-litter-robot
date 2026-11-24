# Cat sync across robots (Home Assistant)

> **Replaced by Clowder on Litter Robot 4.** On LR4 robots with WasteSense, use `packages/litter-robot4/clowder.yaml` and the [Clowder](https://github.com/davidcoulson/ha-clowder) integration instead: one device per cat across all robots, and no visit lost while Home Assistant is down. See [Clowder](../../docs/Litter%20Robot%204/Clowder.md). Cat sync stays for the Litter Robot 3, which has no Clowder package.

Keeps one cat's weight the same on every robot and sums visit, poop and pee counts across robots. Software only; nothing to flash.

## What was checked in the firmware

- `Set Cat N Weight` (`number`, lb, 0-50, step 0.01) calls `CatTrackerCat::set_weight()`. It **does not record a visit** or touch counts. It does update that robot's `Cat N Weight` sensor.
- After an identified visit the tracker calls `set_weight(measured)` on the matched cat, so the `Cat N Weight` sensor only changes on an identified visit, a manual set, or a reset (reset publishes NaN, shown as `unknown`).
- A robot only identifies a cat whose weight is already set (within `weight_tolerance`, default 0.6 lb). Pushing the shared weight is what lets robot B recognise a cat it has never seen.
- Counts (`Cat N Visits`, `Cat N Poop Count`, `Cat N Pee Count`) are per robot and cannot be set from HA, so they are summed, not synced.

Mismatches with `docs/feature-ideas.md`: none on mechanism. Two details it leaves out: the sensors and numbers ship `disabled_by_default: true` (enable them in HA), and the packages define only cats 1-4 (add more in your robot config to use 5-6).

## Entity naming

ESPHome device `robot_a` gives (HA lower-cases and underscores names):
`sensor.robot_a_cat_1_weight`, `number.robot_a_set_cat_1_weight`, `sensor.robot_a_cat_1_visits`, `sensor.robot_a_cat_1_poop_count`, `sensor.robot_a_cat_1_pee_count`.
Use the same cat number for the same cat on every robot.

## Install

1. Enable the `Cat N Weight`, `Set Cat N Weight`, `Cat N Visits`, `Cat N Poop Count` and `Cat N Pee Count` entities on each robot.
2. Copy `cat_sync.yaml` into your config (e.g. `packages/cat_sync.yaml`) and include it:
   ```yaml
   homeassistant:
     packages:
       cat_sync: !include packages/cat_sync.yaml
   ```
   Restart HA. This creates `input_number.cat_1_weight` ... `cat_6_weight`, 18 `input_text.cat_N_<counter>_entities` helpers, and sensors `Cat N Total Visits / Poop Count / Pee Count`.
3. For each cat and counter, fill in the matching **Cat N Visits / Poop Count / Pee Count entities** helper with the full entity ids of that cat's sensor on every robot, comma separated, e.g. `sensor.robot_a_meg_visits,sensor.robot_b_meg_visits`. Nothing is guessed from device names, and a total goes unavailable until every listed entity has a number.
4. Copy `blueprints/automation/cat_weight_sync.yaml` to `<config>/blueprints/automation/cat-sync/` and create **one automation per cat**: pick `input_number.cat_N_weight`, every robot's `Cat N Weight` sensor, and every robot's `Set Cat N Weight` number.
5. Seed the weight once by typing it into the `input_number`; it is pushed to all robots.

## How loops are avoided

An automation run acts on a robot sensor change only if it differs from the shared value by more than the tolerance (default 0.2 lb). Our own push makes the other robots' sensors equal the shared value, so their echo is ignored. Pushes skip the robot that triggered, and skip any robot already within 0.005 lb. Editing the shared helper pushes to every robot. A robot whose weight is still unknown (uncalibrated or just reset) is seeded by the push. When a robot comes back from unavailable, it is given the shared value rather than overwriting it with its saved one; only a real measurement updates the shared weight.

## Limits

- Totals sensors are `unavailable` if any listed robot's sensor is not numeric (avoids false dips). Counts are `state_class: total`.
- Up to 6 cats; per-robot cat numbering must match.
- Weight must stay in lb: the automation ignores sensors whose unit is not `lb`. Do not set a different display unit on these entities.
- A robot with `weight_tolerance` smaller than the real drift between boxes may still miss; tune tolerance.
- Resetting a cat on one robot (NaN) is not propagated.
- Counts only sum from when the package is installed; history is not merged.
- Not tested in a live HA instance.
