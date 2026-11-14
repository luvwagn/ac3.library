/*
 * ac3.library's test: each AC-3 file named decoded through the library's
 * functions (on the host: ac3lib.c built in; on the 68k: the library
 * opened from PROGDIR:Libs or LIBS:), in every mode, and its samples'
 * CRC-32 (big-endian words) printed with the frames' details. The host's
 * lines and the 68k's must match each other and tests/fixtures/ac3/expected.txt.
 * (c) 2026 Brian King, MIT. Usage: ac3_test [-w out.raw] file...
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __amigaos__
#include <proto/exec.h>
#include <proto/ac3.h>
struct Library *Ac3Base;
#else
#include <exec/types.h>
#include <clib/ac3_protos.h>
#endif

static unsigned long crc_tab[256];

static void crc_init(void)
{
    unsigned long c;
    int i, k;
    for (i = 0; i < 256; i++) {
        for (c = (unsigned long)i, k = 0; k < 8; k++)
            c = c & 1 ? 0xEDB88320UL ^ (c >> 1) : c >> 1;
        crc_tab[i] = c;
    }
}

static unsigned long crc_add(unsigned long crc, const WORD *s, long n)
{
    long i;
    for (i = 0; i < n; i++) {
        unsigned char b[2];
        b[0] = (unsigned char)((unsigned short)s[i] >> 8);
        b[1] = (unsigned char)s[i];
        crc = crc_tab[(crc ^ b[0]) & 0xFF] ^ (crc >> 8);
        crc = crc_tab[(crc ^ b[1]) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

static int run(const char *path, ULONG mode, ULONG flags, FILE *raw)
{
    static WORD pcm[AC3_FRAME_SAMPLES * 2];
    static const char *names[3] = { "stereo", "mono", "dolby" };
    FILE *f = fopen(path, "rb");
    UBYTE *buf;
    long len, pos = 0, frames = 0, bad = 0;
    unsigned long crc = 0xFFFFFFFFUL;
    struct Ac3Info in;
    APTR a;
    const char *base = path, *c;
    for (c = path; *c; c++)
        if (*c == '/' || *c == ':')
            base = c + 1;
    if (!f)
        return printf("%s: not found\n", path), 1;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc(len ? len : 1);
    if (!buf || fread(buf, 1, len, f) != (size_t)len)
        return printf("%s: not read\n", path), 1;
    fclose(f);
    memset(&in, 0, sizeof(in));
    if (!(a = Ac3Open(mode, flags)))
        return printf("Ac3Open failed\n"), 1;
    for (;;) {
        LONG at = Ac3Find(buf + pos, len - pos, &in), n;
        if (at < 0 || pos + at + (long)in.ai_FrameBytes > len)
            break;
        pos += at;
        n = Ac3Decode(a, buf + pos, pcm, &in);
        if (n < 0)
            bad++;
        crc = crc_add(crc, pcm, AC3_FRAME_SAMPLES * in.ai_OutChannels);
        if (raw)
            fwrite(pcm, 2, AC3_FRAME_SAMPLES * in.ai_OutChannels, raw);
        pos += in.ai_FrameBytes;
        frames++;
    }
    Ac3Close(a);
    free(buf);
    printf("%s %s%s: %ld frames, %ld bad, %lu Hz, %lu bit/s, acmod %u, lfe %u, out %u: crc %08lx\n",
           base, names[mode], flags & AC3F_NODRC ? " nodrc" : "", frames, bad,
           (unsigned long)in.ai_Rate, (unsigned long)in.ai_BitRate, (unsigned)in.ai_Mode,
           (unsigned)in.ai_LFE, (unsigned)in.ai_OutChannels, crc ^ 0xFFFFFFFFUL);
    return 0;
}

int main(int argc, char **argv)
{
    FILE *raw = NULL;
    int i, r = 0;
    crc_init();
#ifdef __amigaos__
    if (!(Ac3Base = OpenLibrary((CONST_STRPTR) "PROGDIR:Libs/ac3.library", 1)) &&
        !(Ac3Base = OpenLibrary((CONST_STRPTR)AC3NAME, 1)))
        return printf("no ac3.library\n"), 20;
    printf("%s\n", (const char *)Ac3Version());
#endif
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-w") && i + 1 < argc) {
            raw = fopen(argv[++i], "wb");
            continue;
        }
        if (raw) {
            r |= run(argv[i], AC3_STEREO, 0, raw);
            continue;
        }
        r |= run(argv[i], AC3_STEREO, 0, NULL);
        r |= run(argv[i], AC3_MONO, 0, NULL);
        r |= run(argv[i], AC3_DOLBY, 0, NULL);
        r |= run(argv[i], AC3_STEREO, AC3F_NODRC, NULL);
    }
    if (raw)
        fclose(raw);
#ifdef __amigaos__
    CloseLibrary(Ac3Base);
#endif
    return r;
}
