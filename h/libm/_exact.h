/*
 * C header file for exact values and long double fields.
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

#ifndef _EXACT_H
#define _EXACT_H

// The bytes of a long double that hold its significand, its integer bit
// explicit at the top, and the two above them, its sign and exponent.
#define _EXACT_LDOUBLE_SIGNIFICAND 0
#define _EXACT_LDOUBLE_TOP         8
#define _EXACT_LDOUBLE_INT_SHIFT   63

// A long double's integer bit, its fraction below it, its exponent bias,
// its largest biased exponent and its least normal exponent.
#define _EXACT_LDOUBLE_INT_BIT   0x8000000000000000ULL
#define _EXACT_LDOUBLE_FRAC_MASK 0x7FFFFFFFFFFFFFFFULL
#define _EXACT_LDOUBLE_BIAS      16383
#define _EXACT_LDOUBLE_EXP_MAX   0x7FFFU
#define _EXACT_LDOUBLE_EXP_LOW   (-16382)

// The sign bit of a long double's top 16 bits.
#define _EXACT_TOP_SIGN 0x8000U

// Every bit of 64, and a long double's positive infinity.
#define _EXACT_ALL_BITS (~0ULL)
#define _EXACT_INFINITY (__builtin_infl())

// A wide integer's limbs, the bits of each, all its bits, and its lowest.
#define _EXACT_WIDE_LIMBS 12
#define _EXACT_LIMB_BITS  32
#define _EXACT_WIDE_BITS  (_EXACT_WIDE_LIMBS * _EXACT_LIMB_BITS)
#define _EXACT_LOW_BIT    1U

// The top bit of a wide integer that is zero.
#define _EXACT_WIDE_NONE (-1)

// The bits a root is developed to, a round and a guard bit past 64.
#define _EXACT_ROOT_BITS 66

// The bits of a sum kept exact below its top, the rest sticky.
#define _EXACT_SUM_WINDOW 320

// Which way a step moves a long double's magnitude.
typedef enum _Exact_Step _Exact_Step;
enum _Exact_Step {
    _EXACT_STEP_UP,  // away from zero
    _EXACT_STEP_DOWN // toward zero
};

// Which type a value is rounded to.
typedef enum _Exact_Type _Exact_Type;
enum _Exact_Type {
    _EXACT_TYPE_FLOAT,  // float's 24 bits
    _EXACT_TYPE_DOUBLE, // double's 53 bits
    _EXACT_TYPE_LDOUBLE // long double's 64 bits
};

// Which root is taken, by its degree.
typedef enum _Exact_Root _Exact_Root;
enum _Exact_Root {
    _EXACT_ROOT_SQUARE = 2, // a square root
    _EXACT_ROOT_CUBE   = 3  // a cube root
};

// An unsigned integer of limbs, the least significant first.
typedef struct _Exact_Wide _Exact_Wide;
struct _Exact_Wide {
    unsigned int ew_limb[_EXACT_WIDE_LIMBS]; // the limbs, each 32 bits
};

// An exact value, its significand times 2 to its unit.
typedef struct _Exact_Value _Exact_Value;
struct _Exact_Value {
    _Exact_Wide ev_sig;      // the significand, an integer
    int         ev_unit;     // the exponent of its lowest bit
    _Bool       ev_negative; // whether its sign is negative
    _Bool       ev_sticky;   // whether nonzero bits lie below its lowest
};

// A long double's fields, significand and exponent, sign and class.
unsigned long long _Exact_ReadSignificand(long double x);
unsigned int       _Exact_ReadTop(long double x);
long double        _Exact_MakeLDouble(unsigned int top, unsigned long long sig);
unsigned long long _Exact_SplitLDouble(long double x, int *exp);
long double        _Exact_JoinLDouble(int exp, unsigned long long sig);
long double        _Exact_StepLDouble(long double x, _Exact_Step step);
_Bool              _Exact_IsNegative(long double x);
_Bool              _Exact_IsNan(long double x);
_Bool              _Exact_IsInf(long double x);
long double        _Exact_ClearSign(long double x);
long double        _Exact_CopySign(long double x, long double y);

// A wide integer's arithmetic.
void               _Exact_SetWide(_Exact_Wide *w, unsigned long long low);
unsigned long long _Exact_ReadWideLow(const _Exact_Wide *w);
unsigned int       _Exact_ReadWideBit(const _Exact_Wide *w, int at);
int                _Exact_FindWideTop(const _Exact_Wide *w);
void               _Exact_ShiftWideLeft(_Exact_Wide *w, int n);
_Bool              _Exact_ShiftWideRight(_Exact_Wide *w, int n);
_Bool              _Exact_AlignWide(_Exact_Wide *w, int n);
int                _Exact_CompareWides(const _Exact_Wide *a, const _Exact_Wide *b);
void               _Exact_AddWide(_Exact_Wide *w, const _Exact_Wide *add);
void               _Exact_SubtractWide(_Exact_Wide *w, const _Exact_Wide *sub);
void               _Exact_MultiplyWides(_Exact_Wide *to, const _Exact_Wide *a, const _Exact_Wide *b);
void               _Exact_RaiseWide(_Exact_Wide *to, const _Exact_Wide *base, int k);
unsigned int       _Exact_DivideWideSmall(_Exact_Wide *w, unsigned int d);
void               _Exact_DivideWides(_Exact_Wide *to, const _Exact_Wide *a, const _Exact_Wide *b);

// Exact values and their one rounding.
void               _Exact_MakeValue(_Exact_Value *exact, long double x);
void               _Exact_MultiplyValues(_Exact_Value *product, const _Exact_Value *a, const _Exact_Value *b);
void               _Exact_AddValues(_Exact_Value *sum, const _Exact_Value *a, const _Exact_Value *b);
void               _Exact_ExtractRoot(_Exact_Value *root, const _Exact_Value *radicand, _Exact_Root degree);
unsigned long long _Exact_RoundBits(const _Exact_Value *exact, int keep, int *low, _Bool *inexact);
long double        _Exact_RoundValue(const _Exact_Value *exact, _Exact_Type type);

#endif // _EXACT_H
