import os

from dataclasses import astuple, dataclass
from typing import BinaryIO

from utils import int32, uint8, uint32


PALETTE_SIZE = 256


class InvalidSignature(Exception):
    pass


class InvalidFileSize(Exception):
    pass


class InvalidDimensionsException(Exception):
    pass


class MMFile:
    signature = None

    def __init__(self, fh: BinaryIO):
        self.verify_file(fh)
        self.parse_header(fh)
        self.parse_data(fh)

    def verify_file(self, fh: BinaryIO):
        fh.seek(0)
        sig = fh.read(4).decode()
        reported_size = uint32(fh)
        actual_size = fh.seek(0, os.SEEK_END)

        assert self.signature is not None, "signature required"

        if sig != self.signature:
            raise InvalidSignature(f'expected "{self.signature}" but got "{sig}"')

        if reported_size != actual_size:
            raise InvalidFileSize(f"expected {reported_size} but got {actual_size}")

    def parse_header(fh: BinaryIO):
        raise NotImplementedError()

    def parse_data(fh: BinaryIO):
        raise NotImplementedError()


@dataclass
class Frame:
    offset: int
    size: int
    width: int
    height: int
    centre_x: int
    centre_y: int
    name: str
    palette_index: int
    delta_offsets: list
    pixel_offsets: list

    @classmethod
    def from_buffer(cls, fh: BinaryIO, sprite_version: int):
        offset = fh.tell()
        size = uint32(fh)
        width = uint32(fh)
        height = uint32(fh)
        centre_x = int32(fh)
        centre_y = int32(fh)
        name = fh.read(8).decode().strip("\0")
        palette_index = int32(fh)

        # Unknown values
        if sprite_version > 2:
            fh.read(8)

        delta_offsets = [None] * height
        pixel_offsets = [None] * height

        for x in range(height):
            delta_offsets[x] = uint32(fh)
            pixel_offsets[x] = uint32(fh)

        return cls(
            offset=offset,
            size=size,
            width=width,
            height=height,
            centre_x=centre_x,
            centre_y=centre_y,
            name=name,
            palette_index=palette_index,
            delta_offsets=delta_offsets,
            pixel_offsets=pixel_offsets
        )

    @property
    def has_valid_dimensions(self):
        return self.width > 0 and self.height > 0

    def get_pixel_data(self, fh: BinaryIO, palettes: list):
        if not self.has_valid_dimensions:
            raise InvalidDimensionsException("width or height is 0")

        pixels = [None] * self.height

        if self.palette_index < 0 or self.palette_index > len(palettes):
            palette = palettes[0]
        else:
            palette = palettes[self.palette_index]

        for row in range(self.height):
            pixels[row] = []

            delta_offset = self.delta_offsets[row]
            pixel_offset = self.pixel_offsets[row]

            delta_start = self.pixel_offsets[0] if (row + 1 == self.height) else self.delta_offsets[row + 1]
            delta_amt = delta_start - delta_offset
            delta_values = [None] * delta_amt

            for x in range(delta_amt):
                fh.seek(self.offset + delta_offset + x)
                delta_values[x] = uint8(fh)

            for x, delta in enumerate(delta_values):
                is_colour = x & 1

                for y in range(delta):
                    if is_colour:
                        fh.seek(self.offset + pixel_offset + y)
                        pixel_index = uint8(fh)
                        pixel = palette.pixels[pixel_index]
                    else:
                        pixel = Pixel(0, 0, 0, 0)

                    pixels[row].extend(pixel.as_list())

        return pixels


@dataclass
class Palette:
    pixels: list

    @classmethod
    def from_buffer(cls, fh: BinaryIO):
        pixels = [None] * PALETTE_SIZE

        for x in range(PALETTE_SIZE):
            pixels[x] = Pixel(r=uint8(fh), g=uint8(fh), b=uint8(fh), a=255)

        return cls(pixels)


class GreyscalePalette:
    def __init__(self):
        self.pixels = [Pixel(x, x, x, 255) for x in range(PALETTE_SIZE)]


@dataclass
class Pixel:
    r: int
    g: int
    b: int
    a: int

    def as_list(self):
        return list(astuple(self))
