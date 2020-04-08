# Magic & Mayhem Sprite Reader

A simple JavaScript plugin to read image data from sprite files used in the video game [Magic & Mayhem](https://en.wikipedia.org/wiki/Magic_and_Mayhem), aka Duel: The Mage Wars.

This is a JavaScript implementation of a C routine posted on the [OpenXcom forums](https://openxcom.org/forum/index.php/topic,3932.msg125396.html). With thanks to user Nikita_Sadkov, the author, for sharing his work and findings.

## How to Use
After including the JavaScript file in a page, pass an [`ArrayBuffer`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer) of the sprite file to the global `MMSprite` function. A minimal example is shown below:

```html
<input type="file" id="file-input" />

<script src="mm.js"></script>
<script>
    document.getElementById("file-input").addEventListener("input", function() {
        if (this.files.length > 0) {
            const file       = this.files[0];
            const fileReader = new FileReader();

            fileReader.onload = function() {
                const sprite = new MMSprite(this.result);
            }

            fileReader.readAsArrayBuffer(file);
        }
    })
</script>
```

## Properties
`header` _Object_

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

`palettes` _Array_

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

## Methods
`getAllFrames()` returns _Array_

Returns an array of objects describing each frame. A frame object contains the following properties:

| Name            | Type     | Description
| ---             | ---      | ---
| `size`          | _Number_ | Frame size, in bytes.
| `width`         | _Number_ | Frame width, in pixels.
| `height`        | _Number_ | Frame height, in pixels.
| `centre_x`      | _Number_ | Frame centre, X axis.
| `centre_y`      | _Number_ | Frame centre, Y axis.
| `name`          | _String_ | Frame name.
| `palette_index` | _Number_ | Frame colour palette index.
| `unknown_t1`    | _Number_ | Unknown meaning. Only present if file header's `unknown_1` value is 2.
| `unknown_t2`    | _Number_ | Unknown meaning. Only present if file header's `unknown_1` value is 2.
| `canvas`        | _Object_ | A [canvas](https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API) element of the frame.

```js
// RedCap.spr

const frames = sprite.getAllFrames();

// frames[0]
{
    "size"          : 1460,
    "width"         : 33,
    "height"        : 46,
    "centre_x"      : -1,
    "centre_y"      : -2,
    "name"          : "RALA0001",
    "palette_index" : 0,
    "unknown_t1"    : 1312,
    "unknown_t2"    : 1388,
    "canvas"        : <canvas width="33" height="46">
}
```

---

`getFrame(offset)` offset _Number_, returns _Object_

Returns a single frame object from a given offset within the file. Used by the `getAllFrames` method.

## Notes

Certain sprite files ostensibly have no colour palette. `timer.spr`, for example, has a palette value of 0. Currently the plugin will generate a greyscale palette when encountering such values.

Inspecting the file in a hex editor, there does seem to be a palette, however in the case of `timer.spr` the total number of pixels appears to be 384 instead of the usual 768 (3 RGB pixels × 16 × 16).

Extracting such palettes is yet to be implemented.
