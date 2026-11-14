/*
 * Run on the build host: liba52's IMDCT tables (extras/liba52/liba52/
 * imdct.c, built with A52_IMDCT_DUMP, and LIBA52_FIXED for ac3.library)
 * written as C to stdout, for ac3.library's A52_IMDCT_TABLES, so that the
 * Amiga computes none of them. Part of ac3.library, GPL-2.0-or-later.
 */
#include <stdio.h>

void a52_imdct_dump(FILE *f);

int main(void)
{
    printf("/* Made by src/ac3lib/mktables.c from liba52's imdct.c: do not edit. */\n");
    a52_imdct_dump(stdout);
    return 0;
}
