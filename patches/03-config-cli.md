# Patch 03: Config Channel CLI

**Role:** Companion
**Build flag:** `-D CONFIG_CLI_ENABLED=1`

## Overview

Adds a private encrypted channel called "config" that acts as a text CLI for managing companion firmware settings. Messages on this channel are intercepted locally before hitting the radio — zero airtime, instant responses. Works with stock iOS/Android/web apps without modification.

## How It Works

1. Firmware creates a "config" channel on first boot (persists across reboots)
2. Channel appears in the app's channel list alongside "Public" etc.
3. User types commands on the config channel
4. Firmware intercepts the message in `handleCmdFrame()` before `sendGroupMessage()` — the radio never fires
5. Command is parsed by the `ConfigCLI` module
6. Reply is pushed back as a fake incoming channel message
7. App displays the reply in the channel chat view

## Commands

| Command | Description |
|---|---|
| `help` | List available commands |
| `status` | Battery, uptime, airtime, packet stats, queue depth, contact count |
| `get config` | Dump all radio/node parameters and firmware version |
| `set name <X>` | Change node name |
| `set freq <X>` | Change frequency (400-2500 MHz) |
| `set bw <X>` | Change bandwidth (7.8-500 kHz) |
| `set sf <X>` | Change spreading factor (5-12) |
| `set cr <X>` | Change coding rate (5-8) |
| `set power <X>` | Change TX power (-9 to 30 dBm) |
| `set repeat <on\|off>` | Toggle packet forwarding (client repeat mode) |
| `advert` | Send zero-hop self advertisement |
| `advert flood` | Send flood self advertisement |
| `reboot` | Reboot the device |

## Architecture

The command parser (`ConfigCLI.h` / `ConfigCLI.cpp`) is a standalone C-style module with no MeshCore class dependencies. It takes a text command and returns a text reply through a struct of pointers to config values and action callbacks.

This module is designed to be reusable by repeater firmware in a future patch — the repeater would receive commands over the mesh on the same channel, process them through the same parser, and reply over the air.

```
ConfigCLI module (shared)
    |
    +-- Companion: intercepts locally, no radio traffic
    +-- Repeater (future): receives over mesh, replies over mesh
```

## Files

| File | Change |
|---|---|
| `src/helpers/ConfigCLI.h` | New — shared parser interface |
| `src/helpers/ConfigCLI.cpp` | New — parser implementation (~200 lines) |
| `examples/companion_radio/MyMesh.h` | Modified — config channel members and methods |
| `examples/companion_radio/MyMesh.cpp` | Modified — init, intercept, handler, reply (~130 lines) |
| `platformio.ini` | Modified — added ConfigCLI.cpp to build_src_filter |

## Debug Logging

All operations log to Serial via `MESH_DEBUG_PRINTLN` when `MESH_DEBUG` is defined. Log messages are prefixed with `ConfigCLI:` for easy filtering. Covers: channel init, command parsing, reply queuing, prefs save, advert send, reboot, and errors.

## Build

```bash
# Apply to MeshCore source
git apply patches/03-config-cli.patch

# Add build flag to your board's environment
export PLATFORMIO_BUILD_FLAGS="-DCONFIG_CLI_ENABLED=1"

# Or add directly to variant platformio.ini:
#   build_flags = ... -D CONFIG_CLI_ENABLED=1
```

## Customization

Override defaults with build flags:

```
-D CONFIG_CHANNEL_NAME='"mycfg"'    # channel name (default: "config")
-D CONFIG_CHANNEL_PSK='"base64..."'  # channel PSK (default: static key)
```
