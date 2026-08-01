#!/usr/bin/env python3
"""
Pack the optimised GIFs into a single image for the QSPI flash.

The animations are 3.6 MB together, which does not fit the ~1.5 MB of spare
internal flash, so they live on the 8 MB W25Q64 and the firmware reads them
through the memory-mapped window at 0x90000000.

Layout - deliberately trivial, because the firmware parses it with no
allocation and no library:

    offset  size  contents
    0       4     magic 'TTRQ'
    4       4     format version
    8       4     entry count
    12      4     reserved
    16      N*12  entries: offset, size, reserved
    ...           payload, each entry aligned to 4 bytes

Offsets are relative to the start of the image, so the firmware adds
QSPI_BASE_ADDR and has a pointer.

The magic is what lets the firmware tell "nothing programmed yet" from "data
present". An unprogrammed part reads 0xFF everywhere, which would otherwise be
handed to the GIF decoder as if it were an animation.

Usage:

    python tools/make_qspi_image.py dashboard_layout/gif/optimized -o qspi.bin
    STM32_Programmer_CLI -c port=SWD mode=UR -el W25Q64_TTR.stldr -w qspi.bin 0x90000000
"""

import argparse
import glob
import os
import struct
import sys

MAGIC = b"TTRQ"
VERSION = 1
HEADER_SIZE = 16
ENTRY_SIZE = 12
ALIGN = 4
FLASH_SIZE = 8 * 1024 * 1024


def align_up(value, alignment=ALIGN):
    return (value + alignment - 1) & ~(alignment - 1)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="directory of .gif files, packed in name order")
    parser.add_argument("-o", "--output", required=True)
    args = parser.parse_args()

    paths = sorted(glob.glob(os.path.join(args.input, "*.gif")))
    if not paths:
        sys.exit("no .gif files in %s" % args.input)

    payloads = [open(p, "rb").read() for p in paths]

    # Entry table is fixed-size, so payload offsets are known before writing.
    cursor = align_up(HEADER_SIZE + ENTRY_SIZE * len(payloads))

    entries = []
    for data in payloads:
        entries.append((cursor, len(data)))
        cursor = align_up(cursor + len(data))

    total = cursor
    if total > FLASH_SIZE:
        sys.exit("image is %d bytes, larger than the %d byte device" % (total, FLASH_SIZE))

    out = bytearray()
    out += MAGIC
    out += struct.pack("<III", VERSION, len(payloads), 0)
    for offset, size in entries:
        out += struct.pack("<III", offset, size, 0)

    for (offset, _), data in zip(entries, payloads):
        out += b"\x00" * (offset - len(out))
        out += data
    out += b"\x00" * (total - len(out))

    with open(args.output, "wb") as handle:
        handle.write(out)

    print("%-28s %10s %10s" % ("file", "offset", "size"))
    print("-" * 50)
    for path, (offset, size) in zip(paths, entries):
        print("%-28s 0x%08x %10d" % (os.path.basename(path), offset, size))
    print("-" * 50)
    print("%-28s %10s %10d  (%.2f MB, %.0f%% of the device)"
          % (args.output, "", total, total / 1048576.0, 100.0 * total / FLASH_SIZE))
    print()
    print("Flash it with:")
    print("  STM32_Programmer_CLI -c port=SWD mode=UR \\")
    print("      -el W25Q64_TTR.stldr -w %s 0x90000000" % args.output)


if __name__ == "__main__":
    main()
