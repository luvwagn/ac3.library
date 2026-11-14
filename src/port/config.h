/*
 * liba52's config.h for ac3.library (forced first into each of its
 * files): its memory from exec on the Amiga (it asks only when a decoder
 * is opened and closed). Part of ac3.library, GPL-2.0-or-later.
 */
#ifndef AC3LIB_CONFIG_H
#define AC3LIB_CONFIG_H

#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_INTTYPES_H 1
#ifdef __m68k__
#define WORDS_BIGENDIAN 1
#endif

#ifdef __amigaos__
#include <stddef.h>
void *ac3p_malloc(size_t n);
void ac3p_free(void *p);
#define malloc(n) ac3p_malloc(n)
#define free(p) ac3p_free(p)
#endif

#endif
