/*
 * C header file for the floating-point units' controls and flags.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivcc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _FPU_H
#define _FPU_H

// What a function returns when it does not do what it was asked.
#define _FPU_FAILED 1

// The exception flags, the same bits in the x87 status word and MXCSR, the
// five C99 names, and all six with the denormal flag.
#define _FPU_INVALID   0x01
#define _FPU_DIVBYZERO 0x04
#define _FPU_OVERFLOW  0x08
#define _FPU_UNDERFLOW 0x10
#define _FPU_INEXACT   0x20
#define _FPU_EXCEPTS   0x3D
#define _FPU_FLAGS     0x3F

// The rounding directions as the x87 control word holds them, and its field.
#define _FPU_TONEAREST  0x0000
#define _FPU_DOWNWARD   0x0400
#define _FPU_UPWARD     0x0800
#define _FPU_TOWARDZERO 0x0C00
#define _FPU_ROUND_MASK 0x0C00

// How far above the x87 control word's fields MXCSR holds the same ones.
#define _FPU_SSE_ROUND_SHIFT 3
#define _FPU_SSE_MASK_SHIFT  7

// The dwords of the x87 environment that hold its last pointers and opcode.
#define _FPU_X87_POINTERS 4

// The environment a program starts with, as an environment's address.
#define _FPU_DEFAULT_ENV ((const _Fpu_Env *) -1)

// Both units' environments, the x87's image and then MXCSR.
typedef struct _Fpu_Env _Fpu_Env;
struct _Fpu_Env {
    unsigned short fe_control;                    // the x87 control word
    unsigned short fe_unused1;                    // reserved
    unsigned short fe_status;                     // the x87 status word
    unsigned short fe_unused2;                    // reserved
    unsigned short fe_tags;                       // the x87 tag word
    unsigned short fe_unused3;                    // reserved
    unsigned int fe_pointers[_FPU_X87_POINTERS];  // the last opcode and addresses
    unsigned int fe_mxcsr;                        // SSE's control and status
};

// Both units' environments, and a division on SSE.
void  _Fpu_StoreEnv(_Fpu_Env *env);
void  _Fpu_LoadEnv(const _Fpu_Env *env);
float _Fpu_DivideSingle(float a, float b);

// The flags.
void _Fpu_ClearFlags(int flags);
void _Fpu_RaiseFlags(int flags);
void _Fpu_WriteFlags(int flags, int mask);
int  _Fpu_TestFlags(int flags);

// The rounding direction.
int _Fpu_ReadRound(void);
int _Fpu_WriteRound(int round);

// The whole environment.
void _Fpu_HoldEnv(_Fpu_Env *env);
void _Fpu_WriteEnv(const _Fpu_Env *env);

#endif // _FPU_H
