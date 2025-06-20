import os

from typing import BinaryIO

from base_classes import (
    Frame,
    GreyscalePalette,
    MMFile,
    Palette,
    PALETTE_SIZE,
)
from utils import uint32


class SpriteFile(MMFile):
    signature = "SPR\0"
    header_size = 24

    @property
    def data_offset(self):
        return self.header_size + (self.palette_count * PALETTE_SIZE * 3) + (self.frame_count * 4)

    def parse_header(self, fh: BinaryIO):
        fh.seek(8)

        self.version = uint32(fh)
        self.frame_count = uint32(fh)
        self.palette_count = uint32(fh)

    def parse_data(self, fh: BinaryIO):
        self.frames = []
        self.palettes = []

        fh.seek(self.header_size)

        if self.palette_count == 0:
            self.palettes.append(GreyscalePalette())
        else:
            for x in range(self.palette_count):
                self.palettes.append(Palette.from_buffer(fh))

        fh.seek(self.data_offset)

        if self.version == 2:
            fh.seek(-4, os.SEEK_CUR)

        for x in range(self.frame_count):
            frame = Frame.from_buffer(fh, self.version)
            self.frames.append(frame)

            if (x + 1) < self.frame_count:
                fh.seek(frame.offset + frame.size)


class FontFile(SpriteFile):
    signature = "SFT\0"
    header_size = 40

    @property
    def data_offset(self):
        return super().data_offset + sum(self.unknown_values) * self.frame_count * 4

    def parse_header(self, fh: BinaryIO):
        fh.seek(8)

        self.version = uint32(fh)
        self.frame_count = uint32(fh)
        self.unknown_values = [uint32(fh) for x in range(4)]
        self.palette_count = uint32(fh)
