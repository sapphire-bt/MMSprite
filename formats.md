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
// Example values shown in comments
struct MapHeader {
	uint32_t Version;   // 6

	uint32_t MapSizeX;  // 20 (tiles)
	uint32_t MapSizeY;  // 20
	uint32_t MapSizeZ;  // 24

	uint32_t MapArea;   // 400  (MapSizeX x MapSizeY)
	uint32_t MapVolume; // 9600 (MapSizeX x MapSizeY x MapSizeZ) i.e. total tile count

	uint32_t Unknown1;  // 1 (region number?) - shows error if edited: "CAN NOT MAKE MAP Map size found inconsistant with first map"
	uint32_t Unknown2;  // 1
	uint32_t Unknown3;  // NULL - something to do with rotation? shows error if edited: "Section could not be placed in it`s current rotation. MAPMAKER will attempt to place section at another rotation."

	int32_t  Unknown4;  // -1
	int32_t  Unknown5;  // 0

	int32_t  Unknown6;  // -1
	int32_t  Unknown7;  // 0

	int32_t  Unknown8;  // -1
	int32_t  Unknown9;  // 0

	int32_t  Unknown10; // -1
	int32_t  Unknown11; // 512

	int32_t  Unknown12; // -1
	int32_t  Unknown13; // -1
};
```

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

<img src="https://www.bunnytrack.net/images/github/mm/map-segment.gif" />

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
