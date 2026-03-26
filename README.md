# meshcore-patches

Patches and automated builds for [MeshCore](https://github.com/meshcore-dev/MeshCore) firmware. Applies patches on top of upstream releases and builds companion radio firmware for all supported boards.

## Patches

| Patch | Description | Documentation |
|-------|-------------|---------------|
| [01-firmware-retry](patches/01-firmware-retry.patch) | Auto-retry for DM and group messages with HeardRepeatsService fix | [Details](patches/01-firmware-retry.md) |

## Workflows

### Build Patched Companion Release

Builds **all** companion radio firmware variants and creates a GitHub release with individual firmware files.

1. Go to **Actions** > **Build Patched Companion Release**
2. Optionally specify an upstream tag (defaults to latest `companion-*` tag)
3. Run — firmware files appear as a release under **Releases**

### Build Companion Radio (Lightweight)

Builds firmware for a **single board** — faster for development and testing.

1. Go to **Actions** > **Build Companion Radio (Lightweight)**
2. Select board, connection type (ble/usb/wifi/all)
3. Optionally specify an upstream tag
4. Run — firmware files appear as a downloadable artifact

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
