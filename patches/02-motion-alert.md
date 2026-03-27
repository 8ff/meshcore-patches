# Motion Detection Alert via Group Message

### Highlights

- **Analog pin monitoring** -- reads AIN1 (RAK19007) to detect when a solar motion light's LED turns on
- **Group message flood** -- sends "Motion detected" to a configurable private channel (default `fudge`)
- **Debounced with cooldown** -- 60-second cooldown prevents flooding the mesh on sustained motion
- **Explicit channel key** -- uses a pre-shared 16-byte secret key (not derived from channel name)
- **Compile-time opt-in** -- entirely behind `MOTION_ALERT_ENABLED` flag, zero overhead when disabled

---

## Overview

This patch adds motion detection capability to the MeshCore repeater firmware. A RAK4631 + RAK19007 board is placed inside a solar motion light, with the light's LED+ wire connected to the AIN1 analog input. When the light triggers (motion detected), the firmware reads the voltage spike on AIN1 and sends a group message to the `fudge` private channel over the mesh network.

## Hardware Setup

- **Board**: RAK4631 (nRF52840) + RAK19007 baseboard
- **Pin**: AIN1 on RAK19007 J11 header -> nRF52840 P0.31 (Arduino pin `WB_A1`)
- **Connection**: LED+ of solar motion light -> AIN1 (max 3V, within the 3.6V ADC range)
- **Voltage**: 0V = no motion, ~3V = motion (LED on)

## How It Works

1. At boot, `initMotionChannel()` sets up the `GroupChannel` using the pre-shared secret key and derives the channel hash
2. Each loop iteration, `checkMotion()` reads the analog pin
3. When voltage crosses the threshold (rising edge), `sendMotionAlert()` fires once
4. A flood packet with `PAYLOAD_TYPE_GRP_TXT` is sent containing `"<node_name>: Motion detected"`
5. A 60-second cooldown prevents re-triggering
6. The trigger resets only after the voltage drops below threshold (motion light turns off)

### Edge Detection

The patch uses edge detection rather than level detection:
- Only triggers on the **rising edge** (voltage goes from low to high)
- Won't re-trigger during sustained motion (light stays on)
- Resets when motion ends (voltage drops), ready for next event
- Cooldown provides additional protection against rapid on/off cycling

## Configuration

All values are compile-time configurable via build flags. The feature is entirely opt-in:

| Flag | Default | Description |
|------|---------|-------------|
| `MOTION_ALERT_ENABLED` | *(not defined)* | **Required** -- define to enable the feature |
| `MOTION_PIN` | `WB_A1` (pin 31) | Analog input pin |
| `MOTION_THRESHOLD` | `100` | ADC threshold (~0.35V at 10-bit, 3.6V range) |
| `MOTION_COOLDOWN_MS` | `60000` | Milliseconds between alerts (60s) |
| `MOTION_CHANNEL_NAME` | `"fudge"` | Channel name (for logging) |
| `MOTION_CHANNEL_KEY` | `{ 0x3d, 0xb6, ... }` | 16-byte pre-shared channel secret |

### PlatformIO Example

```ini
[env:rak4631_motion_repeater]
extends = env:rak4631_repeater
build_flags =
  ${env:rak4631_repeater.build_flags}
  -DMOTION_ALERT_ENABLED
  -DMOTION_CHANNEL_NAME='"alerts"'
  -DMOTION_CHANNEL_KEY='{ 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 }'
  -DMOTION_COOLDOWN_MS=30000
```

## Channel Compatibility

The patch uses a pre-shared 16-byte secret key configured at compile time. To receive motion alerts, add a private channel on your companion app with:

- **Name**: `fudge` (or your configured `MOTION_CHANNEL_NAME`)
- **Key**: `3db6963cfd7d182166c9ab7f3e918750` (or your configured `MOTION_CHANNEL_KEY`)

The channel hash is derived as `hash[0] = SHA256(secret, 16)[0]`.

## Files Modified

| File | Changes |
|------|---------|
| `examples/simple_repeater/MyMesh.h` | Config defines, motion state members, method declarations |
| `examples/simple_repeater/MyMesh.cpp` | `initMotionChannel()`, `checkMotion()`, `sendMotionAlert()` implementations; calls in `begin()` and `loop()` |

## Mesh Impact

- **One flood packet per motion event** -- minimal airtime usage
- **60-second cooldown** -- at most 1 packet per minute even with constant motion
- **Standard group message format** -- compatible with all MeshCore clients
- **No ACK required** -- group messages use flood routing, fire-and-forget
