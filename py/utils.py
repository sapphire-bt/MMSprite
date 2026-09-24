import struct

from typing import BinaryIO


def int32(fh: BinaryIO) -> int:
    return struct.unpack("<i", fh.read(4))[0]


def uint8(fh: BinaryIO) -> int:
    return struct.unpack("B", fh.read(1))[0]


def uint32(fh: BinaryIO) -> int:
    return struct.unpack("<I", fh.read(4))[0]
