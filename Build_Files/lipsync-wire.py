#!/usr/bin/env python3
"""lipsync-wire.py — minimal LipSync serial-protocol client for bench work.

Usage:
  lipsync-wire.py <port> "<COMMAND>"                 # SETTINGS handshake + command, drain 2 s, print
  lipsync-wire.py <port> "<COMMAND>" --after <sec>   # ... drain <sec> after the command (single process!)
  lipsync-wire.py <port> --drain <seconds>           # listen only (watch pushes)
  lipsync-wire.py <port> --drain <seconds> --ts      # timestamp each line (elapsed seconds)
  lipsync-wire.py <port> "<COMMAND>" --after <sec> --ts
  lipsync-wire.py --list                             # list candidate ports

Protocol notes (LipSync 4.x): every command needs a SETTINGS handshake first;
responses are SUCCESS/FAIL lines; MANUAL,0:* pushes arrive unprompted. A healthy
boot prints nothing until spoken to (the 4.5.29 wire contract).

Bench lessons:
  2026-11-06: a continuous DEBUG stream at 10 Hz + "extend on every chunk" =
  a drain that never ends — the drain uses a fixed wall clock.
  2026-11-07: command + long watch MUST be one process (two clients on the
  port kill each other), and timestamps must be stamped inline (a second
  process buffers everything to the end).
"""
import glob
import sys
import time

import serial  # pyserial


def open_port(path):
    ser = serial.Serial(path, 115200, timeout=0.1)
    ser.reset_input_buffer()
    return ser


def drain(ser, seconds, ts=False):
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
                    lines.append((f"[t={time.time()-start:5.1f}s] " if ts else "") + text)
    if buf.strip():
        text = buf.decode(errors="replace").strip("\r")
        lines.append((" [partial] " if not ts else f"[t={time.time()-start:5.1f}s] [partial] ") + text)
    return lines


def command(ser, cmd, drain_s=2.0, ts=False):
    ser.write(b"SETTINGS\n")
    time.sleep(0.05)
    ser.write((cmd + "\n").encode())
    return drain(ser, drain_s, ts)


def main():
    args = sys.argv[1:]
    if not args or args[0] == "--list":
        ports = sorted(glob.glob("/dev/cu.usbmodem*")) + sorted(glob.glob("/dev/tty.usbmodem*"))
        print("\n".join(ports) if ports else "no usbmodem ports found")
        return

    port = args[0]
    rest = args[1:]
    save_path = None
    drain_s = 2.0
    ts = False
    if "--save" in rest:
        i = rest.index("--save")
        save_path = rest[i + 1]
        del rest[i : i + 2]
    if "--after" in rest:
        i = rest.index("--after")
        drain_s = float(rest[i + 1])
        del rest[i : i + 2]
    if "--ts" in rest:
        rest.remove("--ts")
        ts = True
    if not rest:
        print("usage: lipsync-wire.py <port> \"<COMMAND>\" [--after <sec>] [--ts] [--save <file>] | --drain <sec> [--ts]")
        sys.exit(2)

    ser = open_port(port)
    try:
        if rest[0] == "--drain":
            seconds = float(rest[1]) if len(rest) > 1 else 5.0
            lines = drain(ser, seconds, ts)
        else:
            lines = command(ser, " ".join(rest), drain_s, ts)
        for line in lines:
            print(line)
        if save_path:
            with open(save_path, "w") as f:
                f.write("\n".join(lines) + "\n")
            print(f"[saved to {save_path}]")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
