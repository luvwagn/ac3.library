# ac3.library

A/52 (Dolby Digital, AC-3) audio, as found on DVDs, in transport streams
and in `.ac3` files, decoded to 16-bit PCM for any Amiga program, as
`mpega.library` decodes MPEG audio. Stereo, mono or Dolby Surround
compatible stereo: a stream's 5.1 is mixed down as A/52 says. Made for
Ambiance, an Amiga Media Center application, which opens it at run time
to play DVDs and videos where there is no ZZ9000 card to decode their
sound.

| Library | Build | For |
|---|---|---|
| `ac3.library` | liba52's integer build | a 68020, 68030 or 68040; no FPU needed. Not for a 68060: its 64-bit multiplies would be emulated by 68060.library |
| `ac3fpu.library` | liba52's floating-point build | a 68040 or 68060 with its FPU (the 68040's single-precision instructions: not for a 68881/68882) |

Both have the same functions (`ac3_lib.sfd`, `include/libraries/ac3.h`,
`ac3.doc`). A program tries `ac3fpu.library` (`AC3FPUNAME`) on a
68040/68060 with an FPU, else `ac3.library`.

The decoder is liba52 0.8.0 (`third_party/liba52`, from VideoLAN's
a52dec), changed only as `third_party/liba52/CHANGES` says: one `MULS.L`
a multiply in the integer build (half upstream's time on a 68020, and
more accurate), and the IMDCT's tables made on the build computer, so the
Amiga needs no maths library and computes nothing at `Ac3Open`.

## Using it

```c
#include <proto/ac3.h>
struct Library *Ac3Base = OpenLibrary(AC3FPUNAME, AC3VERSION);   /* 68040/060 with an FPU */
if (!Ac3Base)
    Ac3Base = OpenLibrary(AC3NAME, AC3VERSION);

APTR ac3 = Ac3Open(AC3_STEREO, 0);
struct Ac3Info ai;
WORD pcm[AC3_FRAME_SAMPLES * 2];
/* a frame's start found (Ac3Find), its Ac3SyncInfo(frame, &ai) bytes together: */
if (Ac3Decode(ac3, frame, pcm, &ai) == AC3_FRAME_SAMPLES)
    /* 1536 samples a channel at ai.ai_Rate, left and right interleaved */;
Ac3Close(ac3);
```

`ac3.doc` describes every function.

## Licence

GPL-3.0-or-later (`LICENSE`): liba52 is GPL-2.0-or-later
(`third_party/liba52/COPYING`). The interface (`include/`, `ac3_lib.sfd`,
`ac3.doc`) and the test are MIT and contain nothing of liba52's, so any
program may be built against it; a program that opens the library at run
time is not bound by its licence. The Aminet archive carries the libraries
as built; this repository is their source.

## Building

```
make            build/ac3.library, build/ac3fpu.library
make test       the fixtures (tests/mkac3.sh made them from synthetic
                sound) decoded in every mode on this computer and through
                ac3.library on a 68020 under vamos: their samples' CRCs
                alike, and as tests/fixtures/expected.txt
make lha        build/ac3.lha for Aminet
```

Needs [AmigaPorts' m68k-amigaos-gcc](https://github.com/AmigaPorts/m68k-amigaos-gcc)
16.2 (rc24 or later; the continuation of bebbo's amiga-gcc) with libnix, a
C compiler for the build
computer, and for the tests amitools' vamos.

## Speed

On a 68020, as vamos's Musashi core counts cycles, one frame (1536 samples
a channel: 32 ms at 48 kHz) takes about 3.5 million cycles in stereo at
192 kbit/s and 4.8 million in 5.1 at 448 kbit/s: about 110 and 150 MHz of
a 68020 to keep up. So a 68020 or 68030 cannot play AC-3 as it arrives; a
68040 at 40 MHz or so can manage stereo; a 68060 plays 5.1 with time to
spare (`ac3fpu.library`). Not yet measured on real hardware.

## Memory

About 30 KB a decoder, from exec (MEMF_ANY), and no more than 2 KB of
stack.
