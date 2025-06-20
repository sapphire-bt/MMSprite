import argparse
import os

from glob import glob
from pathlib import Path

import png

from mm_files import FontFile, SpriteFile


def main(args: argparse.Namespace):
    for file_path in glob(args.input):
        if not os.path.exists(file_path):
            print(f"file not found: {file_path}")
            continue

        path = Path(file_path)
        base_name = path.stem
        file_ext = path.suffix.lower()

        if file_ext == ".sft":
            cls = FontFile
        elif file_ext == ".spr":
            cls = SpriteFile
        else:
            print(f"invalid file extension: {file_ext}")
            continue

        output_dir = os.path.join(args.dir, base_name)

        Path(output_dir).mkdir(parents=True, exist_ok=True)

        fh = open(file_path, mode="rb")
        sprite = cls(fh)
        pad = len(str(sprite.frame_count))

        for x, frame in enumerate(sprite.frames):
            print(f"exporting {base_name} frame {x+1} of {sprite.frame_count}")
            pixels = frame.get_pixel_data(fh, palettes=sprite.palettes)
            image = png.from_array(pixels, "RGBA")
            output_path = os.path.join(output_dir, f"{x+1:>0{pad}}.png")
            image.save(output_path)

        fh.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input", required=True, help='file or folder path (e.g. "C:\\sprites\\*.spr")')
    parser.add_argument("-d", "--dir", required=True, help="output directory")
    args = parser.parse_args()
    main(args)
