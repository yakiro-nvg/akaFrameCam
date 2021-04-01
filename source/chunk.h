/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_CHUNK_H_
#define _CAM_CHUNK_H_

#include "prereq.h"

namespace akaFrame{ namespace cam {

/*****************************************************************************/
enum struct opcode_t : u8 { /*                                               */
/* name          args        |   description                                 */
/*****************************************************************************/
Nop, /*                      |   no operation                                */
/*---------------------------------------------------------------------------*/
EnsureSlots, /*  D           |   grows S to at least D slots                 */
/*---------------------------------------------------------------------------*/
Pop, /*          Bx          |   pop Bx values                               */
Load, /*         Bx          |   push S[Bx]                                  */
LoadImport, /*   Bx          |   push I[Bx]                                  */
LoadConstant, /* Bx          |   push K[Bx]                                  */
LoadTupleAt, /*  Bx          |   push S[-1]:Tuple[Bx]                        */
LoadComp2, /*    sAx         |   push sAx:Comp-2                             */
LoadComp4, /*    sAx         |   push sAx:Comp-4                             */
LoadTrue, /*                 |   push true                                   */
LoadFalse, /*                |   push false                                  */
/*---------------------------------------------------------------------------*/
Call, /*         A B         |   call S[-A - 1] usings S[-1], S[-2] to S[-A] */
/*                           |                  returnings push R[1] to R[B] */
Goback, /*                   |   goback                                      */
/*---------------------------------------------------------------------------*/
Set, /*          D E         |   S[D] = S[E]                                 */
}; /* enum opcode_t **********************************************************/

/** Instructions are unsigned 32-bit integers.
All instructions have an opcode in the first 7 bits.
Instructions can have the following formats:
      iABC      C(8)  B(8) k(1)  A(8)   opcode(7)
      iABx            Bx(17)     A(8)   opcode(7)
      iAsBx   signed sBx(17)     A(8)   opcode(7)
      isAx              signed  sAx(25) opcode(7)
      iDE     _(1)    D(12)      E(12)  opcode(7) */
union instruction_t {
struct {
        u32 opcode : 7;
        u32 _      : 25;
} i;
struct {
        u32 opcode : 7;
        u32 A      : 8;
        u32 k      : 1;
        u32 B      : 8;
        u32 C      : 8;
} iABC;
struct {
        u32 opcode : 7;
        u32 A      : 8;
        u32 Bx     : 17;
} iABx;
struct {
        u32 opcode : 7;
        u32 A      : 8;
        i32 sBx    : 17;
} iAsBx;
struct {
        u32 opcode : 7;
        i32 sAx    : 25;
} isAx;
struct {
        u32 opcode : 7;
        u32 D      : 12;
        u32 E      : 12;
        u32 _      : 1;
} iDE;
}; // union instruction_t

}} // namespace akaFrame.cam

#endif // !_CAM_CHUNK_H_
