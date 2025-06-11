#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <math.h>

typedef void u0;

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long long u8;

typedef signed char s1;
typedef signed short s2;
typedef signed int s4;
typedef long long s8;

typedef float f4;
typedef double f8;


static s2 s2le(u1 *P) {return (P[1]<<8) | P[0];}
static s4 s4le(u1 *P) {return (P[3]<<24) | (P[2]<<16) | (P[1]<<8) | P[0];}
static u2 u2le(u1 *P) {return (P[1]<<8) | P[0];}
static u4 u4le(u1 *P) {return (P[3]<<24) | (P[2]<<16) | (P[1]<<8) | P[0];}

static s2 s2be(u1 *P) {return (P[0]<<8) | P[1];}
static s4 s4be(u1 *P) {return (P[0]<<24) | (P[1]<<16) | (P[2]<<8) | P[3];}
static u2 u2be(u1 *P) {return (P[0]<<8) | P[1];}
static u4 u4be(u1 *P) {return (P[0]<<24) | (P[1]<<16) | (P[2]<<8) | P[3];}

static u8 _ru8_tmp;
static u4 _ru4_tmp;
static u2 _ru2_tmp;
#define ru8(X) (_ru8_tmp = *(u8*)X, X+=8, _ru8_tmp)
#define ru4(X) (_ru4_tmp = *(u4*)X, X+=4, _ru4_tmp)
#define ru2(X) (_ru2_tmp = *(u2*)X, X+=2, _ru2_tmp)

#define ru4be(X) (_ru4_tmp = u4be(X), X+=4, _ru4_tmp)
#define ru2be(X) (_ru2_tmp = u2be(X), X+=2, _ru2_tmp)

#define unless(x) if(!(x))
#define times(i,e) for(i=0; i<(e); i++)
#define n(Type) ((Type*)memset(malloc(sizeof(Type)), 0, sizeof(Type)))
#define ns(Type, Sz) ((Type*)memset(malloc(sizeof(Type)*(Sz)), 0, sizeof(Type)*(Sz)))

static void s2bePut(u1 *P, int V) {
  P[0]=(((u4)V>>8)&0xff);
  P[1]=(((u4)V)&0xff);
}

static void s4bePut(u1 *P, int V) {
  P[0]=(((u4)V>>24)&0xff);
  P[1]=(((u4)V>>16)&0xff);
  P[2]=(((u4)V>>8)&0xff);
  P[3]=(((u4)V)&0xff);
}


typedef struct {char *Root; int Size; char **Names;} fileList;

typedef struct pic pic;
struct pic {
  char *N;      // name
  int C;        // compression (0=uncompressed, 1=grayscale, 2=RLE)
  int W;        // width
  int H;        // height
  int B;        // bits per pixel
  int I;        // bytes per line
  int S;        // compressed/uncompressed size of D
  int K;        // color key (-1 means no color key)
  int BK;       // bounding key (for selection)
  int SK;       // shadow key (for shadow)
  int X;        // X displacement
  int Y;        // Y displacement
  int BoxW;     // bounding box width
  int BoxH;     // bounding box height
  int Delay;    // animation delay
  int SharedPalette;
  u1 *P;        // palette
  u1 *D;        // pixels
  pic *Proxy; // this pic acts as a proxy, for other pic.
};

typedef struct {
  int NPics;
  pic **Pics;
} fac;

typedef struct {
  char *Name;  // animation name
  int NFacs;   // number of faces
  fac *Facs;   // faces
} ani;

typedef struct {
  int ColorKey;
  u1 *Palette;
  int NAnis; // number of animations
  ani *Anis;
} spr;




// misc utils
static int max(int A, int B) {return A>B ? A : B;}
static int min(int A, int B) {return A<B ? A : B;}
void hd(u1 *P, int S); // hex dump
char *downcase(char *t);
char *upcase(char *t);

// filesystem utils
int fileExist(char *File);
int fileSize(char *File);
int folderP(char *Name);
int fileP(char *Name);
char *loadFile(char *name);
void removeFile(char *fn);
void tmpName(char *rn, char *pat);
char *shell(char *fmt, ...);
void makePath(char *Path);
void *readFile(u4 Offset, u4 Length, char *FileName);
int writeFile(u4 Offset, u4 Length, char *FileName, void *Data);
void pathParts(char *Dir, char *Name, char *Ext, char *Path);

// enumerates all files recursively, returns them in sorted order
fileList *getFileList(char *DirName);
void freeFileList(fileList *FL);

// Out image and sprite formats (should work as a common denominator)
pic *picNew(int W, int H, int BPP);
void picFree(pic *P);
pic *picDup(pic *P);
pic *picClear(pic *P, u4 V);
#define PIC_FLIP_X 0x01
#define PIC_FLIP_Y 0x02
void picBlt(pic *Destination, pic *Source, int Flags
           ,int DestinationX, int DestinationY
           ,int SourceX, int SourceY
           ,int BlitRegionWidth, int BlitRegionHeight);
spr *sprNew();
void sprDel(spr *S);
pic *picProxy(pic *Src, int X, int Y, int W, int H);
void saveFrames(char *Output, spr *S);
spr *loadFrames(char *DirName);
void picPut(pic *P, int X, int Y, u4 C);
void picPut24(pic *P, int X, int Y, u4 C);
void picPut32(pic *P, int X, int Y, u4 C);
u4 picGet(pic *P, int X, int Y);
u4 picGet24(pic *P, int X, int Y);
u4 picGet32(pic *P, int X, int Y);

#define R8G8B8(R,G,B) (((R)<<16)|((G)<<8)|(B))
#define R8G8B8A8(R,G,B,A) (((A)<<24)|((R)<<16)|((G)<<8)|(B))

#define fromR8G8B8(R,G,B,C) do { \
  u4 _fromC = (C)&0xFFFFFFFF; \
  B = (((_fromC)>> 0)&0xFF); \
  G = (((_fromC)>> 8)&0xFF); \
  R = (((_fromC)>>16)&0xFF); \
 } while (0)
#define fromR8G8B8A8(R,G,B,A,C) do { \
  u4 _fromC = (C)&0xFFFFFFFF; \
  B = (((_fromC)>> 0)&0xFF); \
  G = (((_fromC)>> 8)&0xFF); \
  R = (((_fromC)>>16)&0xFF); \
  A = (((_fromC)>>24)&0xFF); \
 } while (0)
#define fromR5G6B5(R,G,B,C) do { \
  u4 _fromC = (C)&0xFFFF; \
  B = ((((_fromC)>> 0)&0x1f)<<3)|0x7; \
  G = ((((_fromC)>> 5)&0x3f)<<2)|0x3; \
  R = ((((_fromC)>>11)&0x1f)<<3)|0x7; \
 } while (0)
#define fromR5G5B5A1(A,R,G,B,C) do { \
  u4 _fromC = (C)&0xFFFF; \
  A = (_fromC)&1; \
  B = ((((_fromC)>> 1)&0x1f)<<3)|0x7; \
  G = ((((_fromC)>> 6)&0x1f)<<3)|0x7; \
  R = ((((_fromC)>>11)&0x1f)<<3)|0x7; \
 } while (0)
#define fromA1R5G5B5(A,R,G,B,C) do { \
  u4 _fromC = (C)&0xFFFF; \
  B = ((((_fromC)>> 0)&0x1f)<<3)|0x7; \
  G = ((((_fromC)>> 5)&0x1f)<<3)|0x7; \
  R = ((((_fromC)>>10)&0x1f)<<3)|0x7; \
  A = ((_fromC)>>15)&1; \
 } while (0)
#define fromA4R4G4B4(A,R,G,B,C) do { \
  u4 _fromC = (C)&0xFFFF; \
  B = ((((_fromC)>> 0)&0xf)<<4)|0xf; \
  G = ((((_fromC)>> 4)&0xf)<<4)|0xf; \
  R = ((((_fromC)>> 8)&0xf)<<4)|0xf; \
  A = ((((_fromC)>>12)&0xf)<<4)|0xf; \
  B |= B>>4; \
  G |= G>>4; \
  R |= R>>4; \
  A |= A>>4; \
 } while (0)

void rgb_to_hsv(u1 r, u1 g, u1 b, double *h, double *s, double *v);
void hsv_to_rgb(double h, double s, double v, u1 *returncol);

void pcxSave(char *File, pic *P);
pic *pcxLoad(char *File);
void pcxSavePalette(char *File, u1 *Palette);

void gifSave(char *Output, spr *S);
spr *gifLoad(char *Input);

void tgaSave(char *File, pic *P);
pic *tgaLoad(char *File);

void lbmSave(char *File, pic *P);
pic *lbmLoad(char *File);

void bmpSave(char *File, pic *P);
pic *bmpLoad(char *File);

void pngSave(char *Output, pic *P);

spr *impLoad(char *Input);
void impSave(char *Output, spr *S);

spr *mam_sprLoad(char *Input);

void mpqDecompile(char *OutDir, char *FileName);
void mpqCompile(char *FileName, fileList *FL);
// StormLib cant handle Warcraft 2 and Diablo MPQs
void mpqDecompileOld(char *OutDir, char *FileName, char *ListFile);

// zlib utils
u1 *inflateBufIE(int OLen, int Len, u1 *In, int IgnoreError);
u1 *inflateBuf(int OLen, int Len, u1 *In);

typedef struct {
  int L;  // length
  u1 *D; // data
} bytes;
void bytesDel(bytes *B);


void wavSave(char *Output, u1 *Q, int L, int Bits, int Chns, int Freq);

#define FMT_IMAGE     0
#define FMT_SPRITE    1
#define FMT_ARCHIVE   2
#define FMT_BYTES     3 /* plain raw buffer */
#define FMT_TEXT      4 /* plain text */
#define FMT_PCM       5
#define FMT_MIDI      6
#define FMT_2D        7 /* 2d vector graphics */
#define FMT_3D        8 /* 3d vector graphics */

#define MAX_FORMATS 4096

typedef struct {
  int Type;
  char *Name;
  char *Description;
  void *Save;
  void *Load;
  void *Compile;
  void *Decompile;
} format;

extern int NFormats;
extern format Formats[MAX_FORMATS];

extern char ProgDir[];
extern char ProgName[];

#endif

