/*
 * ac3.library: ATSC A/52 (Dolby Digital, AC-3) audio decoded to 16-bit
 * PCM by liba52 0.8.0 (extras/liba52), for any program, as mpega.library
 * decodes MPEG audio. Built twice from this file:
 *   ac3.library     liba52's integer build (LIBA52_FIXED), 68020 and up,
 *                   no FPU; its multiplies one MULS.L each (A52_MUL64)
 *   ac3fpu.library  liba52's floating-point build, for a 68040 or 68060
 *                   with its FPU
 * Its interface (include/, ac3_lib.sfd) is MIT and is all a program
 * compiles; the library is opened at run time.
 *
 * A caller finds a frame (Ac3Find, or Ac3SyncInfo on its first 7 bytes),
 * has all of its bytes (ai_FrameBytes) together, and Ac3Decode makes its
 * 1536 samples a channel. One decoder per stream; each is used by one
 * task at a time. Its memory: about 30 KB a decoder.
 *
 * (c) 2026 Brian King. ac3.library is free software under the GNU General
 * Public License, version 2 or (at your option) any later version, as
 * liba52 is: see COPYING in extras/liba52.
 */
#include "config.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "a52.h"
#include <libraries/ac3.h>

#ifdef __amigaos__
#include <exec/libraries.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include "stabs.h"

#ifdef LIBA52_FIXED
const char LibName[] = "ac3.library";
const char LibIdString[] = "ac3 1.1 (14.11.2026) liba52 0.8.0, integer";
static const char __attribute__((used)) LibVerTag[] =
    "$VER: ac3.library 1.1 (14.11.2026) liba52 0.8.0, integer (68020+)";
#else
const char LibName[] = "ac3fpu.library";
const char LibIdString[] = "ac3fpu 1.1 (14.11.2026) liba52 0.8.0, FPU";
static const char __attribute__((used)) LibVerTag[] =
    "$VER: ac3fpu.library 1.1 (14.11.2026) liba52 0.8.0, FPU (68040/68060)";
#endif
const UWORD LibVersion = 1;
const UWORD LibRevision = 1;   /* 1.1: ac3fpu rounds without touching the FPCR */

struct ExecBase *SysBase;
#define API

void *ac3p_malloc(size_t n)
{
    return AllocVec(n, MEMF_ANY);
}

void ac3p_free(void *p)
{
    if (p)
        FreeVec(p);
}
#else
#define API                                 /* on the host: plain functions (tests/ac3_test.c) */
#endif

#ifdef LIBA52_FIXED
static const char version[] = "ac3 1.1, liba52 0.8.0, integer";
#else
static const char version[] = "ac3fpu 1.1, liba52 0.8.0, FPU";
#endif

struct Ac3 {
    a52_state_t *st;
    ULONG mode;
    ULONG flags;
};

/* A/52's acmod: its full channels */
static const UBYTE full_channels[8] = { 2, 1, 2, 3, 3, 4, 4, 5 };

static LONG sync_info(const UBYTE *buf, struct Ac3Info *info)
{
    int flags, rate, br;
    int n = a52_syncinfo((uint8_t *)buf, &flags, &rate, &br);
    if (!n)
        return 0;
    if (info) {
        info->ai_FrameBytes = (ULONG)n;
        info->ai_Rate = (ULONG)rate;
        info->ai_BitRate = (ULONG)br;
        info->ai_Mode = (UWORD)(flags & 7);
        info->ai_Channels = full_channels[flags & 7];
        info->ai_LFE = (flags & A52_LFE) ? 1 : 0;
        info->ai_OutChannels = 0;
    }
    return n;
}

API LONG Ac3SyncInfo(CONST UBYTE *buf, struct Ac3Info *info)
{
    if (!buf)
        return 0;
    return sync_info(buf, info);
}

API LONG Ac3Find(CONST UBYTE *buf, LONG len, struct Ac3Info *info)
{
    LONG i;
    if (!buf)
        return -1;
    for (i = 0; i + AC3_HEADER <= len; i++)
        if (buf[i] == 0x0B && buf[i + 1] == 0x77 && sync_info(buf + i, info))
            return i;
    return -1;
}

API APTR Ac3Open(ULONG mode, ULONG flags)
{
    struct Ac3 *a;
    if (mode > AC3_DOLBY)
        return NULL;
#ifdef __amigaos__
    a = AllocVec(sizeof(*a), MEMF_ANY | MEMF_CLEAR);
#else
    a = calloc(1, sizeof(*a));
#endif
    if (!a)
        return NULL;
    a->mode = mode;
    a->flags = flags;
    if (!(a->st = a52_init(0))) {
#ifdef __amigaos__
        FreeVec(a);
#else
        free(a);
#endif
        return NULL;
    }
    if (flags & AC3F_NODRC)
        a52_dynrng(a->st, NULL, NULL);
    return a;
}

API VOID Ac3Close(APTR ac3)
{
    struct Ac3 *a = ac3;
    if (!a)
        return;
    a52_free(a->st);
#ifdef __amigaos__
    FreeVec(a);
#else
    free(a);
#endif
}

static inline WORD to16(sample_t s)
{
#ifdef LIBA52_FIXED
    LONG i = s >> 15;
#else
    /* rounded to an integer by the FPU's own addition: 1.5 x 2^23 added,
     * the sum stored as a float leaves the integer in its low mantissa
     * bits. A (LONG) cast made gcc switch the FPCR to rounding toward zero
     * and back around every sample: 6 FPCR writes a stereo sample pair,
     * slow on a real 68040/060 and very slow under UAE's FPU (ac3fpu took
     * 14-35 ms a frame there against the integer build's 1.5). */
    union {
        float f;
        LONG i;
    } u;
    LONG i;
    u.f = s * 32767.0f + 12582912.0f;
    i = u.i - 0x4B400000L;
#endif
    return (WORD)(i > 32767 ? 32767 : i < -32768 ? -32768 : i);
}

API LONG Ac3Decode(APTR ac3, CONST UBYTE *frame, WORD *pcm, struct Ac3Info *info)
{
    static const int want[3] = { A52_STEREO, A52_MONO, A52_DOLBY };
    struct Ac3 *a = ac3;
    struct Ac3Info in;
    int fl, b, i, outch, ok = 1;
#ifdef LIBA52_FIXED
    level_t lv = 1 << 26;
#else
    level_t lv = 1;
#endif
    if (!a || !frame || !pcm)
        return AC3E_ARGS;
    if (!sync_info(frame, &in))
        return AC3E_SYNC;
    outch = a->mode == AC3_MONO ? 1 : 2;
    in.ai_OutChannels = (UWORD)outch;
    if (info)
        *info = in;
    fl = want[a->mode] | A52_ADJUST_LEVEL;
    if (a52_frame(a->st, (uint8_t *)frame, &fl, &lv, 0))
        ok = 0;
    for (b = 0; b < 6; b++) {
        WORD *o = pcm + b * 256 * outch;
        sample_t *s;
        if (!ok || a52_block(a->st)) {
            ok = 0;
            memset(o, 0, 256 * outch * sizeof(WORD));
            continue;
        }
        s = a52_samples(a->st);
        if (outch == 1) {
            for (i = 0; i < 256; i++)
                o[i] = to16(s[i]);
        } else if ((fl & A52_CHANNEL_MASK) == A52_MONO) {
            for (i = 0; i < 256; i++)          /* (a mono stream: both alike) */
                o[2 * i] = o[2 * i + 1] = to16(s[i]);
        } else {
            for (i = 0; i < 256; i++) {
                o[2 * i] = to16(s[i]);
                o[2 * i + 1] = to16(s[i + 256]);
            }
        }
    }
    if (!ok) {
        memset(pcm, 0, AC3_FRAME_SAMPLES * outch * sizeof(WORD));
        return AC3E_CORRUPT;
    }
    return AC3_FRAME_SAMPLES;
}

API CONST_STRPTR Ac3Version(void)
{
    return (CONST_STRPTR)version;
}

#ifdef __amigaos__
/* ---- the library */
int __UserLibInit(struct Library *lib)
{
    (void)lib;
    SysBase = *(struct ExecBase *volatile *)4L;
    return 0;
}

void __UserLibCleanup(void)
{
}

/* the jump table, in the .sfd's order (from bias 30) */
ADDTABL_2(Ac3Open, d0, d1);
ADDTABL_1(Ac3Close, a0);
ADDTABL_2(Ac3SyncInfo, a0, a1);
ADDTABL_3(Ac3Find, a0, d0, a1);
ADDTABL_4(Ac3Decode, a0, a1, a2, a3);
ADDTABL_0(Ac3Version);
ADDTABL_END();
#endif
