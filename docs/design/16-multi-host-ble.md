# Upstream #16 — Multi-host Bluetooth switching: design document (Phase 1)

- **Issue:** https://github.com/makersmakingchange/LipSync/issues/16 — "independently switch
  between multiple Bluetooth devices (e.g., a laptop, smartphone) … using the Hub interface,
  without having to access the host devices."
- **Status:** DESIGN — awaiting sign-off. **No firmware/app code is in this commit.**
- **Branch:** `feature/16-multi-host-ble`, cut from `firmware-release-pipeline` @ `377e4d1`
  (the V4.7.0 tag does not exist yet; tags end at V4.6.6). The release branch is untouched.
- **Doc location convention:** the firmware repo has no engineering-docs folder (only
  `Documentation/` for user-facing PDFs/DOCX and per-library `LIPSYNC.md` notes). This doc
  starts `docs/design/`, mirroring the app repo's `docs/` convention.
- **Citation shorthand:**
  - `FW/` = `Build_Files/Firmware_Files/LipSync_Firmware/`
  - `BL/` = `Build_Files/Firmware_Files/Libraries/Bluefruit52Lib/src/`
  - `APP/` = the LipSync_Connect repo (`~/lipsync_connect`), files under `app/js/` unless noted.
  - Every stack-capability claim cites `file:line` in the vendored lib, the app code, or CI.

---

## Executive summary

**Recommendation: GO** — peripheral-only bond switching, no central role, no new settings
file, no vendored-library fork in v1. The 4.6.6 deferral note's rationale ("requires BLE
central + HID client") is **wrong on the role model** and this document resolves the
disagreement with evidence: the LipSync is an HID *peripheral*; hosts are centrals; switching
hosts is "disconnect and let another known central reconnect", which the existing stack does
with **zero new SoftDevice capability**. The 2026-10-14 roadmap feasibility call was right.

The hard feasibility input (Q2) resolves better than feared: **this stack has no SoftDevice
bond table at all** — Bluefruit52Lib stores each bond as a LittleFS *file* under
`/adafruit/bond_prph/<peer-MAC>` (`BL/utility/bonding.h:41-45`), holding 80 B of keys plus the
peer's **name, cached at pair time** (`BL/utility/bonding.cpp:115-125`). Capacity is
filesystem-bound, not table-bound: 3, 4 or 6 hosts are all trivially safe, and the bond
filename *is* the peer MAC, so per-host Forget needs no library surgery.

The design adds: 6 serial codes on one new `BT` endpoint (verified free against the live
dispatch table), 3 Hub pages (new page id 62 family, appended before "… Back" per the 4.7.0
row-order pattern), one pure header + host tests (the `LSPressureClassify.h` pattern), and an
app "Hosts" surface on the existing Bluetooth card gated at firmware 4.8.0. **Zero bytes** are
added to either settings file — both frozen budgets are untouched, and host data is excluded
from app snapshot/restore/export *by construction* (nothing is added to their key lists).

The #1 open risk is **host reconnect behavior** — what macOS / Windows / iOS / Android do
when a bonded HID peripheral drops them and re-advertises. That is not answerable at the
desk; Q10 is the numbered bench that answers it. The v1 design keeps today's open-advertising
posture (no whitelist exists in the vendored lib — `BL/BLEAdvertising.cpp:350` hardcodes
`BLE_GAP_ADV_FP_ANY`), with an IRK-whitelist patch designed as the contingency if the bench
shows the just-disconnected host racing the target back onto the link.

Effort: the roadmap's "a week of code plus a week of bench" (APP repo `docs/roadmap.md:606`)
**remains credible** — Q2 removed the bond-capacity unknown and v1 needs no library changes;
+2–3 days if the bench forces the whitelist contingency.

---

## Q1. Role model — peripheral-only; central ruled out

**Decision: peripheral-only bond switching. Nothing in this feature uses the GAP central
role or the GATT client for anything beyond the existing name read.**

Evidence:

1. **The LipSync is an HID peripheral today.** `Bluefruit.begin()` is called with no args →
   `prph_count = 1, central_count = 0` (`FW/LSBLE.h:174`; defaults at `BL/bluefruit.h:140`:
   `bool begin(uint8_t prph_count = 1, uint8_t central_count = 0);`). The role config lands
   in `BLE_GAP_CFG_ROLE_COUNT` with `central_role_count = _central_count` and
   `central_sec_count = (_central_count ? 1 : 0)` (`BL/bluefruit.cpp:404-408`) → **0/0/…
   central is fully dormant**. The only GATT service is the HID server (`FW/LSBLE.h:36`
   `BLEHidAdafruit blehid;`) plus DIS.
2. **"Switching between hosts" never requires the central role.** Laptop/phone = GAP central;
   LipSync = GAP peripheral serving HID. Choosing *which* central the peripheral serves is a
   disconnect + advertise + accept-reconnect sequence — all peripheral-role operations.
3. **What `BLEClientHidAdafruit` is actually for** — the LipSync connecting *to* other BLE HID
   devices (receiving a keyboard/mouse's reports as a client). That is a different feature
   (using a third-party BLE input device), unrelated to #16. It is complete but dormant
   (`BL/clients/BLEClientHidAdafruit.h:46-103`), and enabling it costs real resources:
   `central_role_count=1` + `central_sec_count=1` (`BL/bluefruit.cpp:406-407`) plus the whole
   central CONN_CFG block (`BL/bluefruit.cpp:453-479`) — extra SoftDevice SRAM checked against
   the app-RAM ceiling at `sd_ble_enable` (`BL/bluefruit.cpp:484-493`). Zero benefit for #16.
4. **The 4.6.6 posture note's "requires BLE central + HID client" is resolved as a
   category error**: it bundled *everything hard about BLE* into the deferral rationale.
   The concrete scenario that would need central ("LipSync connects to a phone") is not in
   #16's ask — #16 is about the LipSync being *served by* either of two hosts.
5. **Does the HID service require bonding/encryption?** Yes for reports, no for pairing:
   - HID input-report characteristics carry
     `setPermission(SECMODE_ENC_NO_MITM, SECMODE_NO_ACCESS)` (`BL/services/BLEHidGeneric.cpp:171`)
     — the read side (which the CCCD write permission inherits, `BL/BLECharacteristic.cpp:289-297`)
     requires an **encrypted, no-MITM** link. So notification subscription requires
     encryption; a host that wants reports must pair first.
   - Pairing is **auto-accepted Just-Works bonding** for any central:
     defaults `.bond=1, .mitm=0, .io_caps=NONE` (`BL/BLESecurity.cpp:37-49`), and the
     `BLE_GAP_EVT_SEC_PARAMS_REQUEST` handler replies `BLE_GAP_SEC_STATUS_SUCCESS`
     unconditionally — there is no accept/reject gate (`BL/BLESecurity.cpp:273-309`).
   - On reconnect, encryption is *central-initiated*; the peripheral answers
     `BLE_GAP_EVT_SEC_INFO_REQUEST` from stored keys (`BL/BLESecurity.cpp:402-425`) and the
     CCCD is restored on `CONN_SEC_UPDATE` (`BL/BLEConnection.cpp:311-323`).
   - **"Unknown central" policy consequence:** because pairing auto-accepts, a *stranger*
     can pair with the device today (and reports need only encryption, which Just-Works
     grants). This feature does **not** change that posture; tightening it (per-bond
     acceptance gate) is a vendored-lib change explicitly out of v1 scope (see Q8/R-sec).

---

## Q2. Bond capacity — the hard feasibility input

**Decision: capacity is a non-issue. There is no SoftDevice bond table in this stack — bonds
are files. N=3/4/6 are all safe; the design displays 4 and imposes no hard cap.**

Evidence:

1. **No peer manager, no SD bond store.** Greps for `peer_manager|pm_` over `BL/` → no
   matches; no `sd_ble_cfg_set` call configures bonds (all 13 enumerated in
   `BL/bluefruit.cpp:399-478` — VS-UUID count, role counts, service-changed, attr table
   (0x1000 on nRF52840, `BL/bluefruit.h:40-44`), per-role MTU/event-length/queues). The
   SoftDevice headers themselves are not in-repo — the core is pinned at build time
   (`.github/workflows/build_firmware.yml:91`: `Seeeduino:nrf52@1.1.13`), and the S140
   **6.1.1** identification comes from the bench-pinned in-repo notes
   (`FW/LSBLE.h:130-134`, `Libraries/Wire/README.md:58-60`).
2. **The bond store is LittleFS files.** `bond_init()` (called at the end of `begin()`,
   `BL/bluefruit.cpp:534`) mounts InternalFS and creates `/adafruit/bond_prph` and
   `/adafruit/bond_cntr` (`BL/utility/bonding.cpp:86-93`; paths at `bonding.h:41-45`). One
   file per bonded peer, **named by the peer MAC** (`bonding.cpp:100-104`). File layout:
   length-prefixed 80-byte `bond_keys_t` (own LTK + peer LTK + peer identity/IRK,
   `bonding.h:47-53`) + length-prefixed **device name** + length-prefixed CCCD blob
   (`bonding.cpp:55-61`). Re-bonding the same peer overwrites its file
   (`bonding.cpp:106-107`).
3. **Overflow behavior:** there is no bond-table `NRF_ERROR_NO_MEM` path to hit — no capacity
   constant exists anywhere in the repo, and the only `NRF_ERROR_NO_MEM` in the lib is
   attribute-table allocation (`BL/services/BLEHidGeneric.cpp:157`). A bond save that fails
   (filesystem full) fails silently in the deferred callback (`bond_save_keys` →
   `ada_callback`, `bonding.cpp:132-136`; `VERIFY` inside `_dfr`) — unreachable at our scale
   (a bond file is ~150–200 B; InternalFS holds orders of magnitude more); bench step B9
   prints real sizes via `bond_print_list` (`bonding.cpp:297-325`) to close this honestly.
4. **Bond management API surface that exists today:**
   - per-bond delete: `bond_remove_key(role, id_addr)` = `InternalFS.remove(filename)`
     (`bonding.cpp:359-367`) — callable **without a live connection** since the filename is
     the MAC; connection-tied wrapper `BLEConnection::removeBondKey()`
     (`BL/BLEConnection.cpp:273-278`); bulk `bond_clear_prph/_all`
     (`bonding.cpp:330-357`, exposed as `Bluefruit.Periph.clearBonds()`, `BL/BLEPeriph.cpp:81-84`).
   - enumeration: **no programmatic API** (only the printing `bond_print_list`) — a Phase-2
     app-side enumerator reads the documented on-disk layout directly (paths and field
     framing are public: `bonding.h:41-45`, `bonding.cpp:55-84`). **No library fork needed.**
   - IRK access: `bond_keys_t.peer_id.id_info` (`bonding.h:52`) + HW-AES resolver
     `BLESecurity::resolveAddress` (`BL/BLESecurity.cpp:160-185`); RPA hosts are matched by
     iterating all bond files (`bonding.cpp:170-195`) — **known-host reconnect works for
     rotating addresses with any number of bonds.**
   - gaps (no lib support, and none needed for v1): no whitelist API anywhere
     (`grep whitelist` over `BL/` → none), no directed-adv helper
     (`BL/BLEAdvertising.cpp:344` hardcodes `.p_peer_addr = NULL`, `:350` hardcodes
     `BLE_GAP_ADV_FP_ANY`), no enumerate-all-bonds-with-keys helper.

---

## Q3. Slot model & storage — the bond directory IS the host table

**Decision: no new settings file, no new settings keys, no slot map. The four questions the
task poses resolve as: slots = enumerated bonds (MAC-sorted, deterministic); host labels =
already persisted by the lib; active slot = runtime truth; last-used = not persisted.**

1. **Host labels are already stored.** At pair time the lib reads the peer's GAP name
   (`BLEConnection::getPeerName` = GATT read-by-UUID of 0x2A00, `BL/BLEConnection.cpp:138-141`
   via `BL/BLEGatt.cpp:48-77`, 500 ms cap, 0 on failure; works as GATT *client procedure over
   the peripheral connection* — no GAP central role involved) and caches it **inside the bond
   file** (`bonding.cpp:115-125`), capped at 32 bytes (`CFG_MAX_DEVNAME_LEN`,
   `BL/bluefruit_common.h:45`), with the formatted MAC as fallback when the name read fails
   (`bonding.cpp:119-123`). Limits to respect: the name is captured once at pair time (a host
   later renamed keeps its old label until re-bond), and some hosts can refuse the 0x2A00
   read (then we show the MAC form).
2. **Why no slot map / no `/hosts.dat`:** a slot map would duplicate state that can drift
   (a host that unpairs *itself* leaves our map pointing at a ghost), and "active host" is
   derivable at runtime (`Bluefruit.connected()` / `connHandle()`, `BL/bluefruit.cpp:639-648`,
   `:689-692`). Persisting *last-used* would mean a flash write on every connection for zero
   UX gain — row order is stable by construction (MAC sort), not by recency. This kills an
   entire drift class and keeps **both frozen budgets untouched**:
   - `/settings.txt` 480-byte 4.1.0-downgrade cliff (`FW/LSConfig.h:198-217`) — **+0 bytes**.
   - `/settings2.dat` 256-byte budget (`FW/LSConfig.h:230-232`) — **+0 bytes**.
   Byte-accounting table: no rows — the feature's persistent state is exactly the bond files
   the lib already manages.
3. **Downgrade/reset semantics:** 4.7.0-and-older firmware never reads bond files for UX
   (a downgraded device keeps bonding exactly as today — single-host behavior is unchanged;
   bonds are neither created nor destroyed by the downgrade). Factory reset: **see Q8** —
   today's FR already wipes bonds as a side effect of `InternalFS.format()`, and the design
   keeps that, made explicit.
4. **Display cap:** the Hub page shows at most 4 host rows (OLED height); the app lists all.
   A 5th bond still functions (Q8).

---

## Q4. Switching mechanism — exact sequence

**Decision: "switch to host i" = disconnect the current host; open advertising resumes
automatically; the target host reconnects on its own schedule. No whitelist, no directed
advertising in v1.**

Sequence (firmware):

1. **Preconditions** (all validated before touching the radio; refusal shape in Q6):
   BLE initialized (`needsInitialization == false`, the guarded-call-site rule,
   `FW/LSBLE.h:95-97, 257`); not mid-calibration/center; not holding a drag/scroll latch;
   target row valid; target ≠ current host.
2. **Disconnect:** `BLEConnection::disconnect()` (`BL/BLEConnection.h:95`). Existing safety
   nets already fire: the drag/scroll latch is released on disconnect
   (`FW/LipSync_Firmware.ino:4557-4559` — "Never carry a drag/scroll latch across hosts") and
   `LSBLEMouse::service()` clears pending click/button state on disconnect
   (`FW/LSBLE.h:320-334`).
3. **Advertising resumes by itself:** `restartOnDisconnect(true)` is already set
   (`FW/LSBLE.h:246`); the advertising handler re-starts on disconnect when no peripheral is
   connected (`BL/BLEAdvertising.cpp:419-427`), with the same `start(0)` = advertise-forever
   timeout the boot path uses (`FW/LSBLE.h:249`). Intervals stay 32/244 with the 30 s
   fast→slow fallback (`FW/LSBLE.h:247-248`).
4. **The target host reconnects.** Advertising is **open**: `.p_peer_addr = NULL` and
   `filter_policy = BLE_GAP_ADV_FP_ANY` are hardcoded (`BL/BLEAdvertising.cpp:344, 350`);
   the vendored lib exposes **no whitelist and no directed advertising** (grep evidence in
   Q2 item 4). Any bonded central reconnects via the stored-LTK path (Q1 item 5); an RPA
   host is resolved by IRK across all bond files (`bonding.cpp:170-195`).
5. **Feedback:** OLED shows a bounded dwell page "Switching to <name>" (the
   `CHANGING_MODE_PAGE` pattern, `FW/LSScreen.h:53`), then returns to the host list. The
   micro LED keeps its existing, already-correct semantics: blink blue while advertising,
   solid blue on reconnect, red only for init failure
   (`FW/LipSync_Firmware.ino:4500-4502, 4548, 4563-4568`). No new LED states.
6. **Per-connection CCCD restoration** (each host's notification subscriptions) is already
   per-bond (`BL/BLEConnection.cpp:311-323`, `bonding.cpp:262-295`).

**Contingency (only if the bench shows the just-disconnected host racing the target back onto
the link, bench step B4):** a small vendored-lib patch — house precedent is the bounded
`Wire`/LPS2X/LPS35HW forks documented in `LIPSYNC.md` files — adding
`sd_ble_gap_whitelist_set` + `BLE_GAP_ADV_FP_WHITELIST*` filter policy on a dedicated
"switch advertise" call. The data already exists: identity addresses + IRKs in the bond
files (`bond_keys_t.peer_id`, `bonding.h:52`; S140's whitelist resolves RPAs against IRKs).
Estimated +2–3 days (code + bench round 2). **Not in v1.**

**Honest host-side UX (what the user must do per OS) — desk expectations, bench-verified in
Q10.** After the LipSync disconnects and re-advertises:

| Host | Expected reconnect behavior | User action if it doesn't |
|---|---|---|
| macOS | HID reconnects aggressively when the device advertises | Click the device in Bluetooth settings; worst case remove + re-pair |
| Windows | Usually reconnects on advertise; historically the flakiest (service-triggered reconnect) | Toggle the device in Settings → Bluetooth; worst case remove + re-pair |
| iOS/iPadOS | Aggressive auto-reconnect for HID | Open Settings → Bluetooth and tap the device |
| Android | OEM-dependent auto-reconnect | Settings → Bluetooth → tap the device |

**The asymmetry that must be documented (not coded):** when the device *forgets* a bond but
the host still holds its keys, the host's next connect starts encryption, the device answers
`SEC_INFO_REQUEST` with NULL (`BL/BLESecurity.cpp:420-423`), encryption fails, and well-behaved
hosts re-initiate pairing (Just Works) — a **new** bond file appears. Bench step B6 verifies
this self-heal per OS; the card/help text tells users a "remove the device on the host" step
exists for the OSes that don't self-heal.

---

## Q5. Hub menu UX

**Decision: a new "Bluetooth" page in the More menu, built exactly like the 4.7.0 hard pages
(append before "… Back"; pre-existing row indices unchanged).**

1. **Placement:** More menu gains row 10 "Bluetooth"; "… Back" moves to row 11;
   `_moreMenuLen` 11 → 12 (`FW/LSScreen.h:360`, `moreMenu()` at `:2173-2183`; dispatch case
   `MORE_MENU` at `:1150-1188`). This is the 4.7.0 row-order pattern (hard rows appended in
   front of Back so earlier rows keep their indices — `FW/LSScreen.h:74-80`, `:1276-1281`).
2. **Page ids** (existing numbering: main 0-6, dwell 11/21/31-33, More family 51-61 with
   54x/55x/57x subpages, `FW/LSScreen.h:35-90`): `BT_HOSTS_MENU 62`,
   `BT_SWITCH_CONFIRM_PAGE 621`, `BT_FORGET_CONFIRM_PAGE 622`, dwell
   `BT_SWITCHING_PAGE 623` (names itself for `CH,0:0`/`CH,5` honesty, like
   `CHANGING_MODE_PAGE 33`, `FW/LSScreen.h:46-55`).
3. **Rows:** for each enumerated bond (max 4): `<name truncated to 17> (<last MAC byte XX>)`
   — the suffix disambiguates two identically-named hosts ("iPhone (3F)"). Then
   "Pair new…", then "… Back". Row count varies with bond count; the page's
   `_currentMenuLength` is set from the enumeration (the app mirror learns the count from
   `BT,0`).
4. **Actions:**
   - Host row → confirm page 621 ("Switch to <name>?") → execute Q4 sequence → dwell 623 →
     back to 62. Confirm (not silent) because the switch drops a live host — same class as
     `CONFIRM_MODE_CHANGE 31`.
   - "Forget" lives *on* each row? No — a row's long-press concept doesn't exist on this
     menu. **Design:** a separate toggle row "Forget host…" at the bottom of the host list
     (only when ≥1 bond exists) → page 622 lists hosts → select → the **hold-to-confirm
     gate** (`selectMenuItem`'s `destructiveSelection` machinery, `FW/LSScreen.h:1007-1017`)
     + the existing confirm-hold floor (`FW/LSConfig.h:328-338`).
   - "Pair new…" → refuse if connected (error cue + note "disconnect first"), otherwise
     ensure advertising and show a dwell page "Pairing: pair from <host's Bluetooth
     settings>". No radio change is needed beyond what's already running.
5. **Refusal paths** (each = error cue + named note, never a silent no-op):
   - **Drag/scroll latch held:** refuse switch/forget until release (the latch means a host
     button is down; dropping the host mid-hold is exactly what
     `FW/LipSync_Firmware.ino:4557-4559` cleans up *unintentionally* — we refuse
     *deliberately* instead).
   - **Calibration/center in flight:** menu input is already frozen for those windows
     (`inputIsFrozen()` gates `selectMenuItem`, `FW/LSScreen.h:997`); the serial path adds
     its own explicit check (Q6) because `CH,1` bypasses holds
     (`FW/LipSync_Firmware.ino:3558`) but must not bypass calibration.
   - **BLE not initialized** (USB-mode boot, `needsInitialization` true, `FW/LSBLE.h:37`):
     the page renders "Bluetooth not active" and every action refuses.
   - **Mode lock (LK): does NOT gate host switching** — LK's documented contract is "the
     operating mode is locked; inputs cannot change it" (`FW/LSConfig.h:239`), enforced on
     the Mode page rows (`FW/LSScreen.h:1067-1081`). Switching hosts changes neither OM nor
     CM (Q4 — no reset, no `storeModePair`). Flagged for reviewer objection: if review wants
     LK to gate switching too, it is a one-condition addition to the same gate.
   - **Update mode (UM):** unreachable state — UM resets into the bootloader
     (`FW/LSApiDeclarations.h:237-240`); no guard needed.
6. **During the handover** the OLED shows dwell 623 ("Switching to <name>", ~2 s, the
   `CHANGING_MODE_PAGE` dwell), so `CH,0:0`/`CH,5` always name the page actually shown
   (the 4.5.7 dwell-page honesty rule, `FW/LSScreen.h:46-55`).

---

## Q6. Serial API — the `BT` endpoint

**`BT` is free** — verified against the live dispatch table (`FW/LSAPI.ino:33-124`); `BH`/`HB`
(bus health / hard-puff action) untouched. Grammar per house rules: `[A-Z]{1,4}` + one-digit
code; responses through the canonical formatter (`FW/LSAPI.ino:4154-4179`) so SUCCESS/MANUAL
semantics are inherited, never hand-built (the `LSApiDeclarations.h:242-246` rule). All new
declarations go in `FW/LSApiDeclarations.h` in the same commit (house rule from the task).

| Command | Meaning | Response |
|---|---|---|
| `BT,0:0` | Host count | `SUCCESS,0:BT,0:<n>` |
| `BT,0:<i>` | Host row i (1-based, the enumeration order of Q3) | `SUCCESS,0:BT,0:<i>:<name>:<state>` — state `1` = this host is connected/active, `0` = known |
| `BT,1:<i>` | Switch to row i | `SUCCESS,0:BT,1:<i>` then the Q4 sequence; `MANUAL,0:BT,5:...` follows on the transitions |
| `BT,2:<i>` | Forget row i (removes that bond file) | `SUCCESS,0:BT,2:<i>`; if i was active, disconnect happens first; `MANUAL,0:BT,4:<n>` follows |
| `BT,3:1` | Pair-new: ensure advertising (refuse while connected) | `SUCCESS,0:BT,3:1` |
| `BT,4:<n>` | *(push only)* host list changed (pair/forget) — n = new count | `MANUAL,0:BT,4:<n>` |
| `BT,5:<i>` | *(push only)* connection changed — i = active row (1-based) or 0 = none | `MANUAL,0:BT,5:<i>` |

Validation & refusal shapes (FAIL numbers per the parser: `FAIL,1` unknown endpoint,
`FAIL,2` malformed parameter, `FAIL,3` refused/out-of-range — `FW/LSAPI.ino:389,397,403`;
semantic refusals use 3 as the CM-while-gamepad precedent does, `FW/LSAPI.ino:2517`):

- `BT,1` refuses (each `FAIL,3`, plus a named `ERROR:` line when the reason isn't obvious
  from the code — the budget-refusal precedent, `FW/LSMemory.h:341-352`): row out of range;
  target == current host; drag/scroll latch held; calibration/center running;
  `needsInitialization` true; menu input frozen (dwell window).
- `BT,2` refuses: row out of range; calibration/center running; `needsInitialization` true.
  Forgetting the *active* host is allowed (confirm page on-device; app-side confirm) — it
  disconnects first; the host re-pairs or is removed per Q4's asymmetry note.
- `BT,3` refuses while connected (`FAIL,3` — a new pairing needs advertising).
- **Name field sanitization (one rule, host-tested + cross-checked):** the name printed on
  the wire has `,`, `:`, `/` and non-printables replaced with `_`, and is truncated to 24
  chars. Rationale: the response grammar is comma/colon-delimited
  (`APP/dev/firmware/…` protocol regex `[A-Z]{1,4},\d+` with a free-form tail — but the OLED
  and the app both benefit from one deterministic rule).
- **No reset, ever:** switching/forgetting never touches OM/CM/MP — the deliberate-reset
  mode-switch contract (`FW/LSConfig.h:117-125`, `FW/LSAPI.ino:2602-2632`) is untouched;
  no `storeModePair`, no `softwareReset`.
- **MANUAL push behavior for app sync:** `BT,4`/`BT,5` are emitted with the CH,5 pattern —
  change-detected, `printResponseIntArray(true, false, …)` = `MANUAL` + seq 0, matching no
  command waiter (`FW/LSScreen.h:2734-2774`; the 4.5.2 initiator contract). The app's
  updates-map entry (Q7) is what turns them into live UI; **the past bug class — an endpoint
  present in the load plan but missing from `device.js`'s `updates` map silently drops every
  push — is explicitly tested for** (`APP/app/js/serial/device.js:213-227` documents the
  shape; new test asserts BT pushes land).
- **Version gate: firmware 4.8.0** (next minor after 4.7.0; `CONF_LIPSYNC_VERSION_*` at
  `FW/LSConfig.h:31-33` bumps in Phase 2 step f). The app gates on
  `BT_HOSTS_MIN_FIRMWARE = [4, 8, 0]` (declared in `APP/app/js/config/constants.js` beside
  `HARD_BAND_MIN_FIRMWARE = [4, 7, 0]`, `constants.js:205-206`).

---

## Q7. App surface (LipSync Connect)

**Placement: the existing Bluetooth card** (`APP/app/index.html:1825-1869`, section
`sec-bluetooth`, Device area; card logic lives in `APP/app/js/services/mode-switch.js`).
That card already carries the honest comment that *"CM=2 proves the selected transport, not
the live radio state … do not claim 'advertising' without a firmware runtime-status
endpoint"* (`mode-switch.js:52-60`) — `BT,0`/`BT,5` **is** that endpoint; the feature closes
a documented honesty gap.

Interactions (all inside `sec-bluetooth`, gated `BT_HOSTS_MIN_FIRMWARE`):

- **Hosts list:** one row per bond from `BT,0` (name + active marker + Switch / Forget
  buttons). Switch → confirm → `BT,1:<i>`; Forget → confirm → `BT,2:<i>`. Live status via
  `MANUAL,0:BT,5` pushes; list refresh via `MANUAL,0:BT,4` → re-read `BT,0`.
- **Pair new:** button enabled only when disconnected (mirrors the firmware refusal); runs
  `BT,3:1` and shows the per-OS "pair from your host's Bluetooth settings" steps already
  present in the card (`index.html:1856-1868`).
- **Older firmware:** the whole list renders as not-applicable with the hard-band note
  pattern — readouts `—`, controls disabled, honest "needs firmware 4.8.0+" note with the
  Updates link (`markHardBandUnavailable`, `APP/…/settings-service.js:474-498`).
- **Snapshot/restore + settings-export: EXCLUDED, by construction.** Host bonds are
  per-device cryptographic identity — restoring "which computers were bonded" onto another
  device (or after an FR) is meaningless, and the app's include-by-explicit-list design
  means host data is excluded simply by **not adding anything** to `SNAPSHOT_KEYS`
  (`APP/…/firmware-update.js:508-517`), `EXPORTED_VALUES`
  (`APP/…/settings-export.js:18-42`), or the restore `order` array
  (`firmware-update.js:716-740`). This decision is pinned by a cross-check assertion so a
  future edit can't accidentally add BT keys.
- **Hub remote mirror:** firmware page 62 joins `FW_PAGE_TO_MIRROR`
  (`APP/app/js/services/hub-remote.js:72-101`) with the gated rows spread (the 4.7.0
  pattern, `hub-remote.js:429-452`): confirm pages 621/622 and dwell 623 map as transient
  pages (`FW_TRANSIENT_PAGE_MS`, `hub-remote.js:108-112`).
- **Diagnostics/logging:** all BT exchanges already land in the log through the standard
  TX/RX paths (`APP/app/js/serial/commands.js:171`, `device.js:239`); switch/forget actions
  log one info line each; nothing new is added to the support report beyond what the log
  already captures.
- **Version gate:** `BT_HOSTS_MIN_FIRMWARE = [4, 8, 0]` in `constants.js`; the
  firmware-compat tier map gains the 4.8.0 tier
  (`APP/tests/firmware-compat.test.mjs:170-218` pattern).

---

## Q8. Failure modes & destructive semantics

1. **Host never reconnects:** advertising continues forever (`start(0)`, `FW/LSBLE.h:249`);
   LED blinks blue (existing); the Hub page and the app both remain reachable over USB, and
   the user can switch back or pair a different host. The per-OS table (Q4) ships as help
   text.
2. **"Bond full":** there is no hard cap (Q2). The Hub page shows 4 rows + a "+N more" note
   (N reachable via the app, which lists all). Forget is the pressure valve. Pairing a 5th
   host simply works — no refusal, no vendored-lib pairing gate.
3. **Power loss mid-switch:** the operation is stateless — bonds are files written by the
   lib's deferred callback; a crash between disconnect and re-advertise leaves the next
   boot's normal path (advertise, `FW/LSBLE.h:249`). No new persistent state exists to
   corrupt. The CCCD writer's compare-before-write (`bonding.cpp:228-251`) bounds wear.
4. **Factory reset vs bonds — what FR does TODAY (checked, not assumed):**
   `doFactoryReset` → `resetMemory()` → `mem.format()` → `InternalFS.format()`
   (`FW/LSAPI.ino:3986-4016` → `FW/LipSync_Firmware.ino:1536-1551` → `FW/LSMemory.h:247-251`)
   — **the whole LittleFS volume is formatted, so FR already erases every bond file today**
   (and `bond_init` recreates the empty dirs at next begin, `BL/bluefruit.cpp:534`).
   **Intended answer = keep this, make it explicit:** the FR confirm page and app copy gain
   "erases paired hosts", the CHANGES entry documents it, and the cross-check pins a comment
   referencing `resetMemory`. (This incidentally resolves the settings2 orphan trade's
   sibling: there is no third file to orphan.)
5. **`needsInitialization`:** every BT endpoint and every menu action gates on it (the
   guarded-call-site rule, `FW/LSBLE.h:95-97, 274-279, 391-401`). No BT code path can touch
   a dead SoftDevice.
6. **BLE verdict file (`pst=`/`prc=`) + SD-flag race machinery:** the feature adds **zero
   retention-register writes** and runs entirely on the loop task *after* the init task's
   verdict (`g_bleInitDone`/`needsInitialization`, `FW/LipSync_Firmware.ino:562-572`);
   `g_bleInitWindow` / `g_softDeviceEnabled` ordering is untouched
   (`FW/LSBLE.h:166-197`). The verdict stages (0-5, `FW/LSBLE.h:86-258`) are unaffected.
7. **Deliberate-reset mode-switch contract:** switching hosts never resets and never writes
   OM/CM (Q6); `MP` semantics untouched.
8. **Stale bond after host-side unpair:** the NULL-reply path (`BL/BLESecurity.cpp:420-423`)
   forces re-pairing; a failed-to-secure bond on the *central* side self-cleans
   (`BL/BLECentral.cpp:156-163`) — not our role, cited for completeness. Bench B6 verifies
   the peripheral-side self-heal per OS.

---

## Q9. State-machine factoring + test plan

**Pure factored header — `FW/LSPressureClassify.h` pattern** (no Arduino types, contract
comment block, host-tested): `LSHostSwitch.h` containing:

- the row model: enumeration order (MAC sort), display truncation, the wire-name
  sanitization rule (Q6);
- `switchDecision(...)` / `forgetDecision(...)`: pure functions over a small state struct
  (`bleInitialized, calibrationRunning, latchHeld, rowCount, targetRow, activeRow`) →
  `ALLOW` or a refusal reason enum. The firmware shell maps reasons to `FAIL,3` + notes;
  the menu shell maps them to cues/pages.

Host-test matrix (into `host_tests/core_host_tests.cpp`, the CI ASan/UBSan step,
`.github/workflows/build_firmware.yml:114-131`): fresh/boundary rows; target==active;
latch held; calibration; uninitialized BLE; name sanitize (`,`/`:`/`/`/non-printable/
24-char cap); MAC-sort order stability across a forget (rows renumber — pinned so the app's
re-read contract is explicit); overflow display counting.

App node tests:

- new `tests/bt-hosts.test.mjs` (the `hard-band.test.mjs` shape): gate off < 4.8.0 / on at
  4.8.0; load plan reads; `BT,1` switch flow incl. refusal echo; `BT,2` forget →
  `BT,4` push → re-read; `BT,5` connect/disconnect pushes updating the card; **the
  updates-map regression** (BT in load plan ⇒ BT in `updates`, else the sim's push is
  dropped and the test fails — the device.js:213-227 bug class);
- extend `tests/hub-remote.test.mjs`'s stateful sim (`createMenuFirmwareSim`,
  `hub-remote.test.mjs:431-567`): page 62 in `LEN`, confirm/dwell page behavior, row
  renumber after a device-side forget mid-mirror;
- extend `tests/firmware-compat.test.mjs` (`GATED_READS` 94-113, tier map 170-218) with the
  4.8.0 tier.

Browser audit: extend `dev/browser-audit/fake-firmware.mjs` (`respond()` 105-200 +
`currentEmit` 33-35) with the BT endpoints and device-side pushes; new `note()` areas
(`bt-hosts-list`, `bt-switch-refused-old-fw`, `bt-push-live-status`, …) in
`connected-audit.mjs` (primitive at `:76-80`); the card lives in an existing section so
`GROUP_BY_SECTION` (`:33-50`) likely needs no new group.

**Cross-check additions** (new numbered section before the summary,
`APP/dev/firmware/protocol-cross-check.mjs:55-486` pattern):
`BT,0/1/2/3` exist in `_functionList` and match app-sent commands; the app gate equals the
compiled firmware's next version; page ids 62/621/622/623 mapped + gated in app source
(section [5]'s hard-page pattern, `:279-289`); the More-menu row order pin
("Bluetooth" before "… Back", `_moreMenuLen` = 12 — the section [5] row-order pattern,
`EXPECTED_SIP_ROWS` at `:265-277`); the name-sanitization rule appears identically in
firmware header + app renderer; `BT,4`/`BT,5` push shapes parsed by the app's grammar;
snapshot/export exclusion pinned (no `BT`-family keys in `SNAPSHOT_KEYS`/`EXPORTED_VALUES`).

---

## Q10. Bench plan — the deciding instrument

**Matrix:** macOS (this Mac — available), Windows (bench PC used in prior rounds —
available), iOS/iPadOS (**needs an iPhone/iPad — access to be confirmed**), Android
(**needs a phone — STATUS.md's bench part G is already waiting on the same device**,
`APP/docs/STATUS.md:118-122`). iOS/Android steps run when hardware lands; the firmware
gates (B1-B3, B8-B10) can run on macOS+Windows alone.

**Procedure (numbered; PASS criterion per step; ~6-8 h + margin):**

- **B1 Baseline single-host (each OS):** fresh pair; Hub shows the host name; `BT,0` row
  state 1 while connected. *PASS:* name matches the host's Bluetooth name (or honest MAC
  fallback), state correct.
- **B2 Switch away:** with host A connected, `BT,1:<rowB>` (B pre-paired). Observe: A
  disconnects; advertising resumes (LED blink-blue); B reconnects. Measure time-to-cursor
  per OS. *PASS:* B connected within 60 s with **no host-side interaction**; A's cursor
  dead the moment B takes over.
- **B3 Switch back:** same in reverse. *PASS:* symmetric.
- **B4 Wrong-host race (the contingency trigger):** A and B both in range; switch from A.
  Does A grab the link back before B? Repeat 10×. *PASS:* target wins ≥ 9/10, else the
  whitelist contingency (Q4) is scheduled.
- **B5 Hub-menu flow:** the full Q5 flow on-device (rows, confirm pages, refusals: switch
  during a held drag, during center-reset). *PASS:* every refusal = error cue + named note;
  no silent no-op; `CH,0:0` never lies during dwell pages.
- **B6 Forget + self-heal:** `BT,2` a *connected* host; does it re-pair without user action?
  Repeat per OS, including RPA hosts (iOS/Android). *PASS:* documented behavior per OS; no
  wedge; bond file recreated on re-pair.
- **B7 Host-side forget:** unpair from the *host* while bonded. *PASS:* device tolerates
  (open advertising; re-pair works); no stale-row ghost after `BT,0` re-read.
- **B8 Overflow:** pair a 5th host. *PASS:* 5th works; page shows 4 + "+1 more"; app lists 5.
- **B9 Storage truth:** `bond_print_list`-equivalent sizes after 6 bonds + repeated switches.
  *PASS:* files present, sizes sane, CCCD updates bounded (no growth per switch).
- **B10 Power-loss mid-switch:** cut power during the dwell page; reboot. *PASS:* normal
  boot, advertising, bonds intact; verdict file unchanged in class (`st=5` path).
- **B11 Old-firmware downgrade round-trip:** 4.8.0 → 4.7.0 → 4.8.0 with 3 bonds. *PASS:*
  downgrade keeps bonds + settings budgets byte-identical (`EX,0` diff clean); re-upgrade
  shows all hosts.
- **B12 Duration estimate:** B1-B3 ≈ 1.5 h per OS; B4-B10 ≈ 3 h total; B11 ≈ 1 h.

---

## Q11. Risk register + effort estimate

| # | Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|---|
| R1 | Host reconnect policies differ (Windows flakiest) — the #1 unknown | High | High | Bench B2/B3 decides; fallback UX per OS (Q4 table) ships regardless |
| R2 | Just-disconnected host races target back onto the link | Medium | High | Bench B4; IRK-whitelist lib patch designed as contingency (+2-3 d) |
| R3 | `getPeerName` fails at pair time → MAC-only labels | Low-Med | Low | Already the lib's fallback (`bonding.cpp:119-123`); re-bond refreshes |
| R4 | Ambiguous duplicate names on the 4-row OLED | Medium | Low | "(XX)" MAC-suffix discriminator (Q5) |
| R5 | Flash wear from CCCD writes on every switch | Low | Low | Compare-before-write (`bonding.cpp:228-251`); human switch rates are tiny |
| R6 | Silent bond-save failure on FS-full | Very low | Medium | Unreachable at N≤8 (Q2); bench B9 prints sizes |
| R7 | Menu refusal TOCTOU (the 4.6.6 LK class) | Low | Medium | Preconditions re-checked at confirm time, mirroring the 4.6.6 fix (`FW/LSScreen.h:1067-1081`) |
| R8 | Stranger-pairing posture (Q1 item 5) — unchanged by this feature | n/a | n/a | Explicitly out of scope; noted for a future hardening issue |

**Effort:** code ≈ 1 week — pure header + host tests (1-2 d), BT endpoints + enumerator +
pushes (1-2 d), 3 Hub pages + refusals (1-2 d), app card + gating + tests + cross-check +
audit (2 d), CHANGES/version/ZIP/CI (0.5 d). Bench ≈ 1 week (Q10). **The roadmap's estimate
(APP `docs/roadmap.md:606`) stands**; Q2 removed the capacity unknown and v1 needs no
vendored-lib changes. Whitelist contingency: +2-3 d.

---

## Recommendation

**GO**, staged exactly as the task's Phase 2 (a-f), with B4 as the explicit
go/no-go checkpoint mid-bench: if the wrong-host race fails repeatedly, the design absorbs
it via the whitelist contingency without changing the wire contract or the menu model.

## Answered at the desk vs. needs the bench

**Answered at the desk (cited):** role model (Q1); bond capacity + storage model (Q2, Q3);
switch sequence + advertising behavior (Q4); page ids/rows/refusals (Q5); wire contract +
FAIL semantics (Q6); app placement/gating/exclusions (Q7); FR-today behavior, verdict/SD-race
interplay, overflow, power-loss (Q8); factoring + test plan (Q9).

**Needs the multi-host bench to answer:** reconnect latency & reliability per OS (B2/B3);
the wrong-host race (B4 — whitelist trigger); forget self-heal per OS (B6); host-side-forget
tolerance (B7); real-world overflow feel (B8); Windows-specific quirks we cannot predict
from docs.

---

## Q12. BENCH OUTCOME (2026-11-07) — the Q4 contingency is superseded

The bench ran the full Q4 ladder plus its contingencies across 15 firmware
iterations (full record: `Build_Files/bench-16-multi-host-results-2026-11-07.txt`;
every mechanism bench-proven on the wire, macOS + Windows hosts):

1. **The Q4 whitelist contingency is NOT implementable on S140 6.1.1.**
   `sd_ble_gap_adv_set_configure` refuses whitelist-filtered connectable
   advertising with `0x3201 BLE_ERROR_GAP_DISCOVERABLE_WITH_WHITELIST` (the
   restriction covers the scannable/"discoverable" stock type), legacy PDUs
   have **no** connectable-non-scannable-undirected type, and the SoftDevice
   **does not enforce** either the directed target field or a whitelist on
   directed sets (a non-addressed peer's connect-by-address is accepted).
   Three sub-mechanisms (whitelist+open, directed alone, directed+whitelist)
   were each built, flashed, and measured before this was certain.
2. **Directed advertising also does not attract Windows**: its association
   stack ignores directed packets addressed to it (macOS ignores
   directed-to-others — which conveniently suppresses the racer). Fair trial:
   4.8.17, healthy Windows, full 60 s windows, zero attempts.
3. **The shipped mechanism is open-advertising + the firmware bouncer**
   (shipped 4.8.14-4.8.16): the window is the stock `start(0)` open advertise
   held 60 s; ANY non-target connect is dropped within milliseconds by the
   firmware (one bounce per connection — the SD needs ~200 ms to tear a link
   down and a millisecond tick would otherwise spin on the dying handle);
   the target is admitted the instant it connects; the bound guarantees a
   target-less window can never strand the radio. The wrong-host race is
   structurally eliminated without any controller filtering.
4. **Per-OS contract (bench-proven):** macOS reconnects in ~1 s whenever open
   advertising exists and is held off trivially during windows (one bounce,
   then ~60 s of its own backoff). Windows reconnects sub-second **only**
   while its pairing state is fresh, and otherwise requires the documented
   one-click nudge (its association stack ignores both open and directed
   invitations when stale — with Advanced discovery and Best-performance
   power mode set; the Power Management tab is absent on the bench adapter's
   driver). The Q4 per-OS table's "toggle the device in Settings" fallback is
   therefore the shipping behavior for Windows-ward switches, now
   bench-proven rather than predicted.
5. **Host staleness class (new, open):** heavy connect/drop churn (far beyond
   real usage) leaves hosts "connected but HID-dead" (live view healthy, no
   reports); a host-side toggle/reboot/re-pair clears it. A device-side
   self-heal is designed but unimplemented: if a connection's HID
   subscription does not enable within a few seconds, cycle the link.
   **Update (4.8.18): implemented** — the HID-subscription watchdog
   (btFeedbackLoop, 1 s): a bonded peer whose mouse-input CCCD is not
   enabled after a 5 s grace gets its link disconnected once (bounded 3
   cycles/session; the host's own retry re-subscribes cleanly).
6. **Q4 addendum (the contingency is not implementable as specced):** the
   design's SoftDevice-filter window — whitelist and/or directed with
   BLE_GAP_ADV_FP_FILTER_CONNREQ — cannot be built on S140 6.1.1:
   whitelist+discoverable is 0x3201-forbidden, no legacy
   connectable-non-scannable PDU type exists, and the directed target field
   AND the whitelist filter are not enforced on a directed set (measured
   4.8.8-4.8.11). The shipped mechanism is the device-side bouncer over
   directed (4.8.17 trial) or open (4.8.14-16, proven) advertising.
7. **RPA addendum (4.9.0):** the bouncer's identity match is RPA-aware — a
   privacy host (iOS default) reconnects with a rotating resolvable address
   whose bytes never equal the bond's stored identity; the tick resolves it
   against the armed bond's IRK (Core 5.3 ah(), sd_ecb_block_encrypt with
   the LE/BE byte order host-pinned), and IRK-less bonds get a 3 s
   bond-decision grace. The 4.9.0 review also closed the refused-disconnect
   lockout surrender, the mid-window forget interaction, and the BT,3
   honesty/window guards.
