# Motion Sensor as Telemetry (Presence)

### Highlights

- **AIN1 pin read as telemetry** -- reports motion state (HIGH/LOW) via CayenneLPP presence type
- **Standalone patch** -- does not require patch 02 (motion alert); can be used independently or alongside it
- **Compile-time opt-in** -- entirely behind `MOTION_TELEMETRY_ENABLED` flag, zero overhead when disabled
- **Live read on request** -- reads the analog pin at telemetry query time, so the value is always current

---

## Overview

This patch adds the motion sensor pin (AIN1) as a telemetry field in the repeater's telemetry response. When a companion app requests telemetry, the repeater reads the analog pin and reports it as a CayenneLPP **presence** value (1 = motion detected / HIGH, 0 = no motion / LOW), alongside the existing battery voltage and temperature fields.

## How It Works

1. A companion app sends `REQ_TYPE_GET_TELEMETRY_DATA` to the repeater
2. The repeater builds its normal telemetry (voltage, sensors, temperature)
3. **New**: if `MOTION_TELEMETRY_ENABLED` is defined, it reads `analogRead(MOTION_TELEM_PIN)` and adds `telemetry.addPresence(TELEM_CHANNEL_SELF, ...)` with 1 (HIGH) or 0 (LOW) based on the threshold
4. The companion app receives the CayenneLPP buffer containing the presence field

## Configuration

| Flag | Default | Description |
|------|---------|-------------|
| `MOTION_TELEMETRY_ENABLED` | *(not defined)* | **Required** -- define to enable |
| `MOTION_TELEM_PIN` | `WB_A1` (pin 31) | Analog input pin |
| `MOTION_TELEM_THRESHOLD` | `100` | ADC threshold for HIGH/LOW (~0.35V at 10-bit, 3.6V range) |

### PlatformIO Example

```ini
[env:rak4631_motion_telem]
extends = env:rak4631_repeater
build_flags =
  ${env:rak4631_repeater.build_flags}
  -DMOTION_TELEMETRY_ENABLED
```

## Companion App Display

The presence field uses CayenneLPP type 102 (`LPP_PRESENCE`), a 1-byte boolean. How it renders depends on your companion app:

- Apps with presence support will show something like "Presence: 1" or "Motion: ON"
- Apps without may show "unk: 1" or a raw hex value
- Either way the data is correctly encoded and readable

## Relationship to Patch 02 (Motion Alert)

This patch is **independent** of patch 02. You can use either, both, or neither:

| Patch | What it does |
|-------|--------------|
| 02 - Motion Alert | Sends a group message flood when motion is detected (proactive) |
| 03 - Motion Telemetry | Reports current motion state when telemetry is requested (on-demand) |

If both are enabled, they use separate flags (`MOTION_ALERT_ENABLED` vs `MOTION_TELEMETRY_ENABLED`) and separate pin/threshold defines to avoid conflicts, though they default to the same pin and threshold.

## Files Modified

| File | Changes |
|------|---------|
| `examples/simple_repeater/MyMesh.h` | Config defines (`MOTION_TELEM_PIN`, `MOTION_TELEM_THRESHOLD`) |
| `examples/simple_repeater/MyMesh.cpp` | `analogRead` + `addPresence` in `handleRequest()` telemetry block |

## Mesh Impact

- **Zero additional packets** -- only adds data to existing telemetry responses
- **3 extra bytes per response** -- 1 channel + 1 type + 1 data byte (CayenneLPP presence)
- **No new state or timers** -- reads pin on demand, stateless
