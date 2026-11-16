#!/usr/bin/env python3
"""bench-22-sr-storm.py — B22: the boot-bus fix set's SR storm (4.9.8).

N cycles of: SR,1:1 (device acks, dwells CONF_RESET_DWELL_MS, reboots) ->
wait for the CDC port to return -> probe VN + BH -> verdict per boot.

Pass criteria per cycle:
  - VN,0 answers 4.9.8 (the flashed build is the one being exercised);
  - BH fields [2],[3],[4] (display, pressure, joystick) all != 0 — no device
    left NACKing after a deliberate reset (the class fix D exists for);
  - Wire recoveries (BH[0]) stay 0 and the bus stays healthy (BH[1]) — the
    reset path must not WEDGE the bus either.

Any cycle whose boot shows a missing device class is the 4.9.7-class
regression shape and is reported loudly (the device may then sit in safe
mode — recover it with a power cycle, hands on deck).

Usage: bench-22-sr-storm.py [port] [cycles]
"""
import glob
import sys
import time

import serial  # noqa: E402

PORT_CANDIDATES = ["/dev/cu.usbmodem101", "/dev/cu.usbmodem1101"]


def drain(ser, seconds):
    """Read lines until the deadline (the lipsync-wire.py drain, inlined —
    the helper's hyphenated filename can't be imported as a module)."""
    start = time.time()
    end = start + seconds
    lines = []
    buf = b""
    while time.time() < end:
        chunk = ser.read(4096)
        if chunk:
            buf += chunk
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                text = line.decode(errors="replace").strip("\r")
                if text:
                    lines.append(text)
    if buf.strip():
        lines.append("[partial] " + buf.decode(errors="replace").strip("\r"))
    return lines


def find_port():
    for p in PORT_CANDIDATES:
        if any(g.startswith(p) for g in glob.glob("/dev/cu.usbmodem*")):
            return p
    ports = sorted(glob.glob("/dev/cu.usbmodem*"))
    return ports[0] if ports else None


def open_with_retry(path, seconds):
    end = time.time() + seconds
    last_err = None
    while time.time() < end:
        try:
            return serial.Serial(path, 115200, timeout=0.1)
        except (serial.SerialException, OSError, FileNotFoundError) as e:
            last_err = e
            time.sleep(0.5)
    raise last_err


def probe(ser, cmd, drain_s=1.2):
    ser.reset_input_buffer()
    ser.write(b"SETTINGS\n")
    time.sleep(0.05)
    ser.write((cmd + "\n").encode())
    return drain(ser, drain_s)


def bh_fields(lines):
    for line in lines:
        if "SUCCESS,0:BH,0:" in line:
            payload = line.split("SUCCESS,0:BH,0:", 1)[1]
            parts = [p.strip() for p in payload.split(",")]
            return [int(p) for p in parts[:6]]
    return None


def version(lines):
    for line in lines:
        if "SUCCESS,0:VN,0:" in line:
            return line.split("SUCCESS,0:VN,0:", 1)[1].strip()
    return None


def main():
    cycles = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    port = sys.argv[1] if len(sys.argv) > 1 else find_port()
    if not port:
        print("FAIL: no usbmodem port found")
        return 1
    print(f"=== B22 SR storm: {cycles} cycles on {port} ===")

    # Pre-flight: the device must be up and healthy before the storm starts.
    ser = open_with_retry(port, 10)
    pre = probe(ser, "BH,0:0")
    fields = bh_fields(pre)
    print(f"pre-flight BH: {bh_fields(pre)}  (rows: recoveries,bus,display,pressure,joystick,age)")
    if not fields or fields[2] == 0 or fields[3] == 0 or fields[4] == 0:
        print("FAIL: pre-flight device is not healthy (missing device class) — aborting the storm.")
        ser.close()
        return 1
    ser.close()

    results = []
    for cycle in range(1, cycles + 1):
        # 1) SR,1:1 from a fresh connection (each connection does its own handshake)
        ser = open_with_retry(port, 10)
        ack = probe(ser, "SR,1:1", drain_s=1.0)
        acked = any("SUCCESS,0:SR,1:1" in l for l in ack)
        ser.close()
        # 2) the port drops during the dwell+reboot; wait it out
        time.sleep(3.5)
        # 3) reopen and probe the new boot
        ser = open_with_retry(port, 20)
        vn_lines = probe(ser, "VN,0:0")
        bh = bh_fields(probe(ser, "BH,0:0", drain_s=2.5))
        extra = [l for l in probe(ser, "SETTINGS", drain_s=2.0)
                 if any(k in l for k in ("INFO: I2C scan", "WARNING: SAFE", "HB:", "ERROR"))]
        ser.close()
        v = version(vn_lines)
        ok = (v == "4.9.8") and bh is not None and bh[2] != 0 and bh[3] != 0 and bh[4] != 0 and bh[0] == 0 and bh[1] == 1
        results.append((cycle, v, bh, ok, extra))
        state = f"display={bh[2]} pressure={bh[3]} joystick={bh[4]} recov={bh[0]} bus={bh[1]}" if bh else "NO-BH"
        print(f"cycle {cycle:>2}: {'PASS' if ok else 'FAIL'}  VN={v}  {state}"
              + (f"  extra={extra}" if extra else ""))

    bad = [r for r in results if not r[3]]
    print(f"=== B22 verdict: {len(results) - len(bad)}/{len(results)} boots healthy across {cycles} SR cycles ===")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
