// Magic & Mayhem decryption and unpacking routines
// Author: Nikita Sadkov
// License: BSD
// Decscription:
//   Routines used to unpack *.cfg and *.map files.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HIDWORD(x)  (*((uint32_t*)&(x)+1))

#define COMP_PLAIN  0
#define COMP_RLE    1
#define COMP_LZ77   2

#define HEADER_SIZE 20

typedef struct {
  uint32_t Seed; //seed for the PRNG used in decryption
  uint32_t UnpackedSize; //size of data[] after decompression
  uint32_t Checksum1; //checksum of decrypted data[]
  uint32_t Checksum2; //checksum of uncompressed data[]
  uint32_t Compression; //0=plain-text, 1=RLE, 2=LZ77
} __attribute__ ((packed)) header;

int filesize(char *filename) {
  int Size;
  FILE *F = fopen(filename, "rb");
  if (!F) return -1;

  fseek(F, 0L, SEEK_END);
  Size = ftell(F);
  fseek(F, 0L, SEEK_SET);
  fclose(F);

  return Size;
}

uint32_t PRNG_State;
uint32_t PRNG_Map[256];
int PRNG_MapReady = 0;

void init_prng_map() {
  int i;
  for (i = 0; i < 0xF9; i++) PRNG_Map[i] = i+1;
  PRNG_Map[0xF9] = 0;
}

uint32_t prng(uint32_t *table) {
  uint32_t a, b, c;
  a = table[0];
  table[0] = PRNG_Map[a];
  b = table[1];
  table[1] = PRNG_Map[b];
  c = table[b + 2] ^ table[a + 2];
  table[a + 2] = c;
  return c;
}

#define _WORD uint16_t


#define LZDictSize 4096
uint8_t LZDict[LZDictSize];

typedef struct {
  uint8_t *Ptr;
  uint8_t BitPtr;
  uint32_t Value;
} __attribute__ ((packed)) lz_input;

void lz_unpack(uint8_t *Input, uint8_t *Output, int UnpackedSize) {
  lz_input LZInput;
  lz_input *LZ;
  int Count;
  uint8_t Bit;
  uint8_t *PtrInc;
  int ValueBit;
  char NextBit;
  char Value;
  char NextBit2;
  uint32_t BackRefBit;
  int BackRefOff;
  int BackRefI;
  int BackRefLen;
  char NextBit3;
  uint32_t LowBit;
  uint32_t HighBit;
  char NextBit4;
  int Value2;
  int DictIndex;
  int CountSave;

  memset(LZDict, 0, LZDictSize);

  LZ = &LZInput;

  LZ->Ptr = Input;
  LZ->BitPtr = 0x80;
  LZ->Value = 0;

  Count = 0;
  CountSave = 0;
  DictIndex = 1;
  while ( 1 ) {
    Bit = LZ->BitPtr;
    if ( Bit == 0x80 )
    {
      PtrInc = LZ->Ptr + 1;
      LZ->Value = *LZ->Ptr;
      LZ->Ptr = PtrInc;
    }
    ValueBit = LZ->Value & Bit;
    NextBit = Bit >> 1;
    LZ->BitPtr = NextBit;
    if ( !NextBit )
      LZ->BitPtr = 0x80;
    if ( ValueBit )
    {
      HighBit = 0x80;
      Value = 0;
      do
      {
        Bit = LZ->BitPtr;
        if ( Bit == 0x80 )
        {
          PtrInc = LZ->Ptr + 1;
          LZ->Value = *LZ->Ptr;
          LZ->Ptr = PtrInc;
        }
        if ( Bit & LZ->Value )
          Value |= HighBit;
        HighBit >>= 1;
        NextBit2 = Bit >> 1;
        LZ->BitPtr = NextBit2;
        if ( !NextBit2 )
          LZ->BitPtr = 0x80;
      }
      while ( HighBit );
      *Output++ = Value;
      ++Count;
      LZDict[DictIndex] = Value;
      CountSave = Count;
      DictIndex = ((uint16_t)DictIndex + 1) & 0xFFF;
      goto loop_continue;
    }
    BackRefBit = 0x800;
    BackRefOff = 0;
    do
    {
      Bit = LZ->BitPtr;
      if ( Bit == 0x80 )
      {
        PtrInc = LZ->Ptr + 1;
        LZ->Value = *LZ->Ptr;
        LZ->Ptr = PtrInc;
      }
      if ( Bit & LZ->Value )
        BackRefOff |= BackRefBit;
      BackRefBit >>= 1;
      NextBit3 = Bit >> 1;
      LZ->BitPtr = NextBit3;
      if ( !NextBit3 )
        LZ->BitPtr = 0x80;
    } while ( BackRefBit );
    if ( !BackRefOff )
      return;
    LowBit = 8;
    BackRefLen = 0;
    do
    {
      Bit = LZ->BitPtr;
      if ( Bit == 0x80 )
      {
        PtrInc = LZ->Ptr + 1;
        LZ->Value = *LZ->Ptr;
        Count = CountSave;
        LZ->Ptr = PtrInc;
      }
      if ( Bit & LZ->Value )
        BackRefLen |= LowBit;
      LowBit >>= 1;
      NextBit4 = Bit >> 1;
      LZ->BitPtr = NextBit4;
      if ( !NextBit4 )
        LZ->BitPtr = 0x80;
    }
    while ( LowBit );
    BackRefI = 0;
    if ( BackRefLen + 1 >= 0 ) {
      do
      {
        Value2 = LZDict[((uint16_t)BackRefOff + (uint16_t)BackRefI) & 0xFFF];
        *Output++ = Value2;
        ++Count;
        CountSave = Count;
        if ( Count == UnpackedSize ) return;
        LZDict[DictIndex] = Value2;
        ++BackRefI;
        DictIndex = ((uint16_t)DictIndex + 1) & 0xFFF;
      } while (BackRefI < BackRefLen+2);
    }
loop_continue:
    if ( Count == UnpackedSize )
      return;
  }
}

void prng_init(uint32_t *table, uint32_t seed) {
  uint32_t *p;
  int i, count;
  int64_t t;
  uint32_t a;
  uint32_t k;
  int32_t *q;
  int32_t b;

  if (!PRNG_MapReady) {
    for (i = 0; i < 0xF9; i++) PRNG_Map[i] = i+1;
    PRNG_Map[0xF9] = 0;
  }

  PRNG_State = seed;
  table[0] = 0;
  table[1] = 103;
  p = table + 251;
  count = 250;
  for (i=0; i < 250; i++)
  {
    t = 0x41C64E6DULL * PRNG_State;
    HIDWORD(t) <<= 16;
    t += 0xFFFF00003039ULL;
    PRNG_State = t;
    *p = HIDWORD(t) & 0xFFFF0000 | ((uint32_t)t >> 16);
    --p;
  }
  a = 0xFFFFFFFF;
  k = 0x80000000;
  q = table + 5;
  do {
    b = *q;
    *q = k | a & b;
    q += 7;
    k >>= 1;
    a >>= 1;
  } while ( k );
}

// it seeds random number generator with a key
// then it uses generated random numbers to XOR the input
void decrypt(void *Input, int Size) {
  int I, Len;
  uint32_t Table[256];
  uint32_t *P = (uint32_t*)Input;
  memset(Table, 0, 256*sizeof(uint32_t));

  prng_init(Table, 1234567890u);
  Table[254] = 0;

  prng_init(Table, *P++);

  Len = (Size-4)/4;
  for (I=0; I<Len; I++) *P++ ^= prng(Table);

  Len = (Size-4) % 4;
  for (I=0; I<Len; I++) *((uint8_t*)P) ^= prng(Table);
}

uint32_t checksum(void *Data, int Size) {
  int I;
  uint32_t Sum=0, Elem, *P=(uint32_t*)Data;
  Size /= 4;
  for (I = 0; I < Size; I++) {
    Elem = *P++;
    if (I&1) Sum += Elem;
    else Sum ^= Elem;
  }
  return Sum;
}

int main(int argc, char **argv) {
  FILE *InFile, *OutFile;
  uint32_t *Input;
  uint8_t *Output;
  int InSize, OutSize, UnpackedSize;
  header *H;

  if (argc!=3) {
    printf("Usage: mmdecrypt <input> <output>\n");
    return 0;
  }

  InSize = filesize(argv[1]);

  if (InSize <= 20) { //cant be smaller or equal to the header
    printf("Bad file: %s\n", argv[1]);
    return 0;
  }

  InFile = fopen(argv[1], "rb");
  OutFile = fopen(argv[2], "wb");

  Input = (uint32_t*)malloc(InSize*2);
  fread(Input, 1, InSize, InFile);

  init_prng_map();
  decrypt(Input, InSize);

  H = (header*)Input;
  printf("unpacked size: %d\n", H->UnpackedSize);

  if (H->Checksum1 == checksum(H+1, InSize-sizeof(*H))) {
    UnpackedSize = H->UnpackedSize;
  } else {
    UnpackedSize = InSize;
    printf("bad checksum for decrypted data.\n");
  }

  Output = (uint8_t*)malloc(UnpackedSize*2);

  if (COMP_PLAIN == H->Compression) {
    printf("compression: uncompressed\n");
    memcpy(Output, (uint8_t*)(H+1), UnpackedSize);
  } else if (COMP_RLE == H->Compression) {
    printf("compression: RLE\n");
  } else if (COMP_LZ77 == H->Compression) {
    printf("compression: LZ77\n");
    lz_unpack((uint8_t*)(H+1), Output, UnpackedSize);
  } else {
    printf("compression: unknown (%x); aborting\n", H->Compression);
    return 0;
  }

  if (H->Checksum2 != checksum(Output, UnpackedSize)) {
    printf("bad checksum for decompressed data.\n");
  }

  fwrite(Output, 1, UnpackedSize, OutFile);

  fclose(InFile);
  fclose(OutFile);

  printf("done!\n");

  return 0;
}
