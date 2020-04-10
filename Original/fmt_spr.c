//Credits go to SNV

#include "common.h"

typedef struct { // Total: 24 bytes
  u1 Id[4];     // SPR\0
  s4 Size;      // size of the whole file
  u4 U1;  // 0x4 for all creatures and tilesets
          // 0x2 for interf64.spr LOGO.spr magico64.spr magico80.spr
          //         overlay.spr scanner.spr Scrollbutt.spr
  u4 Frames;  // number of Frames
  u4 NPals;   // number of palettes
  u4 U2;  // 0 for battlebuttons.spr border.spr Buttons.spr
          //       CheckBox.spr interf64.spr magico80.spr markers.spr
          //       overlay.spr radiobuttons.spr scrollbar.spr selbut.spr
          //       slider.spr SplIcons.spr spotIcons.spr timer.spr
          // LOGO.spr, scanner.spr, Scrollbutt.spr have it 0xbf000000
          // magico64.spr has it 0xe3000000

  // u1 Palette[256*3*NPals];
  // u4 FrameOffsets[Frames];

} __attribute__ ((packed)) header;


typedef struct {
  u4 Deltas; // offset into Deltas
  u4 Pixels; // offset int Pixels
} __attribute__ ((packed)) row;


typedef struct {
  u4 Size;
  u4 W;  // width
  u4 H;  // height
  s4 CX; // center X
  s4 CY; // center Y
  u1 Name[8]; // Name of the file, this frame belongs to.
  u4 P;  // index of palette, this sprite uses
  u4 T1; // unknown offset 1
  u4 T2; // unknown offset 2

  //line Lines[Height];
  //u4 Deltas[];
  //u1 Pixels[];

  // sometimes, (when header->U1 != 2) there are two tables at the end,
  // pointed by T1 and T2. their purpose is unknown to me and without using them
  // output image is garbled.
  // their size seems has no relation to image size, being
  // somewhere between 80-84
  //u1 Table1[];
  //u1 Table2[];
} __attribute__ ((packed)) frame;


typedef struct {
  s4 X;
  s4 Y;
  u1 Name[8]; // Frame name in associated .spr file
  u4 U1;
  u4 U2;
  s4 X2;
  u4 U3;
  s4 Y2;
  u4 U4;
  u4 U5;
} __attribute__ ((packed)) aniFrame;

typedef struct {
  u1 Id[4]; // ANI\0
  u4 Size;  // size of whole file
  u4 U1;
  u4 U2;
  u4 U3;
  u4 NAnis; // number of animations
  u1 SprName[24]; // Name of associated .spr file;
  // u4 Offsets[NAnis]; // animation offsets
} __attribute__ ((packed)) aniHeader;



static void sprDecompile(char *Output, char *Input) {
  int I, J, K, X, Y, C;
  char Tmp[1024], Name[16];
  u4 *Lens, *Offs, *CU;
  int CTOffset; // Frame table offset
  char *LastName = "NIL Name, unused name";
  pic *P, *B;
  row *R;
  int L = fileSize(Input);
  header *H = (header*)readFile(0, L, Input);

  memset(Name, 0, 16);

  if (!H || memcmp(H->Id, "SPR\0", 4)) {
    printf("Invalid SPR file: %s\n", Input);
    abort();
  }

  printf("NFrames=%d NPals=%d U1=%d U2=#%x\n"
        , H->Frames, H->NPals, H->U1, H->U2);


  CTOffset = sizeof(header) + 256*3*H->NPals;

  if (H->U1 == 2) CTOffset -= 4;

  Offs = (u4*)((u1*)H+CTOffset);
  Lens = ns(u4, H->Frames);
  times (I, H->Frames) Offs[I] += CTOffset+4*H->Frames;
  times (I, H->Frames) Lens[I] = (I==H->Frames-1?H->Size:Offs[I+1])-Offs[I];

  times (I, H->Frames) {
    frame *F = (frame*)((u1*)H + Offs[I]);

    memcpy(Name, F->Name, 8);

    printf("%d,%s: O=#%x L=#%x W=%d H=%d CX=%d CY=%d P=%d, T1=#%x T2=#%x S1=%d S2=%d\n"
       , I, Name, Offs[I], Lens[I], F->W, F->H, F->CX, F->CY, F->P
       , F->T1, F->T2, F->T2 - F->T1, Lens[I]-F->T2);

    P = picNew(F->W?F->W:1, F->H?F->H:1, 8);

    if (H->NPals) {
      if (F->P >= H->NPals) F->P=0;
      u1 *Pal = (u1*)H+sizeof(header) + F->P*256*3;
      times (J, 256) {
        P->P[J*4+0] = Pal[J*3+0];
        P->P[J*4+1] = Pal[J*3+1];
        P->P[J*4+2] = Pal[J*3+2];
        P->P[J*4+3] = 0;
      }
    } else {
      times (J, 256) {
        P->P[J*4+0] = J;
        P->P[J*4+1] = J;
        P->P[J*4+2] = J;
        P->P[J*4+3] = 0;
      }
    }

    CU = ns(u4,256);

    P->K = 0x253;

    if (H->U1 == 2) L--;
    R = (row*)(F+1);

    for(Y=0; Y < F->H; Y++) {
      u1 *Deltas = (u1*)F + R[Y].Deltas;
      u1 *Pixels = (u1*)F + R[Y].Pixels;
      for(X=0, K=0; X < F->W; K=~K) {
        C = *Deltas++;
        unless (K) {X += C; continue;}
        while (C-- && X < F->W) {X++; CU[*Pixels++]++;}
      }
    }

    times (J, 256) if (!CU[J]) P->K = J;
    picClear(P, P->K);

    for(Y=0; Y < F->H; Y++) {
      u1 *Deltas = (u1*)F + R[Y].Deltas;
      u1 *Pixels = (u1*)F + R[Y].Pixels;
      for(X=0, K=0; X < F->W; K=~K) {
        C = *Deltas++;
        unless (K) {X += C; continue;}
        while (C-- && X < F->W) picPut(P,  X++, Y, *Pixels++);
      }
    }

    sprintf(Tmp, "%s/%s_%04d.png", Output, Name, I);
    pngSave(Tmp, P);
  }
}

int sprInit(format *F) {
  F->Type = FMT_ARCHIVE;
  F->Name = "spr";
  F->Description = "Magic & Mayhem sprites";
  F->Decompile = sprDecompile;
  return 1;
}

