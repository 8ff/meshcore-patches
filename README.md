# meshcore-patches

[![Build Status](https://img.shields.io/github/actions/workflow/status/8ff/meshcore-patches/build-patched-release.yml?label=build)](https://github.com/8ff/meshcore-patches/actions)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue)](LICENSE)
[![MeshCore](https://img.shields.io/badge/upstream-MeshCore-orange)](https://github.com/meshcore-dev/MeshCore)

> Patches and automated builds for [MeshCore](https://github.com/meshcore-dev/MeshCore) companion radio firmware.

---

## Patches

| # | Patch | What it does |
|---|-------|-------------|
| 01 | [firmware-retry](patches/01-firmware-retry.patch) | Auto-retry for DM and group messages with HeardRepeatsService fix — [read more](patches/01-firmware-retry.md) |

### Highlights

- **DM auto-retry** — firmware retries up to 200 times in the background, even after the app shows "Failed". Updates to "Delivered" when it finally goes through.
- **Group/channel auto-retry** — retries every 10 seconds with echo detection. Companion app shows "X repeats heard" when repeaters pick it up.
- **No app changes needed** — works with existing iOS, Flutter, and JS companion apps.
- **No network spam** — retries are paced, cancelled on delivery, and use standard mesh dedup.

---

## Pre-built Firmware

Pre-built firmware for all supported boards is available on the [**Releases**](../../releases) page.
Each release is built from a specific upstream MeshCore version with all patches applied.

### Flashing

1. Download the firmware file for your board from [**Releases**](../../releases)
2. Go to [**MeshCore Flasher**](https://flasher.meshcore.co.uk/)
3. Select **Custom Firmware** at the bottom
4. Upload your firmware file and flash

> For more details, see the upstream [MeshCore documentation](https://meshcore.co.uk/docs/).

### Building for a specific board

If your board isn't in the latest release, you can build it yourself:

1. Fork this repo
2. Go to **Actions** > **Build Companion Radio (Lightweight)**
3. Select your board and connection type (ble / usb / wifi / serial / all)
4. Optionally specify an upstream tag
5. Run — download the firmware artifact from the completed run

---

## Applying Patches Manually

```bash
git clone https://github.com/meshcore-dev/MeshCore
cd MeshCore
git checkout companion-v1.14.1  # or any release tag
git apply /path/to/patches/01-firmware-retry.patch
```

---

<sub>GPL-3.0 — matching [upstream MeshCore](https://github.com/meshcore-dev/MeshCore).</sub>
