#!/usr/bin/env python3
"""Capture a board's console output across USB re-enumeration.

The ESP32-S3 USB-Serial-JTAG peripheral resets when the chip resets, so a
crash-reboot loop makes the host-visible /dev/ttyACM* node disappear and come
back. This helper reopens the port transparently and timestamps each session so
reboot cadence is visible in the transcript.
"""

import argparse
import glob
import sys
import time

import serial


def candidate_ports(pattern):
    return sorted(glob.glob(pattern))


def capture(port_pattern, seconds, baud, out_path):
    deadline = time.time() + seconds
    lines = []
    session = 0

    while time.time() < deadline:
        ports = candidate_ports(port_pattern)
        if not ports:
            time.sleep(0.2)
            continue

        port = ports[0]
        session += 1
        header = f"\n===== [capture] session {session} on {port} @ {time.strftime('%H:%M:%S')} ====="
        print(header, flush=True)
        lines.append(header)

        try:
            with serial.Serial(port, baud, timeout=0.2) as ser:
                ser.dtr = False
                ser.rts = False
                # Pulse DTR/RTS in the esptool-compatible order used by
                # `idf.py monitor` so we capture from a known reset point.
                ser.setDTR(False)
                ser.setRTS(True)
                time.sleep(0.1)
                ser.setRTS(False)
                time.sleep(0.05)
                while time.time() < deadline:
                    chunk = ser.read(4096)
                    if not chunk:
                        continue
                    # Decode as latin-1 so every received byte survives verbatim;
                    # a corrupted FreeRTOS task name is only diagnosable if the
                    # raw bytes are preserved.
                    text = chunk.decode("latin-1")
                    sys.stdout.write(text)
                    sys.stdout.flush()
                    lines.append(text)
        except (serial.SerialException, OSError) as exc:
            note = f"\n===== [capture] port dropped: {exc} ====="
            print(note, flush=True)
            lines.append(note)
            time.sleep(0.3)

    if out_path:
        with open(out_path, "w", encoding="utf-8") as handle:
            handle.write("".join(lines))
        print(f"\n===== [capture] transcript written to {out_path} =====", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="/dev/ttyACM*")
    parser.add_argument("--seconds", type=float, default=20.0)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--out", default=None)
    args = parser.parse_args()
    capture(args.port, args.seconds, args.baud, args.out)


if __name__ == "__main__":
    main()
