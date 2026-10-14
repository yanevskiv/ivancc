/*
 * C header file for floating-point formats and arithmetic.
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
 * You should have received a copy of the GNU General Public License
 * along with ivcc.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef FP_H
#define FP_H

// Standard headers.
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Bits in a byte, for splitting a significand into bytes.
#define FP_BITS_PER_BYTE 8

// Bits in a wide integer and in each of its two words.
#define FP_WIDE_BITS 128
#define FP_WORD_BITS 64

// The top bit of a word, a normalized significand's leading bit.
#define FP_WORD_TOP ((uint64_t) 1 << 63)

// Bits in half a word, and the mask of them, the parts a product is taken in.
#define FP_HALF_BITS 32
#define FP_HALF_MASK 0xFFFFFFFFU

// Quotient bits a division develops: a 64-bit significand, rounding and room.
#define FP_ARITH_QUOTIENT_BITS 68

// Bytes the x87 extended format occupies.
#define FP_EXTENDED_SIZE 10

// Offset of the sign and exponent after the significand's bytes.
#define FP_EXTENDED_TOP_OFF 8

// The extended format's exponent bias and its all-ones exponent.
#define FP_EXTENDED_BIAS    16383
#define FP_EXTENDED_EXP_MAX 0x7FFF

// The integer bit, and the quiet bit a NaN carries below it.
#define FP_EXTENDED_INT_BIT   ((uint64_t) 1 << 63)
#define FP_EXTENDED_QUIET_BIT ((uint64_t) 1 << 62)

// The sign bit of the extended format's top two bytes.
#define FP_EXTENDED_SIGN_BIT 0x8000

// An unsigned integer of 128 bits, which C99 lacks.
typedef struct Fp_Wide Fp_Wide;
struct Fp_Wide {
    uint64_t fw_hi; // its upper 64 bits
    uint64_t fw_lo; // its lower 64 bits
};

// A binary interchange format, as IEEE 754 describes one.
typedef struct Fp_Format Fp_Format;
struct Fp_Format {
    int32_t ff_bits;     // the bits it occupies
    int32_t ff_mant;     // the bits of its significand, the leading one too
    int32_t ff_min;      // the least exponent of a normal number
    int32_t ff_max;      // the greatest exponent, its bias too
    bool    ff_explicit; // whether it stores its significand's leading bit
};

// What kind of number a value is.
typedef enum Fp_Class Fp_Class;
enum Fp_Class {
    FP_CLASS_ZERO,       // a zero of either sign
    FP_CLASS_SUBNORMAL,  // finite and below its format's least normal number
    FP_CLASS_NORMAL,     // finite and nonzero, and not known subnormal
    FP_CLASS_INFINITY,   // an infinity of either sign
    FP_CLASS_NAN,        // not a number
    FP_CLASS_UNSUPPORTED // no number: binary80's integer bit clear
};

// A number's sign.
typedef enum Fp_Sign Fp_Sign;
enum Fp_Sign {
    FP_SIGN_POSITIVE, // its sign bit clear
    FP_SIGN_NEGATIVE  // its sign bit set
};

// A number unpacked from its format, or computed exactly before rounding.
typedef struct Fp_Value Fp_Value;
struct Fp_Value {
    Fp_Class fv_class; // what kind of number it is
    Fp_Sign  fv_sign;  // its sign
    int32_t  fv_exp;   // the power of two its significand's top bit weighs
    Fp_Wide  fv_mant;  // its significand from bit 127 down, or a NaN's fraction
};

// The directions a result is rounded in.
typedef enum Fp_RoundMode Fp_RoundMode;
enum Fp_RoundMode {
    FP_ROUND_NEAREST_EVEN, // to the nearest, a tie to the even one
    FP_ROUND_DOWN,         // toward negative infinity
    FP_ROUND_UP,           // toward positive infinity
    FP_ROUND_ZERO,         // toward zero
    FP_ROUND_NEAREST_AWAY  // to the nearest, a tie away from zero
};

// When a nonzero result is tiny, which IEEE 754 leaves to the machine.
typedef enum Fp_RoundTininess Fp_RoundTininess;
enum Fp_RoundTininess {
    FP_ROUND_TINY_BEFORE, // below the least normal before rounding
    FP_ROUND_TINY_AFTER   // below it after rounding with an unbounded exponent
};

// What lies below the bits a rounding keeps, against half their last one.
typedef enum Fp_RoundRest Fp_RoundRest;
enum Fp_RoundRest {
    FP_ROUND_REST_NONE,  // nothing
    FP_ROUND_REST_BELOW, // less than half
    FP_ROUND_REST_HALF,  // exactly half
    FP_ROUND_REST_ABOVE  // more than half
};

// The flags an operation raises: IEEE 754's five, tininess and the rounding's
// direction.
typedef enum Fp_Flag Fp_Flag;
enum Fp_Flag {
    FP_FLAG_INVALID    = 0x01, // no useful result: a NaN, 0/0, inf-inf, 0*inf
    FP_FLAG_DIVIDE     = 0x02, // an exact infinity from finite operands
    FP_FLAG_OVERFLOW   = 0x04, // a result past the greatest finite number
    FP_FLAG_UNDERFLOW  = 0x08, // a result tiny and inexact
    FP_FLAG_INEXACT    = 0x10, // a result rounded
    FP_FLAG_TINY       = 0x20, // a result tiny, exact or not, for a trap
    FP_FLAG_ROUNDED_UP = 0x40  // a result rounded away from zero, x87's C1
};

// How an operation rounds, and the flags it raised.
typedef struct Fp_Context Fp_Context;
struct Fp_Context {
    Fp_RoundMode     fc_mode;      // the direction a result is rounded in
    Fp_RoundTininess fc_tininess;  // when a result is tiny
    uint32_t         fc_flags;     // the Fp_Flag bits raised, never cleared
};

// How two values compare.
typedef enum Fp_Order Fp_Order;
enum Fp_Order {
    FP_ORDER_LESS,     // the first is less
    FP_ORDER_EQUAL,    // they are equal
    FP_ORDER_GREATER,  // the first is greater
    FP_ORDER_UNORDERED // either is a NaN
};

// Which NaNs a comparison finds invalid.
typedef enum Fp_CompareKind Fp_CompareKind;
enum Fp_CompareKind {
    FP_COMPARE_QUIET,      // a signalling NaN alone
    FP_COMPARE_SIGNALLING  // any NaN
};

// The binary32 and binary64 formats, and x87's extended format.
// [Amd64-Vol1:6.3.2.1]: Figure 6-8, the double-extended-precision format
// [Amd64-Vol1:6.3.4.2]: Table 6-9, the unsupported encodings, its integer bit clear
extern const Fp_Format Fp_FormatBinary32;
extern const Fp_Format Fp_FormatBinary64;
extern const Fp_Format Fp_FormatBinary80;

// Wide integers
Fp_Wide Fp_WideMake(uint64_t hi, uint64_t lo);
bool    Fp_WideIsZero(Fp_Wide a);
int32_t Fp_WideCmp(Fp_Wide a, Fp_Wide b);
Fp_Wide Fp_WideAdd(Fp_Wide a, Fp_Wide b);
Fp_Wide Fp_WideSub(Fp_Wide a, Fp_Wide b);
Fp_Wide Fp_WideMul(uint64_t a, uint64_t b);
Fp_Wide Fp_WideShl(Fp_Wide a, int32_t count);
Fp_Wide Fp_WideShr(Fp_Wide a, int32_t count);
Fp_Wide Fp_WideShrSticky(Fp_Wide a, int32_t count);
Fp_Wide Fp_WideMask(int32_t count);
Fp_Wide Fp_WideAnd(Fp_Wide a, Fp_Wide b);
Fp_Wide Fp_WideOr(Fp_Wide a, Fp_Wide b);
int32_t Fp_WideCountZeros(Fp_Wide a);

// Formats
Fp_Value Fp_FormatMake(Fp_Sign sign, Fp_Wide mant, int32_t low);
Fp_Value Fp_FormatUnpack(const Fp_Format *fmt, Fp_Wide bits);
Fp_Wide  Fp_FormatPack(const Fp_Format *fmt, const Fp_Value *value);
bool     Fp_FormatIsNan(const Fp_Value *value);
bool     Fp_FormatIsSignalling(const Fp_Value *value);

// Rounding
bool     Fp_RoundIsUp(const Fp_Context *ctx, Fp_Sign sign, Fp_Wide kept, Fp_RoundRest rest);
Fp_Value Fp_RoundToBits(const Fp_Value *value, int32_t keep, Fp_Context *ctx);
Fp_Value Fp_RoundValue(const Fp_Value *exact, const Fp_Format *fmt, Fp_Context *ctx);

// Arithmetic
Fp_Value Fp_ArithInvalid(Fp_Context *ctx);
Fp_Value Fp_ArithNan(const Fp_Value *a, const Fp_Value *b, Fp_Context *ctx);
Fp_Value Fp_ArithAdd(const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx);
Fp_Value Fp_ArithSub(const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx);
Fp_Value Fp_ArithMul(const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx);
Fp_Value Fp_ArithDiv(const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx);
Fp_Order Fp_ArithCmp(const Fp_Value *a, const Fp_Value *b, Fp_CompareKind kind, Fp_Context *ctx);

// Integers
uint64_t Fp_ToInteger(const Fp_Value *value, int32_t bits, Fp_Context *ctx);
Fp_Value Fp_ToValue(int64_t integer, const Fp_Format *fmt, Fp_Context *ctx);

// The host's float and double, as binary32 and binary64 bits
uint32_t Fp_HostFloatBits(float value);
uint64_t Fp_HostDoubleBits(double value);
float    Fp_HostFloatFromBits(uint32_t bits);
double   Fp_HostDoubleFromBits(uint64_t bits);

// The host's long double, as the x87 80-bit extended format
void Fp_HostEncodeExtended(long double value, uint8_t *out);

#endif // FP_H
