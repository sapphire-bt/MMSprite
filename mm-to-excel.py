import os
import struct
import sys
import xlsxwriter

# File signatures
SIGNATURE_ANI = "ANI\x00"
SIGNATURE_EVT = "EVT\x00"
SIGNATURE_MPS = "MPS\x00"

# .ani
FRAME_TYPE_SPRITE = 0

# .mps
MPS_ELEMENTS = (
	"Undefined",
	"Friendly Wizard",
	"Enemy Wizard",
	"Multiplayer Wizard",
	"Creature",
	"Artifact",
)

MPS_UNDEFINED          = 0
MPS_WIZARD_FRIENDLY    = 1
MPS_WIZARD_ENEMY       = 2
MPS_WIZARD_MULTIPLAYER = 3
MPS_CREATURE           = 4
MPS_ARTIFACT           = 5

# Misc.
MM_CREATURES = (
	"PLAYER_WIZARD",
	"Brownie",
	"Centaur",
	"Elf",
	"Griffin",
	"Hero",
	"Phoenix",
	"Unicorn",
	"BLACK_DOG",
	"MANTICORE",
	"REDCAP",
	"SKELETON",
	"VAMPIRE",
	"WRAITH",
	"ZOMBIE",
	"BAT",
	"BASILISK",
	"CROCODILE",
	"DRAGON",
	"FAUN",
	"Champ of Law",
	"TROLL",
	"EYE",
	"Champ of chaos",
	"Wizard1",
	"Wizard2",
	"Wizard3",
	"Wizard4 (?)",
)

MM_OBJECTS = (
	"MEAT",
	"BREAD",
	"FRUIT",
	"FISH",
	"WINE",
	"WATER BOTTLE",
	"PROVISION PACK",
	"QUILL",
	"SPORE OF SLUMBER",
	"SPORE OF STINGING",
	"SPORE OF SLAYING",
	"CAMPANIFORM BELL",
	"DRAGONS TEETH",
	"DRUM OF CONVOCATION",
	"EVIL EYE AMULET",
	"GLASSES OF VISION",
	"HAND OF GLORY",
	"HEX PENDANT",
	"HOLY WATER",
	"MANA SPRITE",
	"POISON",
	"RING OF MIGHT",
	"RING OF PROTECTION",
	"RUBY AMULET",
	"SALAMANDER BOOTS",
	"SCREAMING SKULL",
	"SEVEN LEAGUE BOOTS",
	"SYRINX PIPES",
	"SUMMON BROWNIE",
	"SUMMON CENTAUR",
	"SUMMON ELF",
	"SUMMON GRIFFIN",
	"SUMMON HERO",
	"SUMMON PHOENIX",
	"SUMMON UNICORN",
	"SUMMON BLACK_DOG",
	"SUMMON MANTICORE",
	"SUMMON REDCAP",
	"SUMMON SKELETON",
	"SUMMON VAMPIRE",
	"SUMMON WRAITH",
	"SUMMON ZOMBIE",
	"SUMMON BAT",
	"SUMMON BASILISK",
	"SUMMON CROCODILE",
	"SUMMON DRAGON",
	"SUMMON FAUN",
	"SUMMON SNAKE",
	"SUMMON TROLL",
	"SUMMON EYE",
	"CHEST",
	"KEY1",
	"KEY2",
	"SCROLL1",
	"SCROLL2",
	"SCROLL3",
	"SCROLL4",
	"SKULLS",
	"KEY3",
	"KEY4",
	"KEY5",
	"PLACE OF POWER",
	"UNUSED1",
	"UNUSED2",
	"UNUSED3",
	"UNUSED4",
	"UNUSED5",
	"UNUSED6",
	"UNUSED7",
	"UNUSED8",
	"UNUSED9",
	"UNUSED10",
)

# Conditional formatting colours
COL_GREEN  = "#63BE7B"
COL_YELLOW = "#FFEB84"
COL_RED    = "#F8696B"

def unpack(f, b):
	return struct.unpack(f, b)[0]

def get_spreadsheet(sheet_path):
	workbook  = xlsxwriter.Workbook(sheet_path)
	worksheet = workbook.add_worksheet()
	formats   = {
		"default": [
			workbook.add_format({
				"align"  : "left",
				"valign" : "vcenter",
			}),
			workbook.add_format({
				"align"  : "left",
				"valign" : "vcenter",
				"bottom" : 1,
			}),
		],
		"number": [
			workbook.add_format({
				"align"      : "left",
				"valign"     : "vcenter",
				"num_format" : "0",
			}),
			workbook.add_format({
				"align"      : "left",
				"valign"     : "vcenter",
				"num_format" : "0",
				"bottom"     : 1,
			}),
		],
		"bold": workbook.add_format({
			"align"  : "left",
			"valign" : "vcenter",
			"bold"   : True,
		}),
	}

	worksheet.set_zoom(120)

	return (workbook, worksheet, formats,)

def unpack_ani(file_path):
	name, ext = os.path.splitext(file_path)

	# Spreadsheet setup
	sheet_path = f"{name}.xlsx"
	workbook, worksheet, FORMATS = get_spreadsheet(sheet_path)

	headers = (
		"Offset",
		"Anim No.",
		"Frame No.",
		"Frame Type",
		"Frame Entry",
		"U01",
		"U02",
		"U03",
		"U04",
		"U05",
		"U06",
		"U07",
		"U08",
		"U09",
		"U10",
		"U11",
	)

	row = 0
	col = 0

	# Add header row
	for h in headers:
		worksheet.write(row, col, h, FORMATS["bold"])
		col += 1

	row += 1

	file_handle = open(file_path, mode = "rb")

	signature = file_handle.read(4).decode()

	if signature != SIGNATURE_ANI:
		raise "Invalid signature"

	file_size    = unpack("<I", file_handle.read(4))
	frames_count = unpack("<I", file_handle.read(4))
	version      = unpack("<I", file_handle.read(4))
	unknown_1    = unpack("<I", file_handle.read(4))
	anim_count   = unpack("<I", file_handle.read(4))
	assoc_sprite = file_handle.read(20).decode().strip("\x00")

	print("--------------------------------")
	print(f"Reading file {file_path}")
	print("--------------------------------")
	print("File size    : ", file_size)
	print("Total frames : ", frames_count)
	print("Version      : ", version)
	print("unknown_1    : ", unknown_1)
	print("Total anims  : ", anim_count)
	print("Anim sprite  : ", assoc_sprite)
	print()

	frame_deltas = []

	for i in range(anim_count):
		frame_deltas.append(unpack("<I", file_handle.read(4)))

	for i in range(anim_count):
		if i + 1 < anim_count:
			anim_frame_count = frame_deltas[i+1] - frame_deltas[i]

			for j in range(anim_frame_count):
				# Reset column
				col = 0

				# Use formatting with bottom-border if this is the last frame of this group
				f = 1 if (j + 1 == anim_frame_count) else 0

				# File offset
				worksheet.write(row, col, file_handle.tell(), FORMATS["number"][f])
				col += 1

				frame_type  = unpack("<I", file_handle.read(4))
				frame_entry = unpack("<i", file_handle.read(4))
				data1       = unpack("<i", file_handle.read(4))
				data2       = unpack("<i", file_handle.read(4))

				if frame_type == FRAME_TYPE_SPRITE:
					data3 = file_handle.read(8).decode().strip("\x00")
				else:
					temp_1 = unpack("<i", file_handle.read(4))
					temp_2 = unpack("<i", file_handle.read(4))

					if temp_1 == 0 and temp_2 == 0:
						data3 = ""
					else:
						data3 = str(temp_1) + " " + str(temp_2)

				data4  = unpack("<b", file_handle.read(1))
				data5  = unpack("<b", file_handle.read(1))
				data6  = unpack("<b", file_handle.read(1))
				data7  = unpack("<b", file_handle.read(1))
				data8  = unpack("<i", file_handle.read(4))
				data9  = unpack("<i", file_handle.read(4))
				data10 = unpack("<i", file_handle.read(4))
				data11 = unpack("<i", file_handle.read(4))

				# Group no.
				worksheet.write(row, col, i + 1, FORMATS["number"][f])
				col += 1

				# Frame no.
				worksheet.write(row, col, j + 1, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, frame_type, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, frame_entry, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data1, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data2, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data3, FORMATS["default"][f])
				col += 1

				worksheet.write(row, col, data4, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data5, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data6, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data7, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data8, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data9, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data10, FORMATS["number"][f])
				col += 1

				worksheet.write(row, col, data11, FORMATS["number"][f])
				col += 1

				row += 1
		else:
			break
			raise "To-do: read last frame"

	# Conditional formatting: low = green
	for h in ("Frame Type",):
		i = headers.index(h)

		worksheet.conditional_format(1, i, row, i, {
			"type"      : "3_color_scale",
			"min_color" : COL_GREEN,
			"mid_color" : COL_YELLOW,
			"max_color" : COL_RED,
		})

	file_handle.close()

	# Data filter
	worksheet.autofilter(0, 0, row, len(headers) - 1)

	# Freeze from C2
	worksheet.freeze_panes(1, 2)

	print(f"Saving data to file {sheet_path}\n")

	workbook.close()

	print("Done\n")

def unpack_evt(file_path):
	name, ext = os.path.splitext(file_path)

	# Spreadsheet setup
	sheet_path = f"{name}_evt.xlsx"
	workbook, worksheet, FORMATS = get_spreadsheet(sheet_path)

	headers = (
		"Offset",
		"Event No.",
		"X1",
		"Y1",
		"Z1",
		"X2",
		"Y2",
		"Z2",
		"Event Name",
	)

	row = 0
	col = 0

	for h in headers:
		worksheet.write(row, col, h, FORMATS["bold"])
		col += 1

	row += 1

	file_handle = open(file_path, mode = "rb")

	signature = file_handle.read(4).decode()

	if signature != SIGNATURE_EVT:
		raise "Invalid signature"

	unknown_1   = unpack("<I", file_handle.read(4))
	version     = unpack("<I", file_handle.read(4))
	event_count = unpack("<I", file_handle.read(4))

	print("--------------------------------")
	print(f"Reading file {file_path}")
	print("--------------------------------")
	print("unknown_1    : ", unknown_1)
	print("Version      : ", version)
	print("Total events : ", event_count)
	print()

	for i in range(event_count):
		col = 0

		worksheet.write(row, col, file_handle.tell(), FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, i + 1, FORMATS["number"][0])
		col += 1

		# Event values
		x1 = unpack("<I", file_handle.read(4))
		y1 = unpack("<I", file_handle.read(4))
		z1 = unpack("<I", file_handle.read(4))
		x2 = unpack("<I", file_handle.read(4))
		y2 = unpack("<I", file_handle.read(4))
		z2 = unpack("<I", file_handle.read(4))
		event_name = file_handle.read(48).decode().strip("\x00")

		worksheet.write(row, col, x1, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, y1, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, z1, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, x2, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, y2, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, z2, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, event_name, FORMATS["default"][0])
		col += 1

		row += 1

	file_handle.close()

	# Data filter
	worksheet.autofilter(0, 0, row, len(headers) - 1)

	# Freeze from C2
	worksheet.freeze_panes(1, 2)

	print(f"Saving data to file {sheet_path}\n")

	workbook.close()

	print("Done\n")

def unpack_mps(file_path):
	name, ext = os.path.splitext(file_path)

	# Spreadsheet setup
	sheet_path = f"{name}_mps.xlsx"
	workbook, worksheet, FORMATS = get_spreadsheet(sheet_path)

	headers = (
		"Offset",
		"El No.",
		"X",
		"Y",
		"Z",
		"Type",
		"Name / Index",
		"U1",
		"U2",
		"U3",
		"U4",
		"U5",
	)

	row = 0
	col = 0

	for h in headers:
		worksheet.write(row, col, h, FORMATS["bold"])
		col += 1

	row += 1

	file_handle = open(file_path, mode = "rb")

	signature = file_handle.read(4).decode()

	if signature != SIGNATURE_MPS:
		raise "Invalid signature"

	unknown_1 = unpack("<I", file_handle.read(4))
	version   = unpack("<I", file_handle.read(4))
	elements  = unpack("<I", file_handle.read(4))

	print("--------------------------------")
	print(f"Reading file {file_path}")
	print("--------------------------------")
	print("unknown_1      : ", unknown_1)
	print("Version        : ", version)
	print("Total elements : ", elements)
	print()

	for i in range(elements):
		col = 0

		worksheet.write(row, col, file_handle.tell(), FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, i + 1, FORMATS["number"][0])
		col += 1

		x        = unpack("<I", file_handle.read(4))
		y        = unpack("<I", file_handle.read(4))
		z        = unpack("<I", file_handle.read(4))
		el_type  = unpack("<I", file_handle.read(4))
		el_index = unpack("<I", file_handle.read(4))
		u1       = unpack("<i", file_handle.read(4))
		u2       = unpack("<i", file_handle.read(4))
		u3       = unpack("<i", file_handle.read(4))
		u4       = unpack("<i", file_handle.read(4))
		u5       = unpack("<i", file_handle.read(4))

		worksheet.write(row, col, x, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, y, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, z, FORMATS["number"][0])
		col += 1

		# MPS element type
		try:
			worksheet.write(row, col, MPS_ELEMENTS[el_type], FORMATS["default"][0])
			col += 1
		except:
			worksheet.write(row, col, f"{el_type} (Invalid)", FORMATS["default"][0])
			col += 1

		# Object name
		if el_type == MPS_ARTIFACT and el_index < len(MM_OBJECTS):
			worksheet.write(row, col, MM_OBJECTS[el_index], FORMATS["default"][0])
			col += 1

		elif el_type == MPS_CREATURE and el_index < len(MM_CREATURES):
			worksheet.write(row, col, MM_CREATURES[el_index], FORMATS["default"][0])
			col += 1

		else:
			worksheet.write(row, col, el_index, FORMATS["number"][0])
			col += 1

		worksheet.write(row, col, u1, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, u2, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, u3, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, u4, FORMATS["number"][0])
		col += 1

		worksheet.write(row, col, u5, FORMATS["number"][0])
		col += 1

		row += 1

	file_handle.close()

	# Data filter
	worksheet.autofilter(0, 0, row, len(headers) - 1)

	# Freeze from C2
	worksheet.freeze_panes(1, 2)

	print(f"Saving data to file {sheet_path}\n")

	workbook.close()

	print("Done\n")

def main():
	supported_types = [
		".ani",
		".evt",
		".mps",
	]

	if len(sys.argv) < 2:
		print(f"Usage: {os.path.basename(sys.argv[0])} <file 1[, file 2, ...]>")
		print("Supported file types:")
		print("\n".join(supported_types))
		exit()

	done = 0

	for file_path in sys.argv[1:]:
		if not os.path.exists(file_path):
			print(f"File not found: {file_path}")
		else:
			name, ext = os.path.splitext(file_path)
			ext = ext.lower()

			if ext not in supported_types:
				print(f"Unsupported file extension: {ext}")
				continue

			if ext == ".ani":
				unpack_ani(file_path)

			elif ext == ".evt":
				unpack_evt(file_path)

			elif ext == ".mps":
				unpack_mps(file_path)

			done += 1

	print(f"Finished (processed {done} {'file' if done == 1 else 'files'})")

if __name__ == "__main__":
	main()