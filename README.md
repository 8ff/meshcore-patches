# meshcore-patches

Patches and automated builds for [MeshCore](https://github.com/meshcore-dev/MeshCore) firmware. Applies patches on top of upstream releases and builds companion radio firmware for all supported boards.

## Patches

| Patch | Description | Documentation |
|-------|-------------|---------------|
| [01-firmware-retry](patches/01-firmware-retry.patch) | Auto-retry for DM and group messages with HeardRepeatsService fix | [Details](patches/01-firmware-retry.md) |

## Pre-built Firmware

Pre-built firmware for all supported boards is available on the [Releases](../../releases) page. Each release is built from a specific upstream MeshCore version with all patches applied.

### Flashing

1. Download the firmware file for your board from the [Releases](../../releases) page
2. Go to [MeshCore Flasher](https://flasher.meshcore.co.uk/)
3. Select **Custom Firmware** at the bottom
4. Upload your firmware file and flash

For more details, see the upstream [MeshCore documentation](https://meshcore.co.uk/docs/).

### Building for a specific board

If your board isn't in the latest release, you can build it yourself:

1. Fork this repo
2. Go to **Actions** > **Build Companion Radio (Lightweight)**
3. Select your board and connection type (ble/usb/wifi/serial/all)
4. Optionally specify an upstream tag
5. Run — download the firmware artifact from the completed run

## Applying Patches Manually

To apply patches to a local MeshCore checkout:

```bash
git clone https://github.com/meshcore-dev/MeshCore
cd MeshCore
git checkout companion-v1.14.1  # or any release tag
git apply /path/to/patches/01-firmware-retry.patch
```

## License

GPL-3.0 — matching [upstream MeshCore](https://github.com/meshcore-dev/MeshCore).
