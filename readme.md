# Magic & Mayhem Sprite Reader

<img src="https://www.bunnytrack.net/images/github/mm/sprites.png" />

A simple JavaScript plugin to read image data from sprite files used in the video game [Magic & Mayhem](https://en.wikipedia.org/wiki/Magic_and_Mayhem), aka Duel: The Mage Wars.

This is a JavaScript implementation of a C routine posted on the [OpenXcom forums](https://openxcom.org/forum/index.php/topic,3932.msg125396.html). These are included in the folder "Original". With thanks to user Nikita_Sadkov, the author, for sharing his work and findings.

Be sure to check out the [map formats writeup](formats.md) if you're interested in how maps are structured.

## How to Use

### Python

A script is included to dump a sprite to PNG; this requires [pypng](https://pypi.org/project/pypng/):

```shell
pip install pypng
```

Then, use `export_image.py` as follows:

```shell
py export_image.py --input RedCap.spr --dir .
```

This will export all sprite frames into the current directory.

If you don't care about the pypng dependency and just want to parse a sprite and do something with it:

```py
from mm_files import SpriteFile

fh = open("RedCap.spr", mode="rb")
sprite = SpriteFile(fh)

print(sprite.frames[0])
```

Output:

```
Frame(offset=2192, size=1460, width=33, height=46, centre_x=-1, centre_y=-2, name='RALA0001', palette_index=0, delta_offsets=[408, 411, 414, 417, 420, 423, 426, 429, 432, 435, 438, 441, 444, 447, 450, 453, 458, 465, 472, 479, 484, 490, 495, 500, 505, 508, 515, 520, 527, 534, 539, 542, 545, 548, 551, 554, 557, 560, 563, 566, 569, 572, 575, 578, 581, 584], pixel_offsets=[587, 591, 598, 607, 617, 628, 641, 654, 667, 681, 697, 715, 734, 752, 770, 791, 810, 828, 848, 868, 887, 908, 934, 956, 983, 1010, 1029, 1048, 1063, 1078, 1092, 1104, 1116, 1129, 1142, 1155, 1170, 1187, 1204, 1221, 1236, 1249, 1262, 1274, 1287, 1300])
```

### JavaScript

After including the JavaScript file in a page, pass an [`ArrayBuffer`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer) of the sprite file to the global `MMSprite` function. A minimal example is shown below:

```html
<input type="file" id="file-input" />

<script src="./mm-reader.js"></script>
<script>
    document.getElementById("file-input").addEventListener("input", function() {
        for (const file of this.files) {
            const fileReader = new FileReader();

            fileReader.onload = function() {
                // Get a sprite reader instance
                const reader = new MMReader();

                // Read sprite data
                const sprite = reader.readSprite(this.result);

                // Unpack frame data
                for (let i = 0; i < sprite.frames.length; i++) {
                    const frame = sprite.frames[i];

                    // Render frame to canvas
                    const canvas = sprite.frameToCanvas(i);

                    // Do something with the canvas
                    document.getElementById("canvas-container").appendChild(canvas);
                }
            }

            fileReader.readAsArrayBuffer(file);
        }
    })
</script>
```

## Properties
`sprite.header` : _Object_

The sprite file header.

```js
// RedCap.spr
{
    "id"        : "SPR\u0000",
    "size"      : 528540,
    "unknown_1" : 4,
    "frames"    : 350,
    "palettes"  : 1,
    "unknown_2" : 1
}
```

---

`sprite.palettes` : _Array_

All colour palettes used in the file. `palettes` is an array containing arrays of RGB pixel value objects.

```js
// RedCap.spr

// sprite.palettes[0][0]
{
    "r": 0,
    "g": 0,
    "b": 0
}

// sprite.palettes[0][1]
{
    "r": 191,
    "g": 0,
    "b": 0
}
```

---

`sprite.frames` : _Array_

Frame metadata. A frame object contains the following properties:

| Name            | Type     | Description
| ---             | ---      | ---
| `begin_offset`  | _Number_ | Offset in file.
| `size`          | _Number_ | Frame size, in bytes.
| `width`         | _Number_ | Frame width, in pixels.
| `height`        | _Number_ | Frame height, in pixels.
| `centre_x`      | _Number_ | Frame centre, X axis.
| `centre_y`      | _Number_ | Frame centre, Y axis.
| `name`          | _String_ | Frame name.
| `palette_index` | _Number_ | Frame colour palette index.
| `unknown_1`     | _Number_ | Unknown meaning. Only present if file header's `unknown_1` value is 2.
| `unknown_2`     | _Number_ | Unknown meaning. Only present if file header's `unknown_1` value is 2.
| `delta_offsets` | _Array_  | Delta value offsets. Use for reading pixel data.
| `pixel_offsets` | _Array_  | As above.

## Methods
`sprite.frameToCanvas(int frameIndex)` returns _[Canvas](https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API)_

Renders a sprite's frame object to a Canvas element. Frames are specified by index. See example in "How to Use" section.

## Notes

Certain sprite files ostensibly have no colour palette. `timer.spr`, for example, has a palette value of 0. Currently the plugin will generate a greyscale palette when encountering such values.
