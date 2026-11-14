/* Written in the form sfdc 1.13 makes from ac3_lib.sfd. MIT, (c) 2026 Brian King. */

#ifndef _INLINE_AC3_H
#define _INLINE_AC3_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AC3_BASE_NAME
#define AC3_BASE_NAME Ac3Base
#endif /* !AC3_BASE_NAME */

#define Ac3Open(___mode, ___flags) \
      LP2(0x1e, APTR, Ac3Open , ULONG, ___mode, d0, ULONG, ___flags, d1,\
      , AC3_BASE_NAME)

#define Ac3Close(___ac3) \
      LP1NR(0x24, Ac3Close , APTR, ___ac3, a0,\
      , AC3_BASE_NAME)

#define Ac3SyncInfo(___buf, ___info) \
      LP2(0x2a, LONG, Ac3SyncInfo , CONST UBYTE *, ___buf, a0, struct Ac3Info *, ___info, a1,\
      , AC3_BASE_NAME)

#define Ac3Find(___buf, ___len, ___info) \
      LP3(0x30, LONG, Ac3Find , CONST UBYTE *, ___buf, a0, LONG, ___len, d0, struct Ac3Info *, ___info, a1,\
      , AC3_BASE_NAME)

#define Ac3Decode(___ac3, ___frame, ___pcm, ___info) \
      LP4(0x36, LONG, Ac3Decode , APTR, ___ac3, a0, CONST UBYTE *, ___frame, a1, WORD *, ___pcm, a2, struct Ac3Info *, ___info, a3,\
      , AC3_BASE_NAME)

#define Ac3Version() \
      LP0(0x3c, CONST_STRPTR, Ac3Version ,\
      , AC3_BASE_NAME)

#endif /* !_INLINE_AC3_H */
