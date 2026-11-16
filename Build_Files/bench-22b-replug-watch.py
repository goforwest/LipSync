#!/usr/bin/env python3
"""bench-22b-replug-watch.py — B22b: the power-cycle matrix (4.9.8).

The human does N unplug/replug cycles. This script autonomously watches the
CDC port: for each cycle it waits for the port to DROP (unplug), waits for it
to RETURN (replug + boot), settles, then probes VN + BH and drains for the
degraded-boot HB lines and the 4.9.8 scan-heal INFO notes.

Verdict per replug: the BH device classes (display/pressure/joystick) — the
wire-side truth that pairs with the human's beep count (2 error-beep pairs =
all missing; 1 pair = one class missing; ready cue = healthy).

Usage: bench-22b-replug-watch.py [cycles] [port]
"""
import glob
import os
import sys
import time

import serial


def find_port():
    ports = [p for p in sorted(glob.glob("/dev/cu.usbmodem*")) if "debug-console" not in p]
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


def drain(ser, seconds):
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


def probe(ser, cmd, drain_s=1.2):
    ser.reset_input_buffer()
    ser.write(b"SETTINGS\n")
    time.sleep(0.05)
    ser.write((cmd + "\n").encode())
    return drain(ser, drain_s)


def bh_fields(lines):
    for line in lines:
        if "SUCCESS,0:BH,0:" in line:
            parts = [p.strip() for p in line.split("SUCCESS,0:BH,0:", 1)[1].split(",")]
            return [int(p) for p in parts[:6]]
    return None


def version(lines):
    for line in lines:
        if "SUCCESS,0:VN,0:" in line:
            return line.split("SUCCESS,0:VN,0:", 1)[1].strip()
    return None


def wait_port(gone, seconds):
    """Wait until the usbmodem port is (gone=True) or is not (gone=False) present."""
    end = time.time() + seconds
    while time.time() < end:
        present = find_port() is not None
        if present != gone:
            return True
        time.sleep(0.3)
    return False


def main():
    cycles = int(sys.argv[1]) if len(sys.argv) > 1 else 5
    print(f"=== B22b replug watch: {cycles} power cycles — human replugs, script probes ===", flush=True)
    for cycle in range(1, cycles + 1):
        print(f"--- cycle {cycle}: waiting for UNPLUG (port drop) ---", flush=True)
        if not wait_port(gone=True, seconds=1800):
            print(f"cycle {cycle}: no unplug observed in 1800 s — stopping (human may be done).", flush=True)
            break
        print(f"--- cycle {cycle}: port dropped; waiting for REPLUG (port return) ---", flush=True)
        if not wait_port(gone=False, seconds=120):
            print(f"cycle {cycle}: port did not return in 120 s — stopping.", flush=True)
            break
        port = find_port()
        time.sleep(5.0)  # settle: bootloader + setup scan + safe-mode decision
        try:
            ser = open_with_retry(port, 30)
        except Exception as e:
            print(f"cycle {cycle}: FAIL could not reopen {port}: {e}", flush=True)
            continue
        vn = version(probe(ser, "VN,0:0"))
        bh = bh_fields(probe(ser, "BH,0:0", drain_s=2.5))
        extra = [l for l in drain(ser, 4.0)
                 if any(k in l for k in ("INFO: I2C scan", "WARNING: SAFE", "HB:", "ERROR"))]
        ser.close()
        state = f"display={bh[2]} pressure={bh[3]} joystick={bh[4]} recov={bh[0]} bus={bh[1]}" if bh else "NO-BH"
        print(f"cycle {cycle}: VN={vn}  {state}" + (f"  extra={extra}" if extra else ""), flush=True)
    print("=== B22b done ===", flush=True)


if __name__ == "__main__":
    main()
