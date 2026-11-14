/*
 * ac3.library: decodes ATSC A/52 (Dolby Digital, AC-3) audio to 16-bit
 * PCM, as mpega.library decodes MPEG audio. Any program may open it.
 *
 * This header, ac3_lib.sfd and the clib, inline and proto headers made
 * from it are MIT: (c) 2026 Brian King. The library itself is
 * GPL-2.0-or-later (liba52 inside): see src/ac3lib/README.
 *
 * ac3.library is liba52's integer build (68020 and up, no FPU needed);
 * ac3fpu.library, where present, is its floating-point build for a 68040
 * or 68060 with an FPU, with the same functions.
 */
#ifndef LIBRARIES_AC3_H
#define LIBRARIES_AC3_H

#include <exec/types.h>

#define AC3NAME         "ac3.library"
#define AC3FPUNAME      "ac3fpu.library"
#define AC3VERSION      1

#define AC3_FRAME_SAMPLES 1536      /* each frame's samples a channel (6 blocks of 256) */
#define AC3_MAX_FRAME   3840        /* a frame's bytes at the most */
#define AC3_HEADER      7           /* bytes Ac3SyncInfo needs */

/* Ac3Open's mode: the channels decoded to (interleaved in Ac3Decode's
 * buffer). The stream's own channels are mixed down as A/52 says. */
#define AC3_STEREO      0           /* two channels */
#define AC3_MONO        1           /* one */
#define AC3_DOLBY       2           /* two, Dolby Surround (Pro Logic) compatible */

/* Ac3Open's flags */
#define AC3F_NODRC      (1UL << 0)  /* the stream's dynamic range compression not applied */

/* A frame's header, as Ac3SyncInfo and Ac3Decode find it. */
struct Ac3Info {
    ULONG ai_FrameBytes;    /* this frame's length in bytes */
    ULONG ai_Rate;          /* samples a second: 48000, 44100 or 32000 */
    ULONG ai_BitRate;       /* bits a second */
    UWORD ai_Channels;      /* the stream's full channels, 1 to 5 */
    UWORD ai_LFE;           /* 1: it also has a low-frequency (.1) channel */
    UWORD ai_Mode;          /* A/52's acmod: 0 dual mono, 1 1/0, 2 2/0, 3 3/0, 4 2/1, 5 3/1, 6 2/2, 7 3/2 */
    UWORD ai_OutChannels;   /* the channels Ac3Decode wrote (1 or 2) */
};

/* Ac3Decode's errors (it returns these, negative) */
#define AC3E_SYNC       (-1)        /* no frame header there */
#define AC3E_CORRUPT    (-2)        /* the frame did not decode (the samples are silence) */
#define AC3E_ARGS       (-3)

#endif /* LIBRARIES_AC3_H */
