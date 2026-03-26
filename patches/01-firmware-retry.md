# Auto-Retry for DM and Group Messages

### Highlights

- **DM auto-retry** — firmware retries up to 200 times in the background, even after the app shows "Failed". Updates to "Delivered" when it finally goes through.
- **Group/channel auto-retry** — retries every 10 seconds with echo detection. Companion app shows "X repeats heard" when repeaters pick it up.
- **No app changes needed** — works with existing iOS, Flutter, and JS companion apps.
- **No network spam** — retries are paced, cancelled on delivery, and use standard mesh dedup.

---

## Overview

MeshCore firmware retries failed DMs up to approximately 5 times before marking the message as "Failed" in the companion app. For group/channel messages, there is no retry at all. This patch extends the retry mechanism to keep trying in the background — up to 200 times (configurable, will change in the future) — even after the app shows "Failed". When a retry eventually succeeds, the app updates the message status from "Failed" to "Delivered" (DMs) or shows "X repeats heard" (channel messages). No companion app changes required.

## Use Case

With stock MeshCore firmware, if you send a message while out of range of any repeater — for example while walking or driving with a T1000-E in your pocket — the message silently fails. You have to manually check the app, notice it didn't go through, and resend. This is especially frustrating with mobile nodes that move in and out of coverage.

With this patch, the firmware keeps retrying in the background. You send your message once, put your phone away, and whenever the node comes within range of a repeater or the recipient, the message goes through automatically. The app updates from "Failed" to "Delivered" (for DMs) or shows "X repeats heard" (for channel messages) without any manual intervention.

## How It Works

### Direct Messages (DM)

When a DM is sent and no ACK is received within the timeout window, the firmware automatically resends it with an incremented attempt counter. This continues until either:

- An ACK is received (message delivered)
- The maximum retry count is reached (default: 200)

The companion app may show the message as **"Failed"** after its own short timeout (typically 3-5 attempts), but the firmware continues retrying in the background for up to 200 attempts. When delivery eventually succeeds, the firmware sends `PUSH_CODE_SEND_CONFIRMED` (0x82) with the original ACK hash, and the app updates the message status from **"Failed"** to **"Delivered"**.

### Group/Channel Messages

Group messages use flood routing with no ACK mechanism. Instead, the firmware detects **echoes** — when a repeater re-broadcasts the message, the firmware receives a copy of its own packet with a matching hash.

Retries are sent every 10 seconds (configurable). Each retry appends a nonce byte after the null-terminated text to produce a different packet hash, which prevents mesh nodes from dropping the retry as a duplicate. The original timestamp is preserved so that the iOS companion app's `HeardRepeatsService` can match the echoed message and display **"X repeats heard"**.

When an echo is detected, retries are cancelled immediately. A 0x91 push frame with `is_final` set is sent to the companion app.

### Companion App Notifications (0x91)

During retries, the firmware sends `PUSH_CODE_RETRY_ATTEMPT` (0x91) frames over the companion protocol (BLE/USB/WiFi). These appear as "unhandled frame: 145" in current companion app debug logs, but serve as a visible indicator that retries are in progress.

Frame format (6 bytes):

| Byte | Content |
|------|---------|
| 0 | 0x91 (frame code) |
| 1 | Flags: bit 0 = group message, bit 7 = done retrying |
| 2-3 | Current retry count (uint16 LE) |
| 4-5 | Max retries (uint16 LE) |

The iOS app already has `MessageStatus.retrying` with "Retrying X/Y" UI built in — it just needs to add 0x91 to its `ResponseCode` enum to activate it.

## Network Impact

The retry mechanism is designed to avoid spamming the mesh network:

- **DM retries** only fire when the ACK timeout expires, which is calculated from the estimated airtime and path length. The firmware waits the full timeout before each retry — it doesn't flood the network with rapid resends.
- **Group retries** are spaced 10 seconds apart (configurable via `MESH_GROUP_RETRY_INTERVAL`), giving the network time to propagate each attempt.
- **Echo cancellation** stops group retries as soon as any repeater re-broadcasts the message. In practice, most group messages succeed within the first few retries once a repeater is in range.
- **ACK cancellation** stops DM retries immediately when any node acknowledges delivery.
- **Mesh deduplication** is handled at the packet level — each retry produces a unique packet hash (via attempt counter for DMs, nonce byte for groups), so mesh nodes forward it as a new message rather than dropping it as a duplicate.

## Configuration

All values are compile-time configurable via build flags:

| Flag | Default | Description |
|------|---------|-------------|
| `MESH_AUTO_RETRY_MAX` | 200 | Max retries for DMs (0 = disabled) |
| `MESH_AUTO_GROUP_RETRY_MAX` | 200 | Max retries for group messages (0 = disabled) |
| `MESH_GROUP_RETRY_INTERVAL` | 10000 | Milliseconds between group retries |

Example: `-DMESH_AUTO_RETRY_MAX=50 -DMESH_AUTO_GROUP_RETRY_MAX=100`

## Files Modified

| File | Changes |
|------|---------|
| `src/helpers/BaseChatMesh.h` | Retry structs, state fields, virtual methods, compile-time defaults |
| `src/helpers/BaseChatMesh.cpp` | DM retry loop, group retry loop, echo detection, ACK matching for retried messages |
| `examples/companion_radio/MyMesh.h` | Override declarations |
| `examples/companion_radio/MyMesh.cpp` | 0x91 push notifications, echo detection call-through, DM retry ACK confirmation |

## Technical Details

### DM Retry ACK Matching

Each DM retry produces a different ACK hash (because the attempt counter changes the packet content). When an ACK arrives for a retried message, `processAck()` won't find it in the `expected_ack_table` (which holds the original hash). The patch adds a fallback: `onRetryAckReceived()` looks up the original ACK hash by matching the recipient's public key, then sends `PUSH_CODE_SEND_CONFIRMED` (0x82) with that original hash so the app can correlate it.

### Group Retry Nonce (HeardRepeatsService Fix)

An earlier approach incremented the timestamp on each group retry to avoid mesh deduplication. This broke the iOS app's `HeardRepeatsService`, which matches echoed messages by timestamp within a 10-second window. After enough retries, the timestamp drifted past that window and echoes were no longer recognized.

The fix uses a retry nonce byte appended after the null-terminated text instead. This changes the encrypted payload (and thus the packet hash) without changing the timestamp. The same pattern is already used by DM retries in `composeMsgPacket()`.

### Echo Detection

`filterRecvFloodPacket()` compares incoming flood packet hashes against the last sent group message hash. On match, retries are cancelled and the companion app is notified. The packet continues through normal processing, where it arrives as an RX log entry (0x88) that `HeardRepeatsService` uses to count repeats.
