# meshcore-patches

[![Build](https://img.shields.io/github/actions/workflow/status/8ff/meshcore-patches/build-patched-release.yml?branch=main&label=build)](https://github.com/8ff/meshcore-patches/actions)
[![Release](https://img.shields.io/github/v/release/8ff/meshcore-patches?label=latest)](../../releases)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![Upstream](https://img.shields.io/badge/upstream-MeshCore-orange)](https://github.com/meshcore-dev/MeshCore)

> Patches and automated builds for [MeshCore](https://github.com/meshcore-dev/MeshCore) firmware.

---

## Patches

| # | Patch | Role | Description | Docs |
|---|-------|------|-------------|------|
| 01 | [firmware-retry](patches/01-firmware-retry.patch) | Companion | Keeps retrying DM and group messages in the background until they go through | [Details](patches/01-firmware-retry.md) |
| 02 | [motion-alert](patches/02-motion-alert.patch) | Repeater | Sends a group message to a private channel when motion is detected on AIN1 | [Details](patches/02-motion-alert.md) |
| 03 | [motion-telemetry](patches/03-motion-telemetry.patch) | Repeater | Reports motion state (HIGH/LOW) as CayenneLPP presence in telemetry responses | [Details](patches/03-motion-telemetry.md) |

All patches are independent and can be combined freely.

---

## Pre-built Firmware

Pre-built firmware for all supported boards is available on the [**Releases**](../../releases) page.

### Flashing

1. Download the firmware file for your board from [**Releases**](../../releases)
2. Go to [**MeshCore Flasher**](https://flasher.meshcore.co.uk/)
3. Select **Custom Firmware** at the bottom
4. Upload your firmware file and flash

> For more details, see the upstream [MeshCore documentation](https://meshcore.co.uk/docs/).

---

## Building

### Full release (all boards)

1. Go to **Actions** > **Build Patched Release**
2. Select a **role** (companion / repeater / both)
3. Check which **patches** to include
4. Optionally specify an upstream tag (auto-detects latest)
5. Run — firmware files appear as a GitHub release

### Single board

1. Go to **Actions** > **Build Single Board**
2. Select your **board**, **role** (companion / repeater), and **connection type**
3. Check which **patches** to include
4. Optionally specify an upstream tag
5. Run — download the firmware artifact from the completed run

---

## Applying Patches Manually

```bash
git clone https://github.com/meshcore-dev/MeshCore
cd MeshCore
git checkout companion-v1.14.1  # or any release tag

# Apply whichever patches you need
git apply /path/to/patches/01-firmware-retry.patch
git apply /path/to/patches/03-motion-telemetry.patch

# For patches 02/03, add build flags to your PlatformIO env or export:
export PLATFORMIO_BUILD_FLAGS="-DMOTION_TELEMETRY_ENABLED"
```
