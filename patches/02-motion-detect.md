# Motion Detection via LPCOMP + Channel Alert

### Highlights

- **Hardware comparator (LPCOMP)** -- uses nRF52840's low-power comparator instead of polling analogRead, near-zero power consumption while idle
- **Debounced state** -- 500ms debounce prevents false triggers from signal bounce during transitions
- **Group message flood** -- sends "Motion detected" to a private channel on rising edge (OFF -> ON)
- **Telemetry presence** -- reports motion state (ON/OFF) as CayenneLPP presence in telemetry responses
- **60-second cooldown** -- prevents flooding the mesh on sustained or repeated motion
- **Compile-time opt-in** -- entirely behind `MOTION_DETECT_ENABLED` flag, zero overhead when disabled

---

## Overview

This patch adds motion detection to the MeshCore repeater firmware using the nRF52840's LPCOMP (low-power comparator) peripheral. A solar motion light's LED ground wire is connected to AIN1 (WB_A1). When the light turns on (motion detected), the voltage on LED- drops from ~1.4V to ~0.15V, crossing the LPCOMP threshold. The firmware debounces the signal and sends a group message to the `fudge` private channel, and reports the motion state via telemetry.

## Hardware Setup

- **Board**: RAK4631 (nRF52840) + RAK19007 baseboard
- **Pin**: AIN1 on RAK19007 -> nRF52840 P0.31 (LPCOMP AIN7)
- **Connection**: LED ground wire of solar motion light -> AIN1
- **Signal**: ~1.4V idle (LEDs off), ~0.15V active (LEDs on)

## How It Works

1. At boot, `initMotionDetect()` configures the LPCOMP with VDD*2/8 (~825mV) threshold and 50mV hysteresis
2. The LPCOMP fires an interrupt on threshold crossings, recording the timestamp
3. Each loop iteration, `checkMotionState()` samples the comparator and applies 500ms debounce
4. On a debounced OFF -> ON transition, `sendMotionAlert()` sends a flood packet to the channel (with 60s cooldown)
5. Telemetry requests include a CayenneLPP presence field (1=motion, 0=idle)

## Configuration

| Flag | Default | Description |
|------|---------|-------------|
| `MOTION_DETECT_ENABLED` | *(not defined)* | **Required** -- define to enable |
| `MOTION_LPCOMP_AIN` | `7` | LPCOMP analog input (7 = AIN7 = P0.31 = WB_A1) |
| `MOTION_DEBOUNCE_MS` | `500` | Debounce window in milliseconds |
| `MOTION_COOLDOWN_MS` | `60000` | Minimum interval between alert messages (60s) |
| `MOTION_CHANNEL_NAME` | `"fudge"` | Channel name (for logging) |
| `MOTION_CHANNEL_KEY` | `{ 0x3d, 0xb6, ... }` | 16-byte pre-shared channel secret |

### PlatformIO Example

```ini
[env:rak4631_motion]
extends = env:rak4631_repeater
build_flags =
  ${env:rak4631_repeater.build_flags}
  -DMOTION_DETECT_ENABLED
```

## Channel Compatibility

To receive motion alerts, add a private channel on your companion app with:

- **Name**: `fudge` (or your configured `MOTION_CHANNEL_NAME`)
- **Key**: `3db6963cfd7d182166c9ab7f3e918750` (or your configured `MOTION_CHANNEL_KEY`)

## Files Modified

| File | Changes |
|------|---------|
| `examples/simple_repeater/MyMesh.h` | Config defines, LPCOMP include, motion state + channel members |
| `examples/simple_repeater/MyMesh.cpp` | LPCOMP ISR, `initMotionDetect()`, `initMotionChannel()`, `sendMotionAlert()`, `checkMotionState()`, telemetry presence, calls in `begin()` and `loop()` |

## Mesh Impact

- **One flood packet per motion event** -- minimal airtime usage
- **60-second cooldown** -- at most 1 packet per minute even with constant motion
- **3 extra bytes in telemetry** -- CayenneLPP presence field (on demand only)
- **Near-zero idle power** -- LPCOMP runs in hardware, no CPU polling
