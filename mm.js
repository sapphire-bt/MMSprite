window.MMSprite = function(arrayBuffer) {
	const self = this;

	this.arrayBuffer = arrayBuffer;
	this.dataView    = new DataView(arrayBuffer);

	// First four bytes must be "SPR\0"
	if (this.dataView.getInt32(0, 4) !== 0x525053) {
		console.log("Sprite file signature invalid");
		return null;
	}

	this.getHeader = function() {
		const header = {};

		let offset = 0;

		header.id        = self.getText(offset, offset += 4);
		header.size      = self.dataView.getInt32(offset, offset += 4, true);
		header.unknown_1 = self.dataView.getUint32(offset, offset += 4, true);
		header.frames    = self.dataView.getUint32(offset, offset += 4, true);
		header.palettes  = self.dataView.getUint32(offset, offset += 4, true);
		header.unknown_2 = self.dataView.getUint32(offset, offset += 4, true);

		return header;
	}

	this.getPalettes = function() {
		const palettes = [];

		// Not every sprite has a palette, so create 0-255 greyscale palette if missing
		if (self.header.palettes === 0) {
			const palette = [];

			for (let j = 0; j < 256; j++) {
				palette.push({
					r: j,
					g: j,
					b: j,
				})
			}

			palettes.push(palette);
		} else {
			for (let i = 0; i < self.header.palettes; i++) {
				const palette = [];
				const paletteOffset = 24 + i * 256 * 3;

				for (let j = 0; j < 256; j++) {
					palette.push({
						r: self.dataView.getUint8(paletteOffset + (j * 3 + 0)),
						g: self.dataView.getUint8(paletteOffset + (j * 3 + 1)),
						b: self.dataView.getUint8(paletteOffset + (j * 3 + 2)),
					})
				}

				palettes.push(palette);
			}
		}

		return palettes;
	}

	this.getAllFrames = function() {
		const frames = [];

		let frameOffset = 24 + self.header.palettes * 256 * 3;

		frameOffset += 4 * self.header.frames;

		if (self.header.unknown_1 === 2) frameOffset -= 4;

		while (frames.length < self.header.frames) {
			const frame = self.getFrame(frameOffset);

			frames.push(frame);

			frameOffset += frame.size;
		}

		return frames;
	}

	this.getFrame = function(offset) {
		const frame = {};

		const frameStartOffset = offset;

		frame.size          = self.dataView.getUint32(offset, offset += 4, true);
		frame.width         = self.dataView.getUint32(offset, offset += 4, true);
		frame.height        = self.dataView.getUint32(offset, offset += 4, true);
		frame.centre_x      = self.dataView.getInt32(offset, offset += 4, true);
		frame.centre_y      = self.dataView.getInt32(offset, offset += 4, true);
		frame.name          = self.getText(offset, offset += 8);
		frame.palette_index = self.dataView.getUint32(offset, offset += 4, true);

		if (self.header.unknown_1 !== 2) {
			frame.unknown_t1 = self.dataView.getUint32(offset, offset += 4, true);
			frame.unknown_t2 = self.dataView.getUint32(offset, offset += 4, true);
		}

		// Some frames appear to be unused, having 0/null properties (e.g. effects3.spr)
		if (!frame.width)  frame.width  = 1;
		if (!frame.height) frame.height = 1;

		// Some frames have huge values for the palette index
		if (frame.palette_index >= self.header.palettes) frame.palette_index = 0;

		// The corresponding colour palette for this frame
		const palette = self.palettes[frame.palette_index];

		// Canvas on which the frame will be drawn
		const canvas  = document.createElement("canvas");
		const context = canvas.getContext("2d");

		canvas.width  = frame.width;
		canvas.height = frame.height;

		const imageData = context.getImageData(0, 0, canvas.width, canvas.height);

		// Get delta/pixel offsets for each row of the image
		const deltaOffsets = [];
		const pixelOffsets = [];

		for (let y = 0; y < frame.height; y++) {
			deltaOffsets.push(self.dataView.getUint32(offset, offset += 4, true));
			pixelOffsets.push(self.dataView.getUint32(offset, offset += 4, true));
		}

		// Current pixel number in the canvas
		let imageDataOffset = 0;

		// Start drawing frame to canvas
		for (let y = 0; y < frame.height; y++) {
			const deltaOffset = deltaOffsets[y];
			const pixelOffset = pixelOffsets[y];

			const delta = [];

			// Delta describes how many pixels in a row are transparent, then how many are coloured,
			// repeated until the total reaches the width of the frame.
			const deltaLength = (y + 1 === frame.height ? pixelOffsets[0] : deltaOffsets[y + 1]) - deltaOffset;

			for (let i = 0; i < deltaLength; i++) {
				delta.push(
					self.dataView.getUint8(frameStartOffset + deltaOffset + i)
				);
			}

			// Go through the delta values for this row of pixels.
			// If the delta index is even then transparent pixels are drawn.
			// This can alternate several times in one image row.
			for (let i = 0; i < delta.length; i++) {
				if (i & 1) {
					for (let j = 0; j < delta[i]; j++) {
						const colour = palette[self.dataView.getUint8(frameStartOffset + pixelOffset + j)];

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

		frame.canvas = canvas;

		return frame;
	}

	this.getText = function(start, end) {
		const decoder = new TextDecoder();
		const data    = self.arrayBuffer.slice(start, end);

		return decoder.decode(data);
	}

	this.header   = this.getHeader();
	this.palettes = this.getPalettes();
}