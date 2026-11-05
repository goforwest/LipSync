#!/usr/bin/env bash
# ============================================================================
# LipSync V4.8.1 — multi-host Bluetooth switching bench (upstream #16)
# Numbered pass/fail script per docs/design/16-multi-host-ble.md Q10.
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

note() { echo "[$(date +%H:%M:%S)] $*" | tee -a "$RESULTS"; }
step() { echo "" | tee -a "$RESULTS"; echo "=== $* ===" | tee -a "$RESULTS"; }
pass() { note "PASS B$1: $*"; }
fail() { note "FAIL B$1: $*"; }

cat <<'WIRE'
Wire snippets used below (paste into the console after SETTINGS):
  SETTINGS          -> SUCCESS,0:SETTINGS        (handshake before every command)
  BT,0:0            -> SUCCESS,0:BT,0:<count>
  BT,0:1            -> SUCCESS,0:BT,0:1:<name>:<active>
  BT,1:2            -> SUCCESS,0:BT,1:2          (switch to row 2)
  BT,2:1            -> SUCCESS,0:BT,2:1          (forget row 1)
  BT,3:1            -> SUCCESS,0:BT,3:1          (ensure advertising)
  VN,0:0            -> SUCCESS,0:VN,0:4.8.1      (verify the build first!)
Pushes arrive unprompted: MANUAL,0:BT,4:<n> / MANUAL,0:BT,5:<row>
WIRE

step "B0 Build identity (all OSes)"
note "ACTION: VN,0:0 must answer 4.8.1; Hub > Info shows the same. Flash the local-builder or CI artifact first."
note "OBSERVED: VN answered: ____________"
read -rp "VN=4.8.1 and matches Info? [y/N] " a; [ "$a" = y ] && pass 0 "build identity" || fail 0 "build identity (VN/Info mismatch?)"

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
note "ACTION: with 3 bonds + custom settings: flash 4.7.0, run, EX,0 dump; flash 4.8.1 back."
note "OBSERVED 4.7.0 boots normally with bonds untouched? ____ EX,0 diff vs pre-downgrade clean? ____"
note "OBSERVED re-upgrade shows all 3 hosts with correct active? ____"
read -rp "Bonds + settings budgets byte-identical across the round trip? [y/N] " a
[ "$a" = y ] && pass 11 "downgrade round-trip" || fail 11 "downgrade round-trip"

echo "" | tee -a "$RESULTS"
echo "================= SUMMARY (${OS}) =================" | tee -a "$RESULTS"
grep -c PASS "$RESULTS" | xargs -I{} echo "PASS lines: {}" | tee -a "$RESULTS"
grep FAIL "$RESULTS" | tee -a "$RESULTS"
note "B12 time spent: ____ h (plan: B1-B3 ~1.5 h/OS, B4-B10 ~3 h, B11 ~1 h)"
note "Matrix coverage note: macOS+Windows can complete B1-B11; iOS/Android steps run when hardware lands."
