window.MMReader = function() {
	const reader = this;

	this.readSprite = function(arrayBuffer) {
		return new Sprite(arrayBuffer);
	}

	class Sprite {
		constructor(arrayBuffer) {
			reader.dataView = new DataView(arrayBuffer);

			this.header = new SpriteHeader();

			this.palettes = new Array(this.header.palettes);

			// Not every sprite has a palette, so create 0-255 greyscale palette if missing
			if (this.palettes.length === 0) {
				this.palettes[0] = new Array(256);

				for (let i = 0; i < this.palettes[0].length; i++) {
					this.palettes[0][i] = {
						r: i,
						g: i,
						b: i,
					}
				}
			} else {
				for (let i = 0; i < this.palettes.length; i++) {
					this.palettes[i] = new SpritePalette();
				}
			}

			// Relative offsets of each frame?
			reader.offset += 4 * this.header.frames;

			// Old sprite version?
			if (this.header.version === 2) reader.offset -= 4;

			this.frames = new Array(this.header.frames);

			for (let i = 0; i < this.frames.length; i++) {
				this.frames[i] = new SpriteFrame(this);
			}
		}

		frameToCanvas = function(frameIndex) {
			const frame = this.frames[frameIndex];

			// Canvas on which the frame will be drawn
			const canvas  = document.createElement("canvas");
			const context = canvas.getContext("2d");

			canvas.width  = frame.width;
			canvas.height = frame.height;

			const imageData = context.getImageData(0, 0, canvas.width, canvas.height);

			// Current pixel number in the canvas
			let imageDataOffset = 0;

			// Start drawing frame to canvas
			for (let y = 0; y < frame.height; y++) {
				const deltaOffset = frame.delta_offsets[y];
				const pixelOffset = frame.pixel_offsets[y];

				const delta = [];

				// Delta values store the difference between sequential pixels, alternating between colours and transparency
				const deltaLength = (y + 1 === frame.height ? frame.pixel_offsets[0] : frame.delta_offsets[y + 1]) - deltaOffset;

				for (let i = 0; i < deltaLength; i++) {
					delta.push(
						reader.dataView.getUint8(frame.begin_offset + deltaOffset + i)
					);
				}

				// Go through the delta values for this row of pixels.
				// If the delta index is even then transparent pixels are drawn.
				// This can alternate several times in one image row.
				for (let i = 0; i < delta.length; i++) {
					if (i & 1) {
						for (let j = 0; j < delta[i]; j++) {
							const colour = this.palettes[frame.palette_index][reader.dataView.getUint8(frame.begin_offset + pixelOffset + j)];

							imageData.data[imageDataOffset++] = colour.r;
							imageData.data[imageDataOffset++] = colour.g;
							imageData.data[imageDataOffset++] = colour.b;
							imageData.data[imageDataOffset++] = 255;
						}
					} else {
						for (let j = 0; j < delta[i]; j++) {
							imageData.data[imageDataOffset++] = 0;
							imageData.data[imageDataOffset++] = 0;
							imageData.data[imageDataOffset++] = 0;
							imageData.data[imageDataOffset++] = 0;
						}
					}
				}
			}

			context.putImageData(imageData, 0, 0);

			return canvas;
		}
	}

	class SpriteHeader {
		constructor() {
			this.signature = reader.getSizedText(4);

			if (this.signature !== "SPR\0") {
				throw "Invalid signature";
			}

			this.size      = reader.getUint32();
			this.version   = reader.getUint32();
			this.frames    = reader.getUint32();
			this.palettes  = reader.getUint32();
			this.unknown_2 = reader.getUint32();
		}
	}

	class SpritePalette {
		constructor() {
			for (let i = 0; i < 256; i++) {
				this[i] = {
					r: reader.getUint8(),
					g: reader.getUint8(),
					b: reader.getUint8(),
				}
			}
		}
	}

	class SpriteFrame {
		constructor(sprite) {
			// Used as reference for delta/pixel offsets
			this.begin_offset = reader.offset;

			this.size = reader.getUint32();

			// Some frames appear to be unused, having 0/null properties (e.g. effects3.spr)
			this.width  = reader.getUint32() || 1;
			this.height = reader.getUint32() || 1;

			this.centre_x = reader.getInt32();
			this.centre_y = reader.getInt32();

			// Name may contain null characters
			this.name = reader.getSizedText(8);

			if (this.name.indexOf("\0") !== -1) {
				this.name = this.name.substring(0, this.name.indexOf("\0"));
			}

			this.palette_index = reader.getUint32();

			// Some frames have huge values for the palette index
			if (this.palette_index >= sprite.header.palettes) this.palette_index = 0;

			if (sprite.header.version > 2) {
				this.unknown_1 = reader.getUint32();
				this.unknown_2 = reader.getUint32();
			}

			// Get delta/pixel offsets for each row of the image
			this.delta_offsets = new Array(this.height);
			this.pixel_offsets = new Array(this.height);

			for (let i = 0; i < this.height; i++) {
				this.delta_offsets[i] = reader.getUint32();
				this.pixel_offsets[i] = reader.getUint32();
			}

			reader.offset = this.begin_offset + this.size;
		}
	}

	/**
	 * Byte-related helper functions
	 */
	this.offset = 0;

	this.seek = function(offset) {
		return reader.offset = offset;
	}

	this.getInt8 = function() {
		const value = reader.dataView.getInt8(reader.offset);
		reader.offset += 1;
		return value;
	}

	this.getUint8 = function() {
		const value = reader.dataView.getUint8(reader.offset);
		reader.offset += 1;
		return value;
	}

	this.getInt16 = function() {
		const value = reader.dataView.getInt16(reader.offset, true);
		reader.offset += 2;
		return value;
	}

	this.getUint16 = function() {
		const value = reader.dataView.getUint16(reader.offset, true);
		reader.offset += 2;
		return value;
	}

	this.getInt32 = function() {
		const value = reader.dataView.getInt32(reader.offset, true);
		reader.offset += 4;
		return value;
	}

	this.getUint32 = function() {
		const value = reader.dataView.getUint32(reader.offset, true);
		reader.offset += 4;
		return value;
	}

	this.getFloat32 = function() {
		const value = reader.dataView.getFloat32(reader.offset, true);
		reader.offset += 4;
		return value;
	}

	this.getInt64 = function() {
		const value = reader.dataView.getBigInt64(reader.offset, true);
		reader.offset += 8;
		return value;
	}

	this.getUint64 = function() {
		const value = reader.dataView.getBigUint64(reader.offset, true);
		reader.offset += 8;
		return value;
	}

	this.getFloat64 = function() {
		const value = reader.dataView.getFloat64(reader.offset, true);
		reader.offset += 8;
		return value;
	}

	/**
	 * Text functions
	 */
	this.textDecoder = new TextDecoder();

	this.decodeText = function(bytes) {
		return reader.textDecoder.decode(bytes);
	}

	this.getSizedText = function(size) {
		if (size === undefined) {
			size = reader.getUint8();
		}

		const bytes = reader.dataView.buffer.slice(reader.offset, reader.offset + size);

		reader.offset += size;

		return reader.decodeText(bytes);
	}
}
