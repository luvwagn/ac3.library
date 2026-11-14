/* Written in the form sfdc 1.13 makes from ac3_lib.sfd. MIT, (c) 2026 Brian King. */

#ifndef CLIB_AC3_PROTOS_H
#define CLIB_AC3_PROTOS_H

#include <exec/types.h>
#include <libraries/ac3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

APTR Ac3Open(ULONG mode, ULONG flags);
VOID Ac3Close(APTR ac3);
LONG Ac3SyncInfo(CONST UBYTE *buf, struct Ac3Info *info);
LONG Ac3Find(CONST UBYTE *buf, LONG len, struct Ac3Info *info);
LONG Ac3Decode(APTR ac3, CONST UBYTE *frame, WORD *pcm, struct Ac3Info *info);
CONST_STRPTR Ac3Version(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIB_AC3_PROTOS_H */
