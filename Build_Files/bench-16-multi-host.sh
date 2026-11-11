#!/usr/bin/env bash
# ============================================================================
# LipSync V4.8.x — multi-host Bluetooth switching bench (upstream #16)
# Numbered pass/fail script per docs/design/16-multi-host-ble.md Q10.
#
# The expected build version is DERIVED from LSConfig.h at run time (the
# 4.8.2 review: B0 used to hardcode the release version and every bump
# silently broke the identity gate — the exact drift the byte-identity ZIP
# check exists to prevent elsewhere). Run from any repo checkout; a missing
# LSConfig.h leaves EXPECT empty and B0 falls back to manual verification.
#
# Usage:   ./bench-16-multi-host.sh [OS_LABEL]
#          OS_LABEL: macos | windows | ios | android   (default: macos)
# Wire access: any serial console at the device's CDC port, 115200 8N1.
#          macOS:  screen /dev/tty.usbmodem* 115200   (or `make monitor`)
#          Windows: PuTTY on the COM port
# Record:  every step appends one line to bench-16-<OS_LABEL>-results.txt
#
# Rules of engagement:
#   * Run steps IN ORDER. B4 is the go/no-go gate: a fail there schedules the
#     IRK-whitelist contingency (+2-3 d) BEFORE any further bench investment.
#   * A step is PASS only when every listed observation matches. Anything else
#     is FAIL with the observed behavior written down.
#   * The deciding steps are B2/B3/B4 (host reconnect policies — the #1
#     unknown) and B11 (downgrade round-trip).
# ============================================================================
set -u
OS="${1:-macos}"
RESULTS="bench-16-${OS}-results.txt"
: > "$RESULTS"

# The version the flashed build must report: read from this checkout's
# LSConfig.h (major.minor.rev), never hardcoded — a version bump without a
# matching bench edit used to fail B0 on a perfectly healthy build.
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
EXPECT="$(awk '/^#define[ \t]+CONF_LIPSYNC_VERSION_MAJOR/{printf "%s.", $3}
               /^#define[ \t]+CONF_LIPSYNC_VERSION_MINOR/{printf "%s.", $3}
               /^#define[ \t]+CONF_LIPSYNC_VERSION_REV/{printf "%s", $3}' \
          "$SCRIPT_DIR/Firmware_Files/LipSync_Firmware/LSConfig.h" 2>/dev/null)"
[ -n "$EXPECT" ] || EXPECT="<read LSConfig.h>"

note() { echo "[$(date +%H:%M:%S)] $*" | tee -a "$RESULTS"; }
step() { echo "" | tee -a "$RESULTS"; echo "=== $* ===" | tee -a "$RESULTS"; }
pass() { note "PASS B$1: $*"; }
fail() { note "FAIL B$1: $*"; }

cat <<WIRE
Wire snippets used below (paste into the console after SETTINGS):
  SETTINGS          -> SUCCESS,0:SETTINGS        (handshake before every command)
  BT,0:0            -> SUCCESS,0:BT,0:<count>
  BT,0:1            -> SUCCESS,0:BT,0:1:<name>:<active>
  BT,1:2            -> SUCCESS,0:BT,1:2          (switch to row 2)
  BT,2:1            -> SUCCESS,0:BT,2:1          (forget row 1)
  BT,3:1            -> SUCCESS,0:BT,3:1          (ensure advertising)
  VN,0:0            -> SUCCESS,0:VN,0:$EXPECT     (verify the build first!)
Pushes arrive unprompted: MANUAL,0:BT,4:<n> / MANUAL,0:BT,5:<row>
WIRE

step "B0 Build identity (all OSes)"
note "ACTION: VN,0:0 must answer $EXPECT; Hub > Info shows the same. Flash the local-builder or CI artifact first."
note "OBSERVED: VN answered: ____________"
read -rp "VN=$EXPECT and matches Info? [y/N] " a; [ "$a" = y ] && pass 0 "build identity" || fail 0 "build identity (VN/Info mismatch?)"

step "B1 Baseline single-host (${OS})"
note "ACTION: factory-fresh bond dir (or note existing bonds). Pair host A (this computer). SETTINGS; BT,0:0; BT,0:1."
note "OBSERVED count: ____ row1 name: ____________ active: ____"
note "OBSERVED Hub > More > Bluetooth shows: ____________"
read -rp "Name matches host A's Bluetooth name (or honest MAC fallback) and active=1 while connected? [y/N] " a
[ "$a" = y ] && pass 1 "baseline single host" || fail 1 "baseline single host"

step "B2 Switch away (${OS}) — THE DECIDING STEP"
note "ACTION: pre-pair host B (= the other computer/phone in BT,0). With A connected: BT,1:<rowB>."
note "OBSERVED A disconnects immediately? ____  LED resumes blink-blue advertising? ____"
note "OBSERVED B reconnects with NO host-side interaction in ____ s"
note "OBSERVED A's cursor dead the moment B took over? ____  MANUAL,0:BT,5 push seen on B's console? ____"
read -rp "B connected <=60 s with no host-side interaction? [y/N] " a
[ "$a" = y ] && pass 2 "switch away; time-to-cursor: see note above" || fail 2 "switch away (latency/reliability per ${OS})"

step "B3 Switch back (${OS})"
note "ACTION: from B's console: BT,1:<rowA>. Same observations in reverse."
note "OBSERVED: ____________________"
read -rp "Symmetric with B2? [y/N] " a
[ "$a" = y ] && pass 3 "switch back symmetric" || fail 3 "switch back asymmetric"

step "B4 Wrong-host race (${OS}) — GO/NO-GO GATE"
note "ACTION: A and B both in range and both powered. From A: BT,1:<rowB>. Repeat 10x. Tally who wins the link."
note "TALLY (target B wins / A steals): ____ / 10"
read -rp "Target won >= 9/10? [y/N] " a
if [ "$a" = y ]; then pass 4 "wrong-host race; whitelist contingency NOT triggered"
else fail 4 "wrong-host race — SCHEDULE THE IRK-WHITELIST CONTINGENCY (design Q4, +2-3 d)"; fi

step "B5 Hub-menu flow (device-side, ${OS})"
note "ACTION: on-device: More > Bluetooth. Walk rows; confirm a switch; hold a drag and try a switch; run Center Reset and try a switch."
note "OBSERVED refusals show error cue + NAMED note (not a bare beep)? ____
OBSERVED CH,0:0 never lies during dwell pages (623/624 named)? ____"
read -rp "Every refusal named, no silent no-op? [y/N] " a
[ "$a" = y ] && pass 5 "hub-menu flow + refusal UX" || fail 5 "hub-menu flow"

step "B6 Forget + self-heal (${OS})"
note "ACTION: BT,2 a CONNECTED host. Watch: does it re-pair without user action? Repeat 3x. On iOS/Android note RPA behavior."
note "OBSERVED: ____________________  bond file recreated? ____"
read -rp "Documented behavior per OS, no wedge? [y/N] " a
[ "$a" = y ] && pass 6 "forget self-heal" || fail 6 "forget self-heal"

step "B7 Host-side forget (${OS})"
note "ACTION: unpair the LipSync from the HOST's Bluetooth settings while bonded. BT,0:0 then BT,0:1..n."
note "OBSERVED stale-row ghost after re-read? ____  re-pair works? ____"
read -rp "Tolerated, no ghost row? [y/N] " a
[ "$a" = y ] && pass 7 "host-side forget" || fail 7 "host-side forget"

step "B8 Overflow (${OS})"
note "ACTION: pair a 5th host (or reuse bonds to reach 5). BT,0:0 must say 5; Hub page shows 4 rows + Pair new + Forget host; Forget list shows all 5 (cap 8)."
read -rp "5th works; page caps at 4; app lists 5? [y/N] " a
[ "$a" = y ] && pass 8 "overflow cap" || fail 8 "overflow cap"

step "B9 Storage truth (${OS})"
note "ACTION: after 6 bonds + repeated switches, check bond file sizes (B9 tooling: bond_print_list or LittleFS dump) — files present, sizes sane, CCCD updates bounded."
read -rp "No growth per switch? [y/N] " a
[ "$a" = y ] && pass 9 "storage truth" || fail 9 "storage truth"

step "B10 Power-loss mid-switch (${OS})"
note "ACTION: cut power during the 'Switching to host...' dwell. Reboot."
note "OBSERVED normal boot? ____ advertising? ____ bonds intact (BT,0 count unchanged)? ____ verdict class st=5 path clean? ____"
read -rp "All four? [y/N] " a
[ "$a" = y ] && pass 10 "power-loss mid-switch" || fail 10 "power-loss mid-switch"

step "B11 Downgrade round-trip (${OS}) — THE OTHER DECIDING STEP"
note "ACTION: with 3 bonds + custom settings: flash 4.7.0, run, EX,0 dump; flash $EXPECT back."
note "OBSERVED 4.7.0 boots normally with bonds untouched? ____ EX,0 diff vs pre-downgrade clean? ____"
note "OBSERVED re-upgrade shows all 3 hosts with correct active? ____"
read -rp "Bonds + settings budgets byte-identical across the round trip? [y/N] " a
[ "$a" = y ] && pass 11 "downgrade round-trip" || fail 11 "downgrade round-trip"

# ---------------------------------------------------------------------------
# 4.9.4 review-loop validation (B13-B15): the switch-window interaction fixes
# were proven by code-path analysis only. These three steps put each seam on
# real radio. Flash the 4.9.4 build first (B0's identity gate now expects it).
# ---------------------------------------------------------------------------

step "B13 Bounce-then-target (${OS}) — THE KEEP-OPEN FIX ON RADIO"
note "ACTION: hosts A (racer) and B (target) both in range. From A's console: BT,1:<rowB>."
note "ACTION: the moment A disconnects, force A to reconnect FIRST (toggle A's Bluetooth off/on, or walk A out of range and back). B stays passive."
note "OBSERVED A bounces within ~1 s (INFO: BT window bouncing non-target ...)? ____"
note "OBSERVED after the bounce: LED does NOT flicker to scan-blink during the window (no bounce-storm narration)? ____"
note "OBSERVED no BT,4/BT,5 pushes during the bounces (the app's host list stays put)? ____"
note "OBSERVED B connects INSIDE the window (within 60 s) without re-running BT,1? ____ MANUAL,0:BT,5 lands on B? ____"
note "OBSERVED the window's INFO lines: no 'window FALLBACK' / no radio-dark gap after the bounce? ____"
read -rp "Bounce happened, then B connected inside the same window? [y/N] " a
[ "$a" = y ] && pass 13 "keep-open: target admitted after a real bounce" || fail 13 "keep-open (radio dark after bounce / target locked out?)"

step "B14 Calibration vs window, both directions (${OS})"
note "ACTION 1 (window -> calibration): from A: BT,1:<rowB> to open the window. Within the dwell/window, from B's console: SETTINGS then CA,1:1."
note "OBSERVED CA,1 answers FAIL,3 AND prints 'ERROR: CA,1 refused: a Bluetooth switch window owns the radio — retry once it closes'? ____"
note "OBSERVED after the window ends: CA,1:1 runs normally? ____"
note "ACTION 2 (calibration -> window): start CA,1:1 again; mid-run (hold a corner), from another console: SETTINGS then BT,1:<rowA>."
note "OBSERVED BT,1 answers FAIL,3 with 'calibration/center flow in flight'? ____ the calibration completes normally? ____"
read -rp "Both directions refused with the named line, flows intact? [y/N] " a
[ "$a" = y ] && pass 14 "calibration/window mutual exclusion" || fail 14 "calibration/window gate (silent acceptance or flow corruption?)"

step "B15 Reset mid-window (${OS})"
note "ACTION 1 (software reset): from A: BT,1:<rowB> (window open, A bouncing harmlessly). From B's console: SETTINGS then CM,1:1 (same-mode write is a no-op ack) or a mode change."
note "OBSERVED the device resets cleanly (RESTARTING page, port drops, re-enumerates)? ____"
note "OBSERVED next boot normal: healthy boot prints nothing (or one BOOT line only if the previous boot was flagged abnormal)? ____ advertising resumes? ____ bonds intact (BT,0:0 count unchanged)? ____"
note "OBSERVED no pt=0/pm=0 unattributed-fault report on the successor boot (the close wrote no forensics)? ____"
note "ACTION 2 (update mode): repeat with the window open, then SETTINGS + UM,1:1 from B."
note "OBSERVED UM,1 ack received BEFORE the port drops? ____ 'Update mode' splash drawn? ____ bootloader appears? ____"
read -rp "Both reset paths clean with a window open? [y/N] " a
[ "$a" = y ] && pass 15 "reset paths close the window first" || fail 15 "reset mid-window (stuck radio / lost ack / forensics noise?)"

echo "" | tee -a "$RESULTS"
echo "================= SUMMARY (${OS}) =================" | tee -a "$RESULTS"
grep -c PASS "$RESULTS" | xargs -I{} echo "PASS lines: {}" | tee -a "$RESULTS"
grep FAIL "$RESULTS" | tee -a "$RESULTS"
note "B12 time spent: ____ h (plan: B1-B3 ~1.5 h/OS, B4-B10 ~3 h, B11 ~1 h)"
note "Matrix coverage note: macOS+Windows can complete B1-B11; iOS/Android steps run when hardware lands."
