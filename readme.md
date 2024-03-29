# Magic & Mayhem Sprite Reader

<img src="https://www.bunnytrack.net/images/github/mm/sprites.png" />

A simple JavaScript plugin to read image data from sprite files used in the video game [Magic & Mayhem](https://en.wikipedia.org/wiki/Magic_and_Mayhem), aka Duel: The Mage Wars.

This is a JavaScript implementation of a C routine posted on the [OpenXcom forums](https://openxcom.org/forum/index.php/topic,3932.msg125396.html). These are included in the folder "Original". With thanks to user Nikita_Sadkov, the author, for sharing his work and findings.

Be sure to check out the [map formats writeup](formats.md) if you're interested in how maps are structured.

## How to Use
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

Inspecting the file in a hex editor, there does seem to be a palette; however, in the case of `timer.spr` the total number of pixels appears to be 384 instead of the usual 768 (3 RGB pixels × 16 × 16).

Extracting such palettes is yet to be implemented.
