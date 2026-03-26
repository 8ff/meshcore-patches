# meshcore-patches

[![Build](https://img.shields.io/github/actions/workflow/status/8ff/meshcore-patches/build-patched-release.yml?branch=main&label=build)](https://github.com/8ff/meshcore-patches/actions)
[![Release](https://img.shields.io/github/v/release/8ff/meshcore-patches?label=latest)](../../releases)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![Upstream](https://img.shields.io/badge/upstream-MeshCore%20v1.14.1-orange)](https://github.com/meshcore-dev/MeshCore/releases/tag/companion-v1.14.1)

> Patches and automated builds for [MeshCore](https://github.com/meshcore-dev/MeshCore) companion radio firmware.

---

## Patches

| # | Patch | Description | Docs |
|---|-------|-------------|------|
| 01 | [firmware-retry](patches/01-firmware-retry.patch) | Keeps retrying DM and group messages in the background until they go through — shows "Delivered" or "X repeats heard" even after the app gives up | [Details](patches/01-firmware-retry.md) |

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

<sub>MIT — matching [upstream MeshCore](https://github.com/meshcore-dev/MeshCore).</sub>
