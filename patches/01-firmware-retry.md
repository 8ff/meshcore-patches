# Auto-Retry Patch for MeshCore

Extends MeshCore's message retry with gradual backoff, immediate cancellation on delivery, and live path updates — so messages eventually get through without spamming the network or draining the battery. Works with existing companion apps, no app changes needed.

---

## Why

Stock MeshCore gives up on messages after about 5 tries. If you're walking around with a T1000-E in your pocket and you're temporarily out of range, the message silently fails. You have to manually check, notice it didn't go through, and resend.

With this patch, the firmware keeps trying in the background with increasing intervals between attempts. It starts fast (in case you're barely out of range), then gradually backs off to conserve battery and airtime. Retries cancel immediately when the message is delivered or echoed by a repeater.

## How retries work

### DMs

When a DM doesn't get an ACK back, the firmware resends it with an incremented attempt counter. It keeps going until either an ACK comes back (delivered) or it hits 200 retries.

The retry interval uses **gradual backoff**:
- **Retries 1–5:** calculated ACK timeout (~4s) — fast phase for when you're barely out of range
- **Retries 6–20:** 3x slower (~13s) — you're probably genuinely out of range
- **Retries 21–200:** 6x slower (~27s) — long tail, conserves battery

This stretches the retry window to about **1.5 hours** instead of burning through all retries in 15 minutes.

The app might show "Failed" after its own short timeout, but the firmware keeps going. When it finally delivers, it sends a 0x82 push and the app updates to "Delivered".

**Live path lookup:** Before each retry, the firmware re-reads the contact's current path from the contacts table instead of using a stale snapshot. So if you reset a path in the app mid-retry, the next retry will use the updated route (or fall back to flood if the path was cleared).

### Groups / Channels

Group messages use flood routing — there's no ACK. Instead, the firmware listens for **echoes** (when a repeater re-broadcasts your message and you hear it back).

Backoff works the same way:
- **Retries 1–5:** 10s apart
- **Retries 6–20:** 30s apart
- **Retries 21–200:** 60s apart

Each retry appends a nonce byte after the text to produce a different packet hash so mesh nodes don't drop it as a duplicate. The original timestamp is preserved so the iOS app's `HeardRepeatsService` can still match echoes.

When an echo is detected, retries cancel immediately.

## 0x91 push frames

During retries, the firmware sends 0x91 frames over BLE/USB/WiFi. Current companion apps log these as "unhandled frame: 145" — they're a visible sign that retries are happening.

Format (6 bytes):

| Byte | Content |
|------|---------|
| 0 | 0x91 (frame code) |
| 1 | Flags: bit 0 = group, bit 7 = done |
| 2-3 | Current retry count (uint16 LE) |
| 4-5 | Max retries (uint16 LE) |

The iOS app already has `MessageStatus.retrying` with "Retrying X/Y" UI — it just needs 0x91 added to its `ResponseCode` enum.

## Network impact

- DM retries only fire after the ACK timeout expires — no rapid flooding
- Group retries are spaced 10s+ apart and back off over time
- Echo detection cancels group retries as soon as any repeater picks it up
- ACK cancellation stops DM retries immediately on delivery
- Each retry gets a unique packet hash so mesh dedup doesn't drop it

## Build flags

| Flag | Default | What it does |
|------|---------|-------------|
| `MESH_AUTO_RETRY_MAX` | 200 | Max DM retries (0 = off) |
| `MESH_AUTO_GROUP_RETRY_MAX` | 200 | Max group retries (0 = off) |
| `MESH_GROUP_RETRY_INTERVAL` | 10000 | Base ms between group retries |
| `MESH_RETRY_BACKOFF_PHASE1` | 5 | Fast retries before first slowdown |
| `MESH_RETRY_BACKOFF_PHASE2` | 20 | Medium retries before second slowdown |

Example: `-DMESH_AUTO_RETRY_MAX=50 -DMESH_AUTO_GROUP_RETRY_MAX=100`

## Files changed

| File | What |
|------|------|
| `BaseChatMesh.h` | Retry structs, state, virtual methods, defaults |
| `BaseChatMesh.cpp` | DM retry loop, group retry loop, echo detection, ACK matching, live path lookup |
| `MyMesh.h` | Override declarations |
| `MyMesh.cpp` | 0x91 push notifications, echo detection, DM retry ACK confirmation |

## Technical notes

**DM ACK matching** — Each retry produces a different ACK hash (because the attempt counter changes). When an ACK arrives for a retry, `processAck()` won't find it in the normal table. The patch adds a fallback that matches by recipient public key and sends 0x82 with the original hash so the app can correlate it.

**Group nonce vs timestamp** — An earlier approach incremented the timestamp on each retry to avoid dedup. This broke `HeardRepeatsService` which matches echoes within a 10-second window. The fix appends a nonce byte instead — changes the packet hash without touching the timestamp.

**Echo detection** — `filterRecvFloodPacket()` compares incoming flood hashes against the last sent group hash. On match, retries cancel and the app gets notified.

**Live path re-read** — Before each DM retry, the firmware calls `getContactByPublicKey()` to fetch the contact's current `out_path` and `out_path_len`. If you reset or update the path in the companion app between retries, the next retry uses the new route instead of the stale one from when the message was first sent.
