# Magic & Mayhem: File Formats
This is an incomplete list of information about the various file formats used by the game.

## Maps
Map data is stored in `Magic & Mayhem\Realms\` and contained in several different formats, the main ones being:

* `*.map` - map segments
* `Terrain.ttd` - "**T**errain **T**ype **D**ata"
* `Terrain.spr` - terrain sprites for this map
* `*.mps` - map placement scheme
* `*.evt` - event list

### .map
Encrypted by default; can be decrypted using Nikita Sadkov's unpacking routine, `mmdecrypt.c`. Always begins with a 76 byte header:

```cpp
// CFsec02.map
struct MapHeader {
	uint32_t Version;     // 6

	uint32_t MapSizeX;    // 40 (tiles)
	uint32_t MapSizeY;    // 40
	uint32_t MapSizeZ;    // 23

	uint32_t MapArea;     // 400  (MapSizeX × MapSizeY)
	uint32_t MapVolume;   // 9600 (MapSizeX × MapSizeY × MapSizeZ) i.e. total tile count

	uint32_t SegmentsX;   // 2 (i.e. 2 × 20 tile square segments across)
	uint32_t SegmentsY;   // 2

	int32_t  Side0_Edge0; // 2 (river)
	int32_t  Side0_Edge1; // 1 (path)
	int32_t  Side1_Edge0; // 0 (nothing)
	int32_t  Side1_Edge1; // 0
	int32_t  Side2_Edge0; // 1
	int32_t  Side2_Edge1; // 0
	int32_t  Side3_Edge0; // 0
	int32_t  Side3_Edge1; // 2

	int32_t  Unknown1;    // 512 (512 for every map file, except CPsec03.map)

	int32_t  Unknown2;    // -1 (-1 for every map file)
	int32_t  Unknown3;    // -1 (-1 for every map file)
};
```

The below image shows how the sides/edges are mapped. Smaller maps (e.g. 1 x 1 segments) have `*_Edge1` set to `-1` to indicate the lack of segment.

<img src="https://www.bunnytrack.net/images/github/mm/sides-edges.png" />

Immediately after the header, `MapVolume` number of map tiles follow. Map tiles begin at the bottom (ground) layer and build upwards layer by layer.

```cpp
// CFsec50.map - first (bottom) layer values:
struct MapTile {
	int16_t TerrainIndex; // 589    index into terrain type data (.TTD)
	int16_t Unknown1;     // -1     always -1 for celtic forest
	int16_t Unknown2;     // -1     always -1 for celtic forest
	int16_t Unknown3;     // -1     always -1 for celtic forest - crashes if set to 0
	int16_t Unknown4;     // 1      ranges from 1-14338; never 0
	int16_t Unknown5;     // 1      bool? only 0/1; bWalkable?
};
```

Using the `TerrainIndex` value into the terrain `.ttd` / `.spr` files, it is possible to reconstruct the segments layer by layer:

<img src="https://www.bunnytrack.net/images/github/mm/map-segment-1.gif" />

### Terrain.ttd / Terrain.spr
A list of tiles which can be used by map segments. Always begins with a 16 byte header:

```cpp
struct TTDHeader {
	char     Signature[4]; // TTD\0
	uint32_t FileSize;     // Calculated by: TileCount * 356 (size of terrain type) + 16 (size of header)
	uint32_t Version;      // 4
	uint32_t TileCount;    // 1753
};
```

`TileCount` number of tiles/terrain types follow the header. Each terrain type block is 356 bytes containing many properties, e.g. which sprite a tile should switch to when it's burnt.

Tiles appear in the same order in both the `.ttd` and `.spr` files.

### .mps
Map placement schemes specify where wizards, creatures, and items should spawn in a map segment. They begin with a 16 byte header:

```cpp
// CFsec14.mps
struct MapPlacementSchemeHeader {
	char     Signature[4]; // MPS\0
	uint32_t Unknown1;     // ?
	uint32_t Version;      // 1
	uint32_t Elements;     // 11 (elements = things to spawn)
};
```

Elements are 40 bytes and contain a couple of custom structs:

```cpp
struct MapPlacementSchemeElement {
	Location L; // X,Y,Z pos

	// Object/creature type
	ElementType Type;

	// Object/creature index; see Wizards\_default.wzd for object list; CFG\creature.cfg for creatures
	uint32_t Index;

	// Health / points?
	uint32_t Unknown6;

	// Only set for TYPE_CREATURE?
	uint32_t Unknown7;
	uint32_t Unknown8;
	uint32_t Unknown9;
	uint32_t Unknown10;
};
```

```cpp
struct Location {
	uint32_t X;
	uint32_t Y;
	uint32_t Z;
};
```

```cpp
enum ElementType {
	TYPE_UNDEFINED,          // 0
	TYPE_WIZARD_FRIENDLY,    // 1
	TYPE_WIZARD_ENEMY,       // 2
	TYPE_WIZARD_MULTIPLAYER, // 3
	TYPE_CREATURE,           // 4
	TYPE_ARTIFACT            // 5
	// TYPE_INVALID             default
};
```

### .evt
A list of locations where events should take place (e.g. dialogue sequences, enemy deployment, experience awards, etc). Used in conjunction with `*.cfg` files in the map's parent directory. Begins with a 16 byte header:

```cpp
struct EventHeader {
	char     Signature[4]; // EVT\0
	uint32_t Unknown;
	uint32_t Version;
	uint32_t EventCount;
};
```

```cpp
// CFsec50.evt
struct Event {
	Location L1;
	Location L2;   // Often (but not always) the same location as L1
	char Name[48]; // CF50 Redcap1_1
};
```

## Animations
Animation data is stored in `.ani` files which are kept alongside the corresponding sprite (`.spr`) files in `Magic & Mayhem\Creatures` and `Magic & Mayhem\Sprites`.

```cpp
/**
 * wizard1.ani - 44 / 0x2C byte header
 */
struct AniHeader {
	char     Signature[4];   // ANI\0
	uint32_t FileSize;
	uint32_t FrameCount;     // Total number of frames in file; not all animation "slots" are used, e.g. non-flyers have null flying anims
	uint32_t Version;        // 1-2 "out of date"; 3-4 supported; 5 most common (always?)
	uint32_t Unknown;        // usually null
	uint32_t AnimCount;      // anim = group of frames that represent an animation
	char     SpriteFile[20]; // associated sprite file name
};
```

Following the header are `AnimCount` number of DWORDs which are the frame counts of each animation (subtracted from the previous value). After that are the animations themselves:

```cpp
/**
 * 44 / 0x2C bytes per frame
 */
struct AniFrame {
	uint32_t FrameType; // 0 = sprite, 5 = something to do with events, 6 = end of anim
	int32_t  FrameData; // if FrameType == 0, this is the sprite index into the .spr file.
	uint32_t Unknown1;
	uint32_t Unknown2;
	char     Name[8];   // Name of sprite frame if type == 0

	// may not all be DWORDs
	uint32_t Unknown3;
	uint32_t Unknown4;
	uint32_t Unknown5;
	uint32_t Unknown6;
	uint32_t Unknown7;
};
```

It appears that all `.ani` files are grouped in the same way, i.e. the first animation group is walking/flying movement, followed by lightning strike, followed by attacking, etc.

Every animation appears in a group of eight, one for each rotation (N, NE, E, etc).

My guesses as to the animation groups are as follows:

| Group # | Description                  |
| -       | -                            |
| 1       | Walking / flying             |
| 2       | Electrocution                |
| 3       | Melee attack 1               |
| 4       | Melee attack 2               |
| 5       | Melee attack 3               |
| 6       | Melee attack 4               |
| 7       | UNUSED                       |
| 8       | UNUSED                       |
| 9       | Idle 1                       |
| 10      | Idle 2                       |
| 11      | Idle 3 / standing → sitting  |
| 12      | Idle 4 / sitting  → standing |
| 13      | ?                            |
| 14      | ?                            |
| 15      | Taunt/intimidate/cry?        |
| 16      | UNUSED                       |
| 17      | UNUSED                       |
| 18      | UNUSED                       |
| 19      | Take damage                  |
| 20      | UNUSED                       |
| 21      | UNUSED                       |
| 22      | UNUSED                       |
| 23      | Flying                       |
| 24      | UNUSED                       |
| 25      | Walking → flying             |
| 26      | UNUSED                       |
| 27      | UNUSED                       |
| 28      | UNUSED                       |
| 29      | UNUSED                       |
| 30      | Dying                        |
| 31      | UNUSED                       |
| 32      | Flying attack (or landing?)  |
| 33      | Ranged attack                |
| 34      | UNUSED                       |
| 35      | UNUSED                       |
| 36      | UNUSED                       |
| 37      | ?                            |
| 38      | ?                            |
| 39      | ?                            |
| 40      | ?                            |
| 41      | ?                            |
| 42      | ?                            |

## Enabling Developer Logs
It is possible to enable basic logging by editing `Chaos.exe`; simply change the value at offset `0xD59CA` from `0x75` to `0x74` and the following log files will be created in the game directory next time the game is launched:

* Creature.plc
* events.plc
* mapgen.log
* mapplace.plc
* objects.plc
* region.plc
* Section.plc
* Wizard.plc
