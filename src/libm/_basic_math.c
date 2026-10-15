/*
 * C source file for the basic mathematical functions.
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


// Module header.
#include <_basic_math.h>

// Includes.
#include <_err.h>
#include <_exact.h>
#include <_fp.h>
#include <_fpu.h>
#include <_str.h>
#include <_sys.h>

// The terms of e to r less 1 over r, 1/k! from k = 1.
const long double _Basic_Math_ExpTerms[_BASIC_MATH_EXP_TERMS] = {
    0x8p-3L, 0x8p-4L, 0xa.aaaaaaaaaaaaaabp-6L, 0xa.aaaaaaaaaaaaaabp-8L, 0x8.888888888888889p-10L,
    0xb.60b60b60b60b60bp-13L, 0xd.00d00d00d00d00dp-16L, 0xd.00d00d00d00d00dp-19L, 0xb.8ef1d2ab6399c7dp-22L,
    0x9.3f27dbbc4fae397p-25L, 0xd.7322b3faa271c7fp-29L, 0x8.f76c77fc6c4bdaap-32L, 0xb.092309d43684be5p-36L,
    0xc.9cba54603e4e906p-40L, 0xd.73f9f399dc0f88fp-44L
};

// The terms of the logarithm's series, 1/(2k + 1) from k = 1.
const long double _Basic_Math_LogTerms[_BASIC_MATH_LOG_TERMS] = {
    0xa.aaaaaaaaaaaaaabp-5L, 0xc.ccccccccccccccdp-6L, 0x9.249249249249249p-6L, 0xe.38e38e38e38e38ep-7L,
    0xb.a2e8ba2e8ba2e8cp-7L, 0x9.d89d89d89d89d8ap-7L, 0x8.888888888888889p-7L, 0xf.0f0f0f0f0f0f0f1p-8L,
    0xd.79435e50d79435ep-8L, 0xc.30c30c30c30c30cp-8L, 0xb.21642c8590b2164p-8L, 0xa.3d70a3d70a3d70ap-8L,
    0x9.7b425ed097b425fp-8L
};

// The natural logarithm of each step of the logarithm's table, its top 64
// bits.
const long double _Basic_Math_LogStepHigh[_BASIC_MATH_LOG_STEP_COUNT] = {
    -0xa.9157039c51ebe71p-5L, -0x9.34b1089a6dc93c2p-5L, -0xf.cc8e3659d9bcbedp-6L, -0xd.49f69e456cf1b79p-6L,
    -0xa.dfa035aa1ed8fdcp-6L, -0x8.8bc74113f23def2p-6L, -0xc.99af2eaca4c4571p-7L, -0x8.42cc5acf1d03445p-7L,
    -0x8.20aec4f3a222381p-8L, 0x0p+0L,                  0xf.c14d873c1980268p-9L,  0xf.85186008b15330cp-8L,
    0xb.78694572b5a5cdfp-7L,  0xf.1383b7157972f4fp-7L,  0x9.4aa97c0ffa91a6p-6L,   0xa.ff983853c9e9e44p-6L,
    0xc.a92d4e7a2b5a3b2p-6L,  0xe.47fbe3cd4d10d61p-6L,  0xf.dc8c36af1f1546bp-6L,  0x8.b3ae55d5d30701dp-5L,
    0x9.74715d708e984e1p-5L,  0xa.30c5e10e2f613e8p-5L,  0xa.e8dedfac04e5284p-5L
};

// The natural logarithm of each step of the logarithm's table, the rest.
const long double _Basic_Math_LogStepLow[_BASIC_MATH_LOG_STEP_COUNT] = {
    0xf.d36714d2f2bbbeep-70L,  0x8.2913127eaac7abap-71L,  0xd.7cc833faf3927c5p-72L,  -0xb.ea77a5c80dccdcfp-71L,
    -0xa.4bb3f20818b6961p-73L, 0xc.74be038d217e1c4p-71L,  0xa.85704ccb6bdda33p-74L,  -0xf.f66fd40cdcb04bfp-74L,
    0x8.c38a2b2777621f2p-73L,  0x0p+0L,                   -0xe.07d87086eb028a9p-75L, -0xc.da3a4453343b396p-75L,
    0x9.3373da336c819cap-73L,  0xa.87ffe1fe9e155dcp-72L,  0xb.b8e203edf4d10ap-72L,   -0xc.1df5f8dedc45019p-71L,
    0x9.83a9c5c4b3b1328p-74L,  0xb.b03de5ff734495cp-72L,  -0xa.b993c86b2d35f6ep-71L, -0xc.e0aa3be4747dc1p-72L,
    0xc.c91a85081b3cdeep-70L,  0xb.7b37b33c734415ep-70L,  0xd.8e0f71ff84567cep-70L
};

// The terms of the sine's series, (-1)^k/(2k + 1)! from k = 1.
const long double _Basic_Math_SinTerms[_BASIC_MATH_SIN_TERMS] = {
    -0xa.aaaaaaaaaaaaaabp-6L, 0x8.888888888888889p-10L, -0xd.00d00d00d00d00dp-16L, 0xb.8ef1d2ab6399c7dp-22L,
    -0xd.7322b3faa271c7fp-29L, 0xb.092309d43684be5p-36L, -0xd.73f9f399dc0f88fp-44L, 0xc.a963b81856a5359p-52L,
    -0x9.7a4da340a0ab926p-60L
};

// The terms of the cosine's series, (-1)^k/(2k)! from k = 1.
const long double _Basic_Math_CosTerms[_BASIC_MATH_COS_TERMS] = {
    -0x8p-4L, 0xa.aaaaaaaaaaaaaabp-8L, -0xb.60b60b60b60b60bp-13L, 0xd.00d00d00d00d00dp-19L, -0x9.3f27dbbc4fae397p-25L,
    0x8.f76c77fc6c4bdaap-32L, -0xc.9cba54603e4e906p-40L, 0xd.73f9f399dc0f88fp-48L, -0xb.413c31dcbecbbdep-56L,
    0xf.2a15d201011283dp-65L
};

// The arc tangent of each step of the arc tangent's table, its top 64 bits.
const long double _Basic_Math_AtanStepHigh[_BASIC_MATH_ATAN_STEP_COUNT] = {
    0x0p+0L, 0xf.faaddb967ef4e37p-8L, 0xf.eadd4d5617b6e33p-7L, 0xb.dcbda5e72d81134p-6L, 0xf.adbafc96406eb15p-6L,
    0x9.b13b9b83f5e5e6ap-5L, 0xb.7b0ca0f26f78474p-5L, 0xd.327761e611fe5b6p-5L, 0xe.d63382b0dda7b45p-5L,
    0x8.32bf4a6d9867e2ap-4L, 0x8.f005d5ef7f59f9bp-4L, 0x9.a2f80e671bdda2p-4L, 0xa.4bc7d1934f70924p-4L,
    0xa.eac4c38b4d8c08p-4L, 0xb.8053e2bc2319e74p-4L, 0xc.0ce85b8ac526641p-4L, 0xc.90fdaa22168c235p-4L
};

// The arc tangent of each step of the arc tangent's table, the rest.
const long double _Basic_Math_AtanStepLow[_BASIC_MATH_ATAN_STEP_COUNT] = {
    0x0p+0L, -0xd.361b48fc747cabbp-74L, -0xd.da19d8305ddc422p-73L, 0xf.6169f103938e90fp-71L, 0xd.b8f3debef442fcbp-71L,
    -0xe.9512d9cb614336p-71L, -0xe.ab9bdba460376fap-70L, 0x8.4f92bd2003ce26dp-70L, 0xd.fc88bd978751a07p-70L,
    0x9.6d41396c34a2b82p-69L, 0xb.906bc2ccb886e8fp-69L, 0x8.44df1c4409fe77ap-69L, 0xc.d43f9522bed64f7p-71L,
    0xa.392f179f290385p-71L, -0xd.3496ab7bd6eef0bp-70L, -0xe.c453a7722da0bb6p-69L, -0xe.ce675d1fc8f8cbbp-70L
};

// The terms of erf's Maclaurin series, 2/sqrt(pi) (-1)^k/(k!(2k + 1)) from
// k = 1.
const long double _Basic_Math_ErfTerms[_BASIC_MATH_ERF_TERMS] = {
    -0xc.093a3581bcf3612p-5L,  0xe.71790d0215f0dafp-7L,   -0xd.c167189445a86fp-9L,   0xa.b2de6873529f72cp-11L,
    -0xe.016da00205c2c4dp-14L, 0xf.cd1937cdc4dbbd3p-17L,  -0xf.a692e1f06175af3p-20L, 0xd.cf364ee328d1404p-23L,
    -0xa.fba17621f38cf8bp-26L, 0xf.e62b9031260de6dp-30L,  -0xa.8eb8c0e2e9b6424p-33L, 0xc.f356af2ad397e41p-37L,
    -0xe.c229e5862374ef7p-41L
};

// e to x^2 times erfc of each node of erfc's table, its top 64 bits.
const long double _Basic_Math_ErfcValueHigh[_BASIC_MATH_ERFC_NODE_COUNT] = {
    0x9.d9de1e4c5879655p-4L, 0x8.1c6aa751ec19c28p-4L, 0xd.aec3c07cab590c1p-5L, 0xb.c534909c3b3d005p-5L,
    0xa.4a6d7fd156b4273p-5L, 0x9.1e7e178ee3f013bp-5L, 0x8.2c338da963baf62p-5L, 0xe.ca2236b13c98e58p-6L,
    0xd.7dd9f9db9a1d661p-6L, 0xc.64f5b47f93eb489p-6L, 0xb.74c13e914e9667p-6L,  0xa.a53d046a5da27d3p-6L,
    0x9.f054cd077489f9cp-6L, 0x9.51578ce0a497c0cp-6L, 0x8.c4995f8470aa213p-6L
};

// e to x^2 times erfc of each node of erfc's table, the rest.
const long double _Basic_Math_ErfcValueLow[_BASIC_MATH_ERFC_NODE_COUNT] = {
    0xf.529dbcefe16215cp-69L,  -0x8.4d3614599434167p-69L, 0xa.88fe4634cddb0adp-71L,  0xf.2e801e32323abfdp-72L,
    0xf.7c3c363883cfdacp-70L,  0x8.207598c611f81c3p-72L,  0xf.871fc40eceb0c54p-71L,  -0xc.c5410b2247b3ad2p-73L,
    -0x8.17942848cb14912p-71L, 0xc.6270e6d72c1f84ap-71L,  -0xe.a7bce26b25cbde5p-71L, -0x8.3644cdaf7239ecfp-71L,
    -0xb.2de5db1979d1599p-74L, -0x9.40d467e516adb2dp-74L, 0xd.5093a3e73c4f2edp-76L
};

// The slope of e to x^2 times erfc at each node, its top 64 bits.
const long double _Basic_Math_ErfcSlopeHigh[_BASIC_MATH_ERFC_NODE_COUNT] = {
    -0x8.33f931f642f3ac5p-4L, -0xb.c66eaa8f728cdbfp-5L, -0x8.be271f8be0280b3p-5L, -0xd.5d566dfd4583452p-6L,
    -0xa.78cc4222657b5b7p-6L, -0x8.61ea9c223223bcep-6L, -0xd.ab81a77e9fb92b3p-7L, -0xb.538695d9ba087cp-7L,
    -0x9.8436bf80d64291p-7L,  -0x8.182bc099804c6fp-7L,  -0xd.eb57268a58b7739p-8L, -0xc.1542915c3450647p-8L,
    -0xa.942e9758f7b9c9fp-8L, -0x9.553281d66d08847p-8L, -0x8.4a49139ba18cf4ep-8L
};

// The slope of e to x^2 times erfc at each node, the rest.
const long double _Basic_Math_ErfcSlopeLow[_BASIC_MATH_ERFC_NODE_COUNT] = {
    -0xd.2284e2eb1e4ebb5p-69L, 0xe.2e18833d44021a9p-70L,  -0xe.6146f9813186973p-70L, 0x9.2cad329600315d5p-74L,
    0xb.14dec5e7181bdcap-71L,  -0xe.6ef2d0457d444b1p-73L, -0xe.e02dbcc31000bcap-73L, -0xc.9ceafca3db59b67p-77L,
    0xb.6b216e5013abcc8p-72L,  0x9.2389d8cdcfa855fp-73L,  -0xe.17be6f770c53ce4p-74L, 0xc.ab6479f9ac437ecp-75L,
    -0xe.72ce7c892569c9cp-73L, -0xa.0546834b10186eep-73L, 0xb.fcd34c6e82843d1p-74L
};

// The terms of lgamma's Taylor series at 2, 1 less Euler's constant, then
// (-1)^k (zeta(k) - 1)/k from k = 2.
const long double _Basic_Math_GammaTerms[_BASIC_MATH_GAMMA_TERMS] = {
    0xd.8773039049e70b6p-5L,   0xa.51a6625307d3231p-5L,   -0x8.9f000d2abb03409p-7L,  0xa.8991563ec241b6p-9L,
    -0xf.2027e10c7af8c37p-11L, 0xb.d6eb756db617ea5p-12L,  -0x9.c562e15fc703e76p-13L, 0x8.59b57c31cb745f3p-14L,
    -0xe.9fea63b697e3e38p-16L, 0xd.093d878beb2d19dp-17L,  -0xb.c6f2debe40f7797p-18L, 0xa.c06e77337581126p-19L,
    -0x9.e5e4b1e7112142bp-20L, 0x9.2cbd1cf9a555c81p-21L,  -0x8.8d975bb3caa08e4p-22L, 0x8.03266f5917879dp-23L,
    -0xf.13006c9e7e975dap-25L, 0xe.3b5dd9f83d26bb3p-26L,  -0xd.7ad365dfc54bb2cp-27L, 0xc.cdc9e1038587a06p-28L,
    -0xc.31639a6f9f56366p-29L, 0xb.a34ed667d6e6593p-30L,  -0xb.21a54223d75681bp-31L, 0xa.aad43bffe9614f1p-32L,
    -0xa.3d8b3c92c68720ap-33L, 0x9.d8ae9597e085e28p-34L,  -0x9.7b4d4fd5f1efcbdp-35L, 0x9.2499519ba1a620cp-36L,
    -0x8.d3e13761291e29fp-37L, 0x8.888b7349f6cbc72p-38L,  -0x8.421265e2a1ec141p-39L, 0x8.0001371fb227a6bp-40L,
    -0xf.83e28a7e4f8505dp-42L, 0xf.0f1013557e6bd3bp-43L,  -0xe.a0eab72f7170ebcp-44L
};

// The terms of Stirling's series, B2k/(2k(2k - 1)) from k = 1.
const long double _Basic_Math_StirlingTerms[_BASIC_MATH_STIRLING_TERMS] = {
    0xa.aaaaaaaaaaaaaabp-7L,   -0xb.60b60b60b60b60bp-12L, 0xd.00d00d00d00d00dp-14L,  -0x9.c09c09c09c09c0ap-14L,
    0xd.ca8f158c7f91ab8p-14L,  -0xf.b5586ccc9e3e41p-13L,  0xd.20d20d20d20d20dp-11L,  -0xf.21436587a9cbee1p-9L,
    0xb.7f4b1c0f033ffd1p-6L,   -0xb.23b3808c0f9cf6ep-3L
};

// lgamma's zeros from -2 to -24, their top 64 bits.
const long double _Basic_Math_GammaZeroHigh[_BASIC_MATH_GAMMA_ZERO_COUNT] = {
    -0x9.d3fe4b007c360abp-2L, -0xa.fda0850dec8065ep-2L, -0xc.9306de4f2cd7beep-2L, -0xf.d238aa3e17f285cp-2L,
    -0x8.14273c2ccac0618p-1L, -0x9.fbabbd37757e6a2p-1L, -0xa.04352bf85b6c865p-1L, -0xb.ff497ac8fa06afcp-1L,
    -0xc.00b592c4be4676cp-1L, -0xd.ffe5fbb5c377fe8p-1L, -0xe.0019fef6ff0f5bfp-1L, -0xf.fffcbfc0ace7879p-1L,
    -0x8.0001a01459fc9f6p+0L, -0x8.ffffd1c425e81p+0L,   -0x9.00002e3bb47d86dp+0L, -0x9.fffffb606bdfdcdp+0L,
    -0xa.0000049f93bb992p+0L, -0xa.ffffff9466e9f1bp+0L, -0xb.0000006b9915316p+0L, -0xb.fffffff70893874p+0L,
    -0xc.00000008f76c773p+0L, -0xc.ffffffff4f6dcf6p+0L, -0xd.00000000b09230ap+0L, -0xd.fffffffff36345bp+0L,
    -0xe.000000000c9cba5p+0L, -0xe.ffffffffff28c06p+0L, -0xf.0000000000d73fap+0L, -0xf.fffffffffff28cp+0L,
    -0x8.000000000006bap+1L,  -0x8.7fffffffffff9abp+1L, -0x8.800000000000655p+1L, -0x8.fffffffffffffa6p+1L,
    -0x9.00000000000005ap+1L, -0x9.7fffffffffffffbp+1L, -0x9.800000000000005p+1L, -0xap+1L,
    -0xap+1L,                 -0xa.8p+1L,               -0xa.8p+1L,               -0xbp+1L,
    -0xbp+1L,                 -0xb.8p+1L,               -0xb.8p+1L,               -0xcp+1L
};

// lgamma's zeros from -2 to -24, the rest.
const long double _Basic_Math_GammaZeroLow[_BASIC_MATH_GAMMA_ZERO_COUNT] = {
    0x9.b0675072fc769e6p-71L,  -0xc.ad25a320f575fa6p-67L, -0xb.e699ad3d9ba6545p-68L, -0xd.4561291236cc321p-68L,
    -0xe.7c2c3786044ab06p-66L, -0xd.0aa4076988501d8p-68L, -0x9.315b9654e537b32p-66L, 0xa.c4a74d0b38481c7p-66L,
    -0xf.85b2da2c70b970dp-69L, 0xa.765268f700b783p-66L,   0xe.df48c919e1f5536p-66L,  -0xa.7a0ad48ac32a74cp-66L,
    -0xc.b3cec1cec857667p-68L, 0xd.e6c5aa2db698607p-66L,  -0xd.b087fdb86a3bd6fp-65L, -0xc.55cef4a0a8f8d3ap-69L,
    -0xf.68bb2bc2a883c06p-65L, -0xd.b6b34ab6f463417p-66L, 0x9.a69640056fc6d05p-66L,  0xf.0437ca678586d0fp-65L,
    -0xa.b3e07812879c3c9p-67L, -0xb.fcbd2ffe3abeaa4p-67L, 0xf.e65f08b91bf0724p-66L,  0x8.c3cf6b83ec5dd12p-65L,
    -0x8.bd29cebd8ae31efp-65L, -0xc.6604ef30371f89dp-68L, 0xc.6642f1bdf07a161p-68L,  -0xc.18cc43ea25ce5cap-65L,
    0xc.18cc4bd7b680d89p-65L,  -0x9.c47e7a93e1c46a1p-64L, 0x9.c47e7a97778935ap-64L,  0x9.e18ee5f6697ba6ap-67L,
    -0x9.e18ee5f65261d98p-67L, -0x8.5b25cbf5f545ceep-64L, 0x8.5b25cbf5f547e48p-64L,  0xf.2a15d2010112853p-65L,
    -0xf.2a15d2010112828p-65L, 0xb.8dc77b6e7ab8c6p-69L,   -0xb.8dc77b6e7ab8c5fp-69L, 0x8.671cb6dbfc294a3p-73L,
    -0x8.671cb6dbfc294a2p-73L, 0xb.b0da098b1c0ceccp-78L,  -0xb.b0da098b1c0ceccp-78L, 0xf.96780cb97abbe65p-83L
};

// Classify a value by its biased exponent and its fraction.
int _Basic_Math_ClassifyFields(unsigned int exp, unsigned int exp_max, unsigned long long frac)
{
    if (exp == exp_max) {
        return frac ? _BASIC_MATH_FP_NAN : _BASIC_MATH_FP_INFINITE;
    }
    if (exp == 0) {
        return frac ? _BASIC_MATH_FP_SUBNORMAL : _BASIC_MATH_FP_ZERO;
    }
    return _BASIC_MATH_FP_NORMAL;
}

// Classify x as a float.
int _Basic_Math_ClassifyFloat(float x)
{
    unsigned int bits;
    _Str_CopyBytes(&bits, &x, sizeof(bits));
    return _Basic_Math_ClassifyFields((bits >> _BASIC_MATH_FLOAT_EXP_SHIFT) & _BASIC_MATH_FLOAT_EXP_MAX, _BASIC_MATH_FLOAT_EXP_MAX, bits & _BASIC_MATH_FLOAT_FRAC_MASK);
}

// Classify x as a double.
int _Basic_Math_ClassifyDouble(double x)
{
    unsigned long long bits;
    _Str_CopyBytes(&bits, &x, sizeof(bits));
    return _Basic_Math_ClassifyFields((unsigned int) (bits >> _BASIC_MATH_DOUBLE_EXP_SHIFT) & _BASIC_MATH_DOUBLE_EXP_MAX, _BASIC_MATH_DOUBLE_EXP_MAX, bits & _BASIC_MATH_DOUBLE_FRAC_MASK);
}

// Classify x as a long double, the x87's refused encodings as glibc does.
int _Basic_Math_ClassifyLDouble(long double x)
{
    unsigned long long sig;
    unsigned short top;
    _Str_CopyBytes(&sig, (unsigned char *) &x + _EXACT_LDOUBLE_SIGNIFICAND, sizeof(sig));
    _Str_CopyBytes(&top, (unsigned char *) &x + _EXACT_LDOUBLE_TOP, sizeof(top));
    unsigned int exp = top & _EXACT_LDOUBLE_EXP_MAX;

    if (exp == 0) {
        return sig >> _EXACT_LDOUBLE_INT_SHIFT ? _BASIC_MATH_FP_NORMAL : _Basic_Math_ClassifyFields(exp, _EXACT_LDOUBLE_EXP_MAX, sig);
    }
    if (! (sig >> _EXACT_LDOUBLE_INT_SHIFT)) {
        return _BASIC_MATH_FP_NAN;
    }
    return _Basic_Math_ClassifyFields(exp, _EXACT_LDOUBLE_EXP_MAX, sig & _EXACT_LDOUBLE_FRAC_MASK);
}

// Return 1 if x's sign is negative, else 0.
int _Basic_Math_ReadSignFloat(float x)
{
    return ((unsigned char *) &x)[_BASIC_MATH_FLOAT_SIGN_BYTE] >> _BASIC_MATH_SIGN_SHIFT;
}

// Return 1 if x's sign is negative, else 0.
int _Basic_Math_ReadSignDouble(double x)
{
    return ((unsigned char *) &x)[_BASIC_MATH_DOUBLE_SIGN_BYTE] >> _BASIC_MATH_SIGN_SHIFT;
}

// Return 1 if x's sign is negative, else 0.
int _Basic_Math_ReadSignLDouble(long double x)
{
    return ((unsigned char *) &x)[_BASIC_MATH_LDOUBLE_SIGN_BYTE] >> _BASIC_MATH_SIGN_SHIFT;
}

// Return x < y, compared only once known to be ordered.
int _Basic_Math_CompareLess(long double x, long double y)
{
    return ! _Basic_Math_CompareUnordered(x, y) && x < y;
}

// Return x <= y, compared only once known to be ordered.
int _Basic_Math_CompareLessEqual(long double x, long double y)
{
    return ! _Basic_Math_CompareUnordered(x, y) && x <= y;
}

// Return x < y || x > y, compared only once known to be ordered.
int _Basic_Math_CompareLessGreater(long double x, long double y)
{
    return ! _Basic_Math_CompareUnordered(x, y) && x != y;
}

// Return whether x and y are unordered, by != alone, quiet on a quiet NaN.
int _Basic_Math_CompareUnordered(long double x, long double y)
{
    return x != x || y != y;
}

// Scale x by 2 to n, rounding once.
long double _Basic_Math_ScaleLDouble(long double x, int n)
{
    for (int i = 0; i < _BASIC_MATH_SCALE_STEPS && n > _BASIC_MATH_LDOUBLE_EXP_HIGH; i++) {
        x *= _BASIC_MATH_LDOUBLE_SCALE_UP;
        n -= _BASIC_MATH_LDOUBLE_EXP_HIGH;
    }
    for (int i = 0; i < _BASIC_MATH_SCALE_STEPS && n < _BASIC_MATH_LDOUBLE_EXP_LOW; i++) {
        x *= _BASIC_MATH_LDOUBLE_SCALE_DOWN;
        n += _BASIC_MATH_LDOUBLE_STEP_DOWN;
    }
    n = n > _BASIC_MATH_LDOUBLE_EXP_HIGH ? _BASIC_MATH_LDOUBLE_EXP_HIGH : n < _BASIC_MATH_LDOUBLE_EXP_LOW ? _BASIC_MATH_LDOUBLE_EXP_LOW : n;
    return x * _Exact_MakeLDouble((unsigned int) (n + _EXACT_LDOUBLE_BIAS), _EXACT_LDOUBLE_INT_BIT);
}

// Clamp a long exponent to an int.
int _Basic_Math_ClampExponent(long n)
{
    return n > _BASIC_MATH_INT_MAX ? _BASIC_MATH_INT_MAX : n < _BASIC_MATH_INT_MIN ? _BASIC_MATH_INT_MIN : (int) n;
}

// Take the remainder of x by y by a quotient of kind.
long double _Basic_Math_TakeRemainder(long double x, long double y, _Basic_Math_Quotient kind, unsigned int *quo)
{
    int ex;
    int ey;
    unsigned int q = 0;
    unsigned long long carry = 0;
    long double sign = x;
    unsigned long long rest = _Exact_SplitLDouble(x, &ex);
    unsigned long long div = _Exact_SplitLDouble(y, &ey);

    for (int shift = ex - ey; shift >= 0; shift--) {
        if (carry != 0 || rest >= div) {
            rest -= div;
            q++;
        }
        if (shift > 0) {
            carry = rest & _EXACT_LDOUBLE_INT_BIT;
            rest <<= 1;
            q <<= 1;
        }
    }
    if (kind == _BASIC_MATH_QUOTIENT_NEAREST && ex >= ey && (rest > div - rest || (rest == div - rest && (q & _BASIC_MATH_QUOTIENT_ODD) != 0))) {
        rest = div - rest;
        sign = -x;
        q++;
    } else if (kind == _BASIC_MATH_QUOTIENT_NEAREST && ex == ey - 1 && rest > div) {
        rest = div - (rest - div);
        sign = -x;
        q = _BASIC_MATH_QUOTIENT_ODD;
    }
    *quo = q;
    return __basic_math_copysignl(_Exact_JoinLDouble(ex < ey ? ex : ey, rest), sign);
}

// Read nan's payload from tagp as strtod reads one from "NAN(tagp)".
unsigned long long _Basic_Math_ReadPayload(const char *tagp)
{
    unsigned long len = 0;

    while (tagp[len] == '_' || _Fp_IsAlnum((unsigned char) tagp[len])) {
        len++;
    }
    return tagp[len] == '\0' ? _Fp_ReadPayload(tagp, _FP_TEXT_NARROW, 0, len) : 0;
}

// Raise the domain error of x and y.
long double _Basic_Math_RaiseDomain(long double x, long double y)
{
    _Err_Errno = _SYS_EDOM;
    return (x * y) / (x * y);
}

// Raise the pole error, an exact -inf from a finite argument.
long double _Basic_Math_RaisePole(void)
{
    _Err_Errno = _SYS_ERANGE;
    _Fpu_RaiseFlags(_FPU_DIVBYZERO);
    return -_EXACT_INFINITY;
}

// Check y, x scaled, for a range error.
void _Basic_Math_CheckScaled(long double x, long double y)
{
    if (x == x && x != 0 && __basic_math_fabsl(x) != _EXACT_INFINITY && (y == 0 || __basic_math_fabsl(y) == _EXACT_INFINITY)) {
        _Err_Errno = _SYS_ERANGE;
    }
}

// Check y, a function of x, for a range error and a nonzero y below least.
long double _Basic_Math_CheckRange(long double x, long double y, long double least)
{
    _Basic_Math_CheckScaled(x, y);
    if (y == y && y != 0 && __basic_math_fabsl(y) < least) {
        _Fpu_RaiseFlags(_FPU_UNDERFLOW | _FPU_INEXACT);
    }
    return y;
}

// Check y, a function of x that is never tiny, for an overflow's range error.
long double _Basic_Math_CheckOverflow(long double x, long double y)
{
    if (x == x && __basic_math_fabsl(x) != _EXACT_INFINITY && __basic_math_fabsl(y) == _EXACT_INFINITY) {
        _Err_Errno = _SYS_ERANGE;
    }
    return y;
}

// Check z, x to y, for a range error and a nonzero z below least.
long double _Basic_Math_CheckPower(long double x, long double y, long double z, long double least)
{
    if (y == y && __basic_math_fabsl(y) != _EXACT_INFINITY) {
        _Basic_Math_CheckRange(x, z, least);
    }
    return z;
}

// Check a step from x to a value of class for a range error.
void _Basic_Math_CheckNext(long double x, int class)
{
    if (class == _BASIC_MATH_FP_INFINITE) {
        _Fpu_RaiseFlags(_FPU_OVERFLOW | _FPU_INEXACT);
        _Err_Errno = _SYS_ERANGE;
    } else if ((class == _BASIC_MATH_FP_SUBNORMAL || class == _BASIC_MATH_FP_ZERO) && x != 0) {
        _Fpu_RaiseFlags(_FPU_UNDERFLOW | _FPU_INEXACT);
        _Err_Errno = _SYS_ERANGE;
    } else if (class == _BASIC_MATH_FP_SUBNORMAL) {
        _Fpu_RaiseFlags(_FPU_UNDERFLOW | _FPU_INEXACT);
    }
}

// Take x's square or cube root, correctly rounded to type.
long double _Basic_Math_TakeRoot(long double x, _Exact_Root degree, _Exact_Type type)
{
    _Exact_Value root;
    _Exact_Value radicand;

    if (_BASIC_MATH_IS_NAN(x)) {
        return x + x;
    }
    if (x == 0 || (_BASIC_MATH_IS_INF(x) && (degree == _EXACT_ROOT_CUBE || x > 0))) {
        return x;
    }
    if (x < 0 && degree == _EXACT_ROOT_SQUARE) {
        return _Basic_Math_RaiseDomain(x, 0);
    }
    _Exact_MakeValue(&radicand, x);
    _Exact_ExtractRoot(&root, &radicand, degree);
    return _Exact_RoundValue(&root, type);
}

// Take the hypotenuse of x and y, correctly rounded to type.
long double _Basic_Math_TakeHypot(long double x, long double y, _Exact_Type type)
{
    _Exact_Value sum;
    _Exact_Value root;
    _Exact_Value side_x;
    _Exact_Value side_y;
    long double result;

    if (_BASIC_MATH_IS_INF(x) || _BASIC_MATH_IS_INF(y)) {
        return _EXACT_INFINITY;
    }
    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return x + y;
    }
    if (x == 0 || y == 0) {
        return __basic_math_fabsl(x) + __basic_math_fabsl(y);
    }
    _Exact_MakeValue(&side_x, x);
    _Exact_MakeValue(&side_y, y);
    _Exact_MultiplyValues(&side_x, &side_x, &side_x);
    _Exact_MultiplyValues(&side_y, &side_y, &side_y);
    _Exact_AddValues(&sum, &side_x, &side_y);
    _Exact_ExtractRoot(&root, &sum, _EXACT_ROOT_SQUARE);
    result = _Exact_RoundValue(&root, type);
    if (_BASIC_MATH_IS_INF(result)) {
        _Err_Errno = _SYS_ERANGE;
    }
    return result;
}

// Take x times y plus z, correctly rounded once to type.
long double _Basic_Math_TakeFma(long double x, long double y, long double z, _Exact_Type type)
{
    _Exact_Value sum;
    _Exact_Value addend;
    _Exact_Value product;

    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return x * y + z;
    }
    if (_BASIC_MATH_IS_NAN(z)) {
        return z + z;
    }
    if (_BASIC_MATH_IS_INF(x) || _BASIC_MATH_IS_INF(y)) {
        return x * y + z;
    }
    if (_BASIC_MATH_IS_INF(z)) {
        return z;
    }
    if (x == 0 || y == 0) {
        return x * y + z;
    }
    _Exact_MakeValue(&product, x);
    _Exact_MakeValue(&addend, y);
    _Exact_MultiplyValues(&product, &product, &addend);
    if (z == 0) {
        return _Exact_RoundValue(&product, type);
    }
    _Exact_MakeValue(&addend, z);
    _Exact_AddValues(&sum, &product, &addend);
    return _Exact_RoundValue(&sum, type);
}

// Reduce x to r, e or 2 to x being 2 to the returned n times e to r.
int _Basic_Math_ReduceExp(long double x, _Basic_Math_Base base, long double *r)
{
    long double t = base == _BASIC_MATH_BASE_2 ? x : x * _BASIC_MATH_LOG2E;
    int n = (int) t;

    if (t - n > _BASIC_MATH_HALF) {
        n++;
    } else if (n - t > _BASIC_MATH_HALF) {
        n--;
    }
    if (base == _BASIC_MATH_BASE_2) {
        *r = (x - n) * _BASIC_MATH_LN2;
    } else {
        *r = x - n * _BASIC_MATH_LN2_HIGH - n * _BASIC_MATH_LN2_LOW;
    }
    return n;
}

// Split a positive x into m in [sqrt(1/2), sqrt 2] and a power of 2.
int _Basic_Math_SplitFraction(long double x, long double *m)
{
    int e;

    *m = _Exact_SplitLDouble(x, &e) * _BASIC_MATH_LDOUBLE_UNIT;
    if (*m > _BASIC_MATH_SQRT_TWO) {
        *m *= _BASIC_MATH_HALF;
        e++;
    }
    return e;
}

// Sum e to a reduced r less 1 by Taylor's series.
long double _Basic_Math_SumExp(long double r)
{
    long double sum = 0;

    for (int k = _BASIC_MATH_EXP_TERMS - 1; k >= 0; k--) {
        sum = sum * r + _Basic_Math_ExpTerms[k];
    }
    return r * sum;
}

// Sum the natural logarithm of 1 plus a small f as 2 atanh(f/(2 + f)).
long double _Basic_Math_SumLog(long double f)
{
    long double s = f / (2 + f);
    long double square = s * s;
    long double sum = 0;

    for (int k = _BASIC_MATH_LOG_TERMS - 1; k >= 0; k--) {
        sum = sum * square + _Basic_Math_LogTerms[k];
    }
    return f - s * (f - 2 * square * sum);
}

// Take e or 2 to x.
long double _Basic_Math_TakeExp(long double x, _Basic_Math_Base base)
{
    long double r;
    int n;

    if (x != x || x == _EXACT_INFINITY) {
        return x + x;
    }
    if (x == -_EXACT_INFINITY) {
        return 0;
    }
    if (__basic_math_fabsl(x) > _BASIC_MATH_EXP_CLAMP) {
        x = __basic_math_copysignl(_BASIC_MATH_EXP_CLAMP, x);
    }
    n = _Basic_Math_ReduceExp(x, base, &r);
    return _Basic_Math_ScaleLDouble(1 + _Basic_Math_SumExp(r), n);
}

// Take e to x less 1.
long double _Basic_Math_TakeExpm1(long double x)
{
    long double r;
    long double sum;
    int n;

    if (x != x || x == _EXACT_INFINITY) {
        return x + x;
    }
    if (x == -_EXACT_INFINITY) {
        return -1;
    }
    if (x < -_BASIC_MATH_EXPM1_FLOOR) {
        x = -_BASIC_MATH_EXPM1_FLOOR;
    } else if (x > _BASIC_MATH_EXP_CLAMP) {
        x = _BASIC_MATH_EXP_CLAMP;
    }
    n = _Basic_Math_ReduceExp(x, _BASIC_MATH_BASE_E, &r);
    sum = _Basic_Math_SumExp(r);
    if (n == 0) {
        return sum;
    }
    if (n > _BASIC_MATH_EXPM1_WIDE) {
        return _Basic_Math_ScaleLDouble(1 + sum, n);
    }
    return _Basic_Math_ScaleLDouble(1 + sum, n) - 1;
}

// Take the logarithm of x to base.
long double _Basic_Math_TakeLog(long double x, _Basic_Math_Base base)
{
    int e;
    long double m;
    long double log;

    if (x != x || x == _EXACT_INFINITY) {
        return x + x;
    }
    if (x < 0) {
        return _Basic_Math_RaiseDomain(x, 0);
    }
    if (x == 0) {
        return _Basic_Math_RaisePole();
    }
    if (x == 1) {
        return 0;
    }
    e = _Basic_Math_SplitFraction(x, &m);
    log = _Basic_Math_SumLog(m - 1);
    if (base == _BASIC_MATH_BASE_2) {
        return e + log * _BASIC_MATH_LOG2E;
    }
    if (base == _BASIC_MATH_BASE_10) {
        return e * _BASIC_MATH_LOG10_2 + log * _BASIC_MATH_LOG10E;
    }
    return e * _BASIC_MATH_LN2_HIGH + (e * _BASIC_MATH_LN2_LOW + log);
}

// Take the natural logarithm of 1 plus x.
long double _Basic_Math_TakeLog1p(long double x)
{
    long double u;

    if (x != x || x == _EXACT_INFINITY) {
        return x + x;
    }
    if (x < -1) {
        return _Basic_Math_RaiseDomain(x, 0);
    }
    if (x == -1) {
        return _Basic_Math_RaisePole();
    }
    if (__basic_math_fabsl(x) < _BASIC_MATH_LOG1P_TINY) {
        return x;
    }
    u = 1 + x;
    return _Basic_Math_TakeLog(u, _BASIC_MATH_BASE_E) + (x - (u - 1)) / u;
}

// Split x into its top 32 bits and the rest, by Veltkamp's method.
long double _Basic_Math_SplitHalves(long double x, long double *low)
{
    long double c = x * _BASIC_MATH_SPLIT;
    long double high = c - (c - x);

    *low = x - high;
    return high;
}

// Add x and y into a rounded sum and its error, by Knuth's method.
long double _Basic_Math_AddParts(long double x, long double y, long double *error)
{
    long double sum = x + y;
    long double z = sum - x;

    *error = (x - (sum - z)) + (y - z);
    return sum;
}

// Multiply x by y into a rounded product and its error, by Dekker's method.
long double _Basic_Math_MultiplyParts(long double x, long double y, long double *error)
{
    long double x_low;
    long double y_low;
    long double x_high = _Basic_Math_SplitHalves(x, &x_low);
    long double y_high = _Basic_Math_SplitHalves(y, &y_low);
    long double product = x * y;

    *error = ((x_high * y_high - product) + x_high * y_low + x_low * y_high) + x_low * y_low;
    return product;
}

// Take the natural logarithm of a positive x as a high and a low part.
long double _Basic_Math_TakeLogParts(long double x, long double *low)
{
    long double m;
    long double s_low;
    long double p_low;
    long double q_low;
    long double error;
    long double rest;
    long double sum = 0;
    int e = _Basic_Math_SplitFraction(x, &m);
    int j = (int) (m * _BASIC_MATH_LOG_STEPS + _BASIC_MATH_HALF);
    long double c = (long double) j / _BASIC_MATH_LOG_STEPS;
    long double d = m - c;
    long double s = d / (m + c);
    long double square = s * s;
    long double p = _Basic_Math_MultiplyParts(2 * c, s, &p_low);
    long double q = _Basic_Math_MultiplyParts(d, s, &q_low);
    long double high = _Basic_Math_AddParts(e * _BASIC_MATH_LN2_HIGH, _Basic_Math_LogStepHigh[j - _BASIC_MATH_LOG_STEP_FIRST], &error);

    s_low = (((d - p) - q) - (p_low + q_low)) / (m + c);
    for (int k = _BASIC_MATH_POW_TERMS - 1; k >= 0; k--) {
        sum = sum * square + _Basic_Math_LogTerms[k];
    }
    high = _Basic_Math_AddParts(high, 2 * s, &rest);
    rest += error + (e * _BASIC_MATH_LN2_LOW + _Basic_Math_LogStepLow[j - _BASIC_MATH_LOG_STEP_FIRST] + 2 * s_low + 2 * s * square * sum);
    return _Basic_Math_AddParts(high, rest, low);
}

// Read whether a finite nonzero y is an integer, and which.
_Basic_Math_Parity _Basic_Math_ReadParity(long double y)
{
    int exp;
    unsigned long long sig = _Exact_SplitLDouble(y, &exp);
    int point = _EXACT_LDOUBLE_INT_SHIFT - exp;

    if (point < 0) {
        return _BASIC_MATH_PARITY_EVEN;
    }
    if (point > _EXACT_LDOUBLE_INT_SHIFT || (sig & ~(_EXACT_ALL_BITS << point)) != 0) {
        return _BASIC_MATH_PARITY_NONE;
    }
    return (sig >> point & _EXACT_LOW_BIT) != 0 ? _BASIC_MATH_PARITY_ODD : _BASIC_MATH_PARITY_EVEN;
}

// Take x to y.
long double _Basic_Math_TakePow(long double x, long double y)
{
    long double low;
    long double high;
    long double t;
    long double t_low;
    long double r;
    long double result;
    _Basic_Math_Parity parity;
    _Bool negative;
    int n;

    if (y == 0 || x == 1) {
        return 1;
    }
    if (x != x || y != y) {
        return x + y;
    }
    if (__basic_math_fabsl(y) == _EXACT_INFINITY) {
        return __basic_math_fabsl(x) == 1 ? 1 : (__basic_math_fabsl(x) < 1) == (y < 0) ? _EXACT_INFINITY : 0;
    }
    parity = _Basic_Math_ReadParity(y);
    negative = (x == 0 ? _BASIC_MATH_SIGNBIT(x) != 0 : x < 0) && parity == _BASIC_MATH_PARITY_ODD;
    if (x == 0 && y < 0) {
        return negative ? _Basic_Math_RaisePole() : -_Basic_Math_RaisePole();
    }
    if (x == 0 || __basic_math_fabsl(x) == _EXACT_INFINITY) {
        result = (x == 0) == (y < 0) ? _EXACT_INFINITY : 0;
        return negative ? -result : result;
    }
    if (x < 0 && parity == _BASIC_MATH_PARITY_NONE) {
        return _Basic_Math_RaiseDomain(x, 0);
    }
    if (__basic_math_fabsl(y) > _BASIC_MATH_POW_CLAMP) {
        y = __basic_math_copysignl(_BASIC_MATH_POW_CLAMP, y);
    }
    high = _Basic_Math_TakeLogParts(__basic_math_fabsl(x), &low);
    t = _Basic_Math_MultiplyParts(y, high, &t_low);
    t_low += y * low;
    if (__basic_math_fabsl(t) > _BASIC_MATH_EXP_CLAMP) {
        t = __basic_math_copysignl(_BASIC_MATH_EXP_CLAMP, t);
        t_low = 0;
    }
    n = _Basic_Math_ReduceExp(t, _BASIC_MATH_BASE_E, &r);
    result = 1 + _Basic_Math_SumExp(r + t_low);
    return _Basic_Math_ScaleLDouble(negative ? -result : result, n);
}

// Iterate to the square root of a nonnegative finite x by Newton's steps.
long double _Basic_Math_IterateRoot(long double x)
{
    int e;
    long double m;
    long double y;
    long double s;
    long double p;
    long double p_low;

    if (x == 0) {
        return 0;
    }
    m = _Exact_SplitLDouble(x, &e) * _BASIC_MATH_LDOUBLE_UNIT;
    if (e % 2 != 0) {
        m *= 2;
        e--;
    }
    y = _BASIC_MATH_ROOT_GUESS_0 + m * (_BASIC_MATH_ROOT_GUESS_1 + m * _BASIC_MATH_ROOT_GUESS_2);
    for (int k = 0; k < _BASIC_MATH_ROOT_STEPS; k++) {
        y += _BASIC_MATH_HALF * y * (1 - m * y * y);
    }
    s = m * y;
    p = _Basic_Math_MultiplyParts(s, s, &p_low);
    s += _BASIC_MATH_HALF * y * ((m - p) - p_low);
    return _Basic_Math_ScaleLDouble(s, e / 2);
}

// Reduce a positive x to r and its low part, returning its quadrant.
unsigned int _Basic_Math_ReduceTrig(long double x, long double *r, long double *low)
{
    _Exact_Value f;
    _Exact_Value part;
    long double high;
    long double error;
    long double rest;
    long double p;
    long double p_low;
    unsigned int quadrant;
    int n;

    if (x <= _BASIC_MATH_QUARTER_PI) {
        *r = x;
        *low = 0;
        return 0;
    }
    if (x < _BASIC_MATH_TRIG_NEAR) {
        n = (int) (x * _BASIC_MATH_TWO_OVER_PI + _BASIC_MATH_HALF);
        high = _Basic_Math_AddParts(x - n * _BASIC_MATH_HALF_PI_1, -n * _BASIC_MATH_HALF_PI_2, &error);
        p = _Basic_Math_MultiplyParts(n, _BASIC_MATH_HALF_PI_3, &p_low);
        high = _Basic_Math_AddParts(high, -p, &rest);
        rest += error - p_low - n * _BASIC_MATH_HALF_PI_4;
        *r = _Basic_Math_AddParts(high, rest, low);
        return (unsigned int) n & _BASIC_MATH_QUADRANT_MASK;
    }
    quadrant = _Exact_ReduceQuarters(&f, x);
    high = _Exact_RoundValue(&f, _EXACT_TYPE_LDOUBLE);
    _Exact_MakeValue(&part, -high);
    _Exact_AddValues(&f, &f, &part);
    rest = _Exact_RoundValue(&f, _EXACT_TYPE_LDOUBLE);
    p = _Basic_Math_MultiplyParts(high, _BASIC_MATH_HALF_PI_HIGH, &p_low);
    p_low += high * _BASIC_MATH_HALF_PI_LOW + rest * _BASIC_MATH_HALF_PI_HIGH;
    *r = _Basic_Math_AddParts(p, p_low, low);
    return quadrant;
}

// Sum the sine of a reduced r and its low part by Taylor's series.
long double _Basic_Math_SumSin(long double r, long double low)
{
    long double square = r * r;
    long double sum = 0;

    for (int k = _BASIC_MATH_SIN_TERMS - 1; k >= 0; k--) {
        sum = sum * square + _Basic_Math_SinTerms[k];
    }
    return r + (low + r * square * sum);
}

// Sum the cosine of a reduced r and its low part by Taylor's series.
long double _Basic_Math_SumCos(long double r, long double low)
{
    long double square = r * r;
    long double sum = 0;

    for (int k = _BASIC_MATH_COS_TERMS - 1; k >= 0; k--) {
        sum = sum * square + _Basic_Math_CosTerms[k];
    }
    return 1 + (square * sum - r * low);
}

// Sum the arc tangent of a t in [0, 1] by its nearest step and a series.
long double _Basic_Math_SumAtan(long double t)
{
    int j = (int) (t * _BASIC_MATH_ATAN_STEPS + _BASIC_MATH_HALF);
    long double c = (long double) j / _BASIC_MATH_ATAN_STEPS;
    long double s = (t - c) / (1 + t * c);
    long double square = s * s;
    long double sum = 0;

    for (int k = _BASIC_MATH_ATAN_TERMS - 1; k >= 0; k--) {
        sum = _Basic_Math_LogTerms[k] - sum * square;
    }
    return _Basic_Math_AtanStepHigh[j] + (_Basic_Math_AtanStepLow[j] + (s - s * square * sum));
}

// Take the sine, the cosine or the tangent of x.
long double _Basic_Math_TakeTrig(long double x, _Basic_Math_Trig which)
{
    long double r;
    long double low;
    long double sine;
    long double cosine;
    long double result;
    unsigned int quadrant;

    if (x != x) {
        return x + x;
    }
    if (__basic_math_fabsl(x) == _EXACT_INFINITY) {
        return _Basic_Math_RaiseDomain(x, 0);
    }
    if (x == 0) {
        return which == _BASIC_MATH_TRIG_COS ? 1 : x;
    }
    quadrant = _Basic_Math_ReduceTrig(__basic_math_fabsl(x), &r, &low);
    if (which == _BASIC_MATH_TRIG_TAN) {
        sine = _Basic_Math_SumSin(r, low);
        cosine = _Basic_Math_SumCos(r, low);
        result = (quadrant & _BASIC_MATH_QUADRANT_ODD) != 0 ? -cosine / sine : sine / cosine;
    } else {
        if (which == _BASIC_MATH_TRIG_COS) {
            quadrant++;
        }
        result = (quadrant & _BASIC_MATH_QUADRANT_ODD) != 0 ? _Basic_Math_SumCos(r, low) : _Basic_Math_SumSin(r, low);
        if ((quadrant & _BASIC_MATH_QUADRANT_HALF) != 0) {
            result = -result;
        }
    }
    return which != _BASIC_MATH_TRIG_COS && x < 0 ? -result : result;
}

// Take the arc tangent of y over x by both signs.
long double _Basic_Math_TakeAtan2(long double y, long double x)
{
    long double a = __basic_math_fabsl(y);
    long double b = __basic_math_fabsl(x);
    long double angle;

    if (x != x || y != y) {
        return x + y;
    }
    if (y == 0) {
        return _BASIC_MATH_SIGNBIT(x) == 0 ? y : _BASIC_MATH_SIGNBIT(y) != 0 ? -(_BASIC_MATH_PI_HIGH + _BASIC_MATH_PI_LOW) : _BASIC_MATH_PI_HIGH + _BASIC_MATH_PI_LOW;
    }
    if (b == _EXACT_INFINITY && a != _EXACT_INFINITY) {
        return x > 0 ? y * 0 : y < 0 ? -(_BASIC_MATH_PI_HIGH + _BASIC_MATH_PI_LOW) : _BASIC_MATH_PI_HIGH + _BASIC_MATH_PI_LOW;
    }
    if (a == _EXACT_INFINITY && b == _EXACT_INFINITY) {
        a = 1;
        b = 1;
    }
    if (a < b * _BASIC_MATH_ATAN_TINY && x > 0) {
        return y / b;
    }
    if (a <= b) {
        angle = _Basic_Math_SumAtan(a / b);
    } else {
        angle = _BASIC_MATH_HALF_PI_HIGH + (_BASIC_MATH_HALF_PI_LOW - _Basic_Math_SumAtan(b / a));
    }
    if (x < 0) {
        angle = _BASIC_MATH_PI_HIGH + (_BASIC_MATH_PI_LOW - angle);
    }
    return y < 0 ? -angle : angle;
}

// Take the arc sine or the arc cosine of x.
long double _Basic_Math_TakeArc(long double x, _Basic_Math_Trig which)
{
    long double a = __basic_math_fabsl(x);
    long double side;

    if (x != x) {
        return x + x;
    }
    if (a > 1) {
        return _Basic_Math_RaiseDomain(x, 0);
    }
    if (which == _BASIC_MATH_TRIG_SIN && x == 0) {
        return x;
    }
    if (which == _BASIC_MATH_TRIG_COS && x == 1) {
        return 0;
    }
    side = _Basic_Math_IterateRoot((1 - a) * (1 + a));
    return which == _BASIC_MATH_TRIG_SIN ? _Basic_Math_TakeAtan2(x, side) : _Basic_Math_TakeAtan2(side, x);
}

// Take the hyperbolic sine, cosine or tangent of x.
long double _Basic_Math_TakeHyperbolic(long double x, _Basic_Math_Trig which)
{
    long double a = __basic_math_fabsl(x);
    long double em1;
    long double r;
    long double result;
    int n;

    if (x != x) {
        return x + x;
    }
    if (x == 0) {
        return which == _BASIC_MATH_TRIG_COS ? 1 : x;
    }
    if (a == _EXACT_INFINITY) {
        return which == _BASIC_MATH_TRIG_COS ? a : which == _BASIC_MATH_TRIG_SIN ? x : x > 0 ? 1 : -1;
    }
    if (which == _BASIC_MATH_TRIG_TAN) {
        if (a > _BASIC_MATH_HYPERBOLIC_WIDE) {
            a = _BASIC_MATH_HYPERBOLIC_WIDE;
        }
        em1 = _Basic_Math_TakeExpm1(2 * a);
        result = em1 / (em1 + 2);
    } else if (a > _BASIC_MATH_HYPERBOLIC_WIDE) {
        if (a > _BASIC_MATH_EXP_CLAMP) {
            a = _BASIC_MATH_EXP_CLAMP;
        }
        n = _Basic_Math_ReduceExp(a, _BASIC_MATH_BASE_E, &r);
        result = 1 + _Basic_Math_SumExp(r);
        return _Basic_Math_ScaleLDouble(which == _BASIC_MATH_TRIG_SIN && x < 0 ? -result : result, n - 1);
    } else {
        em1 = _Basic_Math_TakeExpm1(a);
        if (which == _BASIC_MATH_TRIG_SIN) {
            result = _BASIC_MATH_HALF * (em1 + em1 / (em1 + 1));
        } else {
            result = 1 + _BASIC_MATH_HALF * (em1 * em1 / (em1 + 1));
        }
    }
    return which != _BASIC_MATH_TRIG_COS && x < 0 ? -result : result;
}

// Take the inverse hyperbolic sine, cosine or tangent of x.
long double _Basic_Math_TakeArcHyperbolic(long double x, _Basic_Math_Trig which)
{
    long double a = __basic_math_fabsl(x);
    long double result;

    if (x != x) {
        return x + x;
    }
    if ((which == _BASIC_MATH_TRIG_COS && x < 1) || (which == _BASIC_MATH_TRIG_TAN && a > 1)) {
        return _Basic_Math_RaiseDomain(x, 0);
    }
    if (which == _BASIC_MATH_TRIG_TAN && a == 1) {
        return x > 0 ? -_Basic_Math_RaisePole() : _Basic_Math_RaisePole();
    }
    if (a == _EXACT_INFINITY || x == 0) {
        return x;
    }
    if (which != _BASIC_MATH_TRIG_TAN && a > _BASIC_MATH_ARC_HYPERBOLIC_WIDE) {
        result = _Basic_Math_TakeLog(a, _BASIC_MATH_BASE_E) + _BASIC_MATH_LN2;
    } else if (which == _BASIC_MATH_TRIG_SIN) {
        result = _Basic_Math_TakeLog1p(a + a * a / (1 + _Basic_Math_IterateRoot(1 + a * a)));
    } else if (which == _BASIC_MATH_TRIG_COS) {
        result = _Basic_Math_TakeLog1p((a - 1) + _Basic_Math_IterateRoot((a - 1) * (a + 1)));
    } else {
        result = _BASIC_MATH_HALF * _Basic_Math_TakeLog1p(2 * a / (1 - a));
    }
    return which != _BASIC_MATH_TRIG_COS && x < 0 ? -result : result;
}

// Sum e to a high and a low part as a fraction scaled by 2 to the nth.
long double _Basic_Math_SumExpParts(long double high, long double low, int *n)
{
    long double r;

    if (__basic_math_fabsl(high) > _BASIC_MATH_EXP_CLAMP) {
        high = __basic_math_copysignl(_BASIC_MATH_EXP_CLAMP, high);
        low = 0;
    }
    *n = _Basic_Math_ReduceExp(high, _BASIC_MATH_BASE_E, &r);
    return 1 + _Basic_Math_SumExp(r + low);
}

// Sum erf of an x below 1/2 by its Maclaurin series, a high part and a low.
long double _Basic_Math_SumErf(long double x, long double *low)
{
    long double square = x * x;
    long double sum = 0;
    long double high;

    for (int k = _BASIC_MATH_ERF_TERMS - 1; k >= 0; k--) {
        sum = sum * square + _Basic_Math_ErfTerms[k];
    }
    high = _Basic_Math_MultiplyParts(x, _BASIC_MATH_TWO_OVER_ROOT_PI_HIGH, low);
    *low += x * (_BASIC_MATH_TWO_OVER_ROOT_PI_LOW + square * sum);
    return high;
}

// Sum erfc of an x of 1/2 or more as a fraction scaled by 2 to the nth.
long double _Basic_Math_SumErfc(long double x, int *n)
{
    long double square_low;
    long double square = _Basic_Math_MultiplyParts(x, x, &square_low);
    long double exp = _Basic_Math_SumExpParts(-square, -square_low, n);
    long double fraction = 0;
    long double sum = 0;
    long double node;
    long double h;
    long double power;
    long double next;
    long double value;
    long double slope;
    int j;

    if (x >= _BASIC_MATH_ERFC_FRACTION) {
        for (int k = _BASIC_MATH_ERFC_TERMS; k > 0; k--) {
            fraction = k * _BASIC_MATH_HALF / (x + fraction);
        }
        return exp * (_BASIC_MATH_INV_ROOT_PI / (x + fraction));
    }
    j = (int) (x * _BASIC_MATH_ERFC_NODES + _BASIC_MATH_HALF);
    node = (long double) j / _BASIC_MATH_ERFC_NODES;
    j -= _BASIC_MATH_ERFC_NODE_FIRST;
    h = x - node;
    value = _Basic_Math_ErfcValueHigh[j];
    slope = _Basic_Math_ErfcSlopeHigh[j];
    power = h;
    for (int k = 1; k < _BASIC_MATH_ERFC_TAYLOR_TERMS; k++) {
        sum += slope * power;
        next = (2 * node * slope + 2 * value) / (k + 1);
        value = slope;
        slope = next;
        power *= h;
    }
    value = _Basic_Math_ErfcValueHigh[j] + (_Basic_Math_ErfcValueLow[j] + h * _Basic_Math_ErfcSlopeLow[j] + sum);
    return exp * value;
}

// Take the error function of x.
long double _Basic_Math_TakeErf(long double x)
{
    long double a = __basic_math_fabsl(x);
    long double low;
    long double result;
    int n;

    if (x != x) {
        return x + x;
    }
    if (x == 0) {
        return x;
    }
    if (a == _EXACT_INFINITY) {
        return x > 0 ? 1 : -1;
    }
    if (a < _BASIC_MATH_HALF) {
        result = _Basic_Math_SumErf(x, &low);
        return result + low;
    }
    if (a > _BASIC_MATH_ERF_ONE) {
        a = _BASIC_MATH_ERF_ONE;
    }
    result = _Basic_Math_SumErfc(a, &n);
    result = 1 - _Basic_Math_ScaleLDouble(result, n);
    return x < 0 ? -result : result;
}

// Take the complementary error function of x.
long double _Basic_Math_TakeErfc(long double x)
{
    long double a = __basic_math_fabsl(x);
    long double low;
    long double error;
    long double result;
    int n;

    if (x != x) {
        return x + x;
    }
    if (x == 0) {
        return 1;
    }
    if (a == _EXACT_INFINITY) {
        return x > 0 ? 0 : 2;
    }
    if (a < _BASIC_MATH_HALF) {
        result = _Basic_Math_SumErf(x, &low);
        result = _Basic_Math_AddParts(1, -result, &error);
        return result + (error - low);
    }
    if (x < 0 && a > _BASIC_MATH_ERF_ONE) {
        a = _BASIC_MATH_ERF_ONE;
    }
    if (a > _BASIC_MATH_ERFC_CLAMP) {
        a = _BASIC_MATH_ERFC_CLAMP;
    }
    result = _Basic_Math_SumErfc(a, &n);
    result = _Basic_Math_ScaleLDouble(result, n);
    return x < 0 ? 2 - result : result;
}

// Take the sine of pi times an x below 2^63 in magnitude.
long double _Basic_Math_SinPi(long double x)
{
    long long n = (long long) x;
    long double r = x - (long double) n;
    long double a;
    long double turn;
    long double p;
    long double p_low;
    long double sine;

    if (r > _BASIC_MATH_HALF) {
        r -= 1;
        n++;
    } else if (r < -_BASIC_MATH_HALF) {
        r += 1;
        n--;
    }
    a = __basic_math_fabsl(r);
    turn = a > _BASIC_MATH_QUARTER ? _BASIC_MATH_HALF - a : a;
    p = _Basic_Math_MultiplyParts(turn, _BASIC_MATH_PI_HIGH, &p_low);
    p_low += turn * _BASIC_MATH_PI_LOW;
    sine = a > _BASIC_MATH_QUARTER ? _Basic_Math_SumCos(p, p_low) : _Basic_Math_SumSin(p, p_low);
    return (r < 0) != ((n & 1) != 0) ? -sine : sine;
}

// Shift an x from -2 to 10 by the returned m, t being x + m less 2.
int _Basic_Math_ShiftGamma(long double x, long double *t)
{
    int n = (int) (x + (_BASIC_MATH_GAMMA_CENTER + _BASIC_MATH_HALF));

    *t = x + (_BASIC_MATH_GAMMA_CENTER - n);
    return 2 * _BASIC_MATH_GAMMA_CENTER - n;
}

// Sum the logarithm of gamma of 2 plus a t within 1/2 by Taylor's series.
long double _Basic_Math_SumLgamma(long double t)
{
    long double sum = 0;

    for (int k = _BASIC_MATH_GAMMA_TERMS - 1; k >= 0; k--) {
        sum = sum * t + _Basic_Math_GammaTerms[k];
    }
    return t * sum;
}

// Sum lgamma of an x of 10 or more by Stirling's series, a high and a low.
long double _Basic_Math_SumStirling(long double x, long double *low)
{
    long double ln_low;
    long double p_low;
    long double error;
    long double rest;
    long double ln = _Basic_Math_TakeLogParts(x, &ln_low);
    long double w = x - _BASIC_MATH_HALF;
    long double p = _Basic_Math_MultiplyParts(w, ln, &p_low);
    long double inverse = 1 / x;
    long double square = inverse * inverse;
    long double sum = 0;
    long double high;

    for (int k = _BASIC_MATH_STIRLING_TERMS - 1; k >= 0; k--) {
        sum = sum * square + _Basic_Math_StirlingTerms[k];
    }
    high = _Basic_Math_AddParts(p, -x, &error);
    rest = error + (p_low + w * ln_low) + (_BASIC_MATH_LN_ROOT_2PI_LOW + inverse * sum);
    high = _Basic_Math_AddParts(high, _BASIC_MATH_LN_ROOT_2PI_HIGH, &error);
    return _Basic_Math_AddParts(high, rest + error, low);
}

// Sum lgamma of an x from -24 to -2 from its zero nearest in its half.
long double _Basic_Math_SumLgammaZero(long double x)
{
    int n = (int) -x;
    int lower = x <= -n - _BASIC_MATH_HALF;
    int i = 2 * (n - _BASIC_MATH_GAMMA_ZERO_FIRST) + lower;
    int m = n + _BASIC_MATH_GAMMA_CENTER + lower;
    long double zero = _Basic_Math_GammaZeroHigh[i];
    long double zero_low = _Basic_Math_GammaZeroLow[i];
    long double h = (x - zero) - zero_low;
    long double t = x + (m - _BASIC_MATH_GAMMA_CENTER);
    long double t0 = (zero + (m - _BASIC_MATH_GAMMA_CENTER)) + zero_low;
    long double ratio = 1;
    long double excess = 0;
    long double power = 1;
    long double q = 0;
    long double sum = 0;
    long double u;
    _Bool near = 1;

    for (int k = 0; k < m; k++) {
        ratio *= ((zero + k) + zero_low) / (x + k);
        u = -h / (x + k);
        excess += u + excess * u;
        near = near && __basic_math_fabsl(u) <= _BASIC_MATH_HALF;
    }
    for (int k = 0; k < _BASIC_MATH_GAMMA_TERMS; k++) {
        q = t * q + power;
        power *= t0;
        sum += _Basic_Math_GammaTerms[k] * q;
    }
    if (near && __basic_math_fabsl(excess) <= _BASIC_MATH_HALF) {
        return _Basic_Math_TakeLog1p(excess) + h * sum;
    }
    return _Basic_Math_TakeLog(ratio, _BASIC_MATH_BASE_E) + h * sum;
}

// Sum gamma of an x from -2 to 10 by lgamma's series at 2.
long double _Basic_Math_SumTgamma(long double x)
{
    long double t;
    int m = _Basic_Math_ShiftGamma(x, &t);
    long double gamma = _Basic_Math_TakeExp(_Basic_Math_SumLgamma(t), _BASIC_MATH_BASE_E);

    for (int k = 1; k <= -m; k++) {
        gamma *= x - k;
    }
    for (int k = m - 1; k >= 0; k--) {
        gamma /= x + k;
    }
    return gamma;
}

// Take ln |gamma| of x as a fraction scaled by 2 to the nth.
long double _Basic_Math_TakeLgamma(long double x, int *n)
{
    long double t;
    long double low;
    long double result;
    long double product = 1;
    int m;

    *n = 0;
    if (x != x) {
        return x + x;
    }
    if (__basic_math_fabsl(x) == _EXACT_INFINITY) {
        return _EXACT_INFINITY;
    }
    if (x == 1 || x == 2) {
        return 0;
    }
    if (x == 0 || (x < 0 && _Basic_Math_ReadParity(x) != _BASIC_MATH_PARITY_NONE)) {
        return -_Basic_Math_RaisePole();
    }
    if (x >= _BASIC_MATH_STIRLING_WIDE) {
        *n = _BASIC_MATH_GAMMA_SHIFT;
        return _Basic_Math_ScaleLDouble(x, -_BASIC_MATH_GAMMA_SHIFT) * (_Basic_Math_TakeLog(x, _BASIC_MATH_BASE_E) - 1);
    }
    if (x >= _BASIC_MATH_STIRLING_FROM) {
        result = _Basic_Math_SumStirling(x, &low);
        return result + low;
    }
    if (x > -_BASIC_MATH_GAMMA_CENTER) {
        m = _Basic_Math_ShiftGamma(x, &t);
        result = _Basic_Math_SumLgamma(t);
        if (m > 0) {
            for (int k = 0; k < m - 1; k++) {
                result -= _Basic_Math_TakeLog(__basic_math_fabsl(x + k), _BASIC_MATH_BASE_E);
            }
            return result - _Basic_Math_TakeLog1p(t);
        }
        for (int k = 1; k <= -m; k++) {
            product *= x - k;
        }
        return result + _Basic_Math_TakeLog(product, _BASIC_MATH_BASE_E);
    }
    if (x > _BASIC_MATH_GAMMA_REFLECT) {
        return _Basic_Math_SumLgammaZero(x);
    }
    result = _Basic_Math_SumStirling(-x, &low);
    t = _BASIC_MATH_LN_PI - _Basic_Math_TakeLog(__basic_math_fabsl(_Basic_Math_SinPi(x)), _BASIC_MATH_BASE_E);
    return (t - _Basic_Math_TakeLog(-x, _BASIC_MATH_BASE_E)) - (result + low);
}

// Take gamma of x as a fraction scaled by 2 to the nth.
long double _Basic_Math_TakeTgamma(long double x, int *n)
{
    long double low;
    long double factor;
    long double result;

    *n = 0;
    if (x != x) {
        return x + x;
    }
    if (x == 0) {
        return _BASIC_MATH_SIGNBIT(x) != 0 ? _Basic_Math_RaisePole() : -_Basic_Math_RaisePole();
    }
    if (x == _EXACT_INFINITY) {
        return x;
    }
    if (x == -_EXACT_INFINITY || (x < 0 && _Basic_Math_ReadParity(x) != _BASIC_MATH_PARITY_NONE)) {
        return _Basic_Math_RaiseDomain(x, 0);
    }
    if (x > 0 && x <= _BASIC_MATH_FACTORIAL_MAX && _Basic_Math_ReadParity(x) != _BASIC_MATH_PARITY_NONE) {
        result = 1;
        for (int k = 2; k < (int) x; k++) {
            result *= k;
        }
        return result;
    }
    if (x > _BASIC_MATH_GAMMA_CLAMP) {
        x = _BASIC_MATH_GAMMA_CLAMP;
    }
    if (x >= _BASIC_MATH_STIRLING_FROM) {
        result = _Basic_Math_SumStirling(x, &low);
        return _Basic_Math_SumExpParts(result, low, n);
    }
    if (__basic_math_fabsl(x) < _BASIC_MATH_GAMMA_TINY) {
        *n = _BASIC_MATH_GAMMA_SHIFT;
        return 1 / _Basic_Math_ScaleLDouble(x, _BASIC_MATH_GAMMA_SHIFT);
    }
    if (x > -_BASIC_MATH_GAMMA_CENTER) {
        return _Basic_Math_SumTgamma(x);
    }
    factor = -_BASIC_MATH_PI_HIGH / (x * _Basic_Math_SinPi(x));
    if (-x < _BASIC_MATH_STIRLING_FROM) {
        return factor / _Basic_Math_SumTgamma(-x);
    }
    result = _Basic_Math_SumStirling(-x, &low);
    return factor * _Basic_Math_SumExpParts(-result, -low, n);
}

// Take a fraction of x to nearest, then scale it in the rounding direction.
long double _Basic_Math_TakeNearest(long double (*take)(long double, int *), long double x)
{
    int round = _Fpu_ReadRound();
    long double result;
    int n;

    if (round != _FPU_TONEAREST) {
        _Fpu_WriteRound(_FPU_TONEAREST);
    }
    result = take(x, &n);
    if (round != _FPU_TONEAREST) {
        _Fpu_WriteRound(round);
    }
    return _Basic_Math_ScaleLDouble(result, n);
}

// Take the arc cosine of x.
double __basic_math_acos(double x)
{
    return (double) _Basic_Math_TakeArc(x, _BASIC_MATH_TRIG_COS);
}

// Take the arc cosine of x.
float __basic_math_acosf(float x)
{
    return (float) _Basic_Math_TakeArc(x, _BASIC_MATH_TRIG_COS);
}

// Take the arc cosine of x.
long double __basic_math_acosl(long double x)
{
    return _Basic_Math_TakeArc(x, _BASIC_MATH_TRIG_COS);
}

// Take the arc sine of x.
double __basic_math_asin(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeArc(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_DBL_MIN);
}

// Take the arc sine of x.
float __basic_math_asinf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeArc(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_FLT_MIN);
}

// Take the arc sine of x.
long double __basic_math_asinl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeArc(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_LDBL_MIN);
}

// Take the arc tangent of x.
double __basic_math_atan(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeAtan2(x, 1), _BASIC_MATH_DBL_MIN);
}

// Take the arc tangent of x.
float __basic_math_atanf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeAtan2(x, 1), _BASIC_MATH_FLT_MIN);
}

// Take the arc tangent of x.
long double __basic_math_atanl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeAtan2(x, 1), _BASIC_MATH_LDBL_MIN);
}

// Take the arc tangent of y over x by both signs.
double __basic_math_atan2(double y, double x)
{
    return (double) _Basic_Math_CheckRange(__basic_math_fabsl(x) == _EXACT_INFINITY ? 0 : y, (double) _Basic_Math_TakeAtan2(y, x), _BASIC_MATH_DBL_MIN);
}

// Take the arc tangent of y over x by both signs.
float __basic_math_atan2f(float y, float x)
{
    return (float) _Basic_Math_CheckRange(__basic_math_fabsl(x) == _EXACT_INFINITY ? 0 : y, (float) _Basic_Math_TakeAtan2(y, x), _BASIC_MATH_FLT_MIN);
}

// Take the arc tangent of y over x by both signs.
long double __basic_math_atan2l(long double y, long double x)
{
    return _Basic_Math_CheckRange(__basic_math_fabsl(x) == _EXACT_INFINITY ? 0 : y, _Basic_Math_TakeAtan2(y, x), _BASIC_MATH_LDBL_MIN);
}

// Take the cosine of x.
double __basic_math_cos(double x)
{
    return (double) _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_COS);
}

// Take the cosine of x.
float __basic_math_cosf(float x)
{
    return (float) _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_COS);
}

// Take the cosine of x.
long double __basic_math_cosl(long double x)
{
    return _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_COS);
}

// Take the sine of x.
double __basic_math_sin(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_DBL_MIN);
}

// Take the sine of x.
float __basic_math_sinf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_FLT_MIN);
}

// Take the sine of x.
long double __basic_math_sinl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_LDBL_MIN);
}

// Take the tangent of x.
double __basic_math_tan(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_DBL_MIN);
}

// Take the tangent of x.
float __basic_math_tanf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_FLT_MIN);
}

// Take the tangent of x.
long double __basic_math_tanl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeTrig(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_LDBL_MIN);
}

// Take the inverse hyperbolic cosine of x.
double __basic_math_acosh(double x)
{
    return (double) _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_COS);
}

// Take the inverse hyperbolic cosine of x.
float __basic_math_acoshf(float x)
{
    return (float) _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_COS);
}

// Take the inverse hyperbolic cosine of x.
long double __basic_math_acoshl(long double x)
{
    return _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_COS);
}

// Take the inverse hyperbolic sine of x.
double __basic_math_asinh(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_DBL_MIN);
}

// Take the inverse hyperbolic sine of x.
float __basic_math_asinhf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_FLT_MIN);
}

// Take the inverse hyperbolic sine of x.
long double __basic_math_asinhl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_LDBL_MIN);
}

// Take the inverse hyperbolic tangent of x.
double __basic_math_atanh(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_DBL_MIN);
}

// Take the inverse hyperbolic tangent of x.
float __basic_math_atanhf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_FLT_MIN);
}

// Take the inverse hyperbolic tangent of x.
long double __basic_math_atanhl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeArcHyperbolic(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_LDBL_MIN);
}

// Take the hyperbolic cosine of x.
double __basic_math_cosh(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_COS), _BASIC_MATH_DBL_MIN);
}

// Take the hyperbolic cosine of x.
float __basic_math_coshf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_COS), _BASIC_MATH_FLT_MIN);
}

// Take the hyperbolic cosine of x.
long double __basic_math_coshl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_COS), _BASIC_MATH_LDBL_MIN);
}

// Take the hyperbolic sine of x.
double __basic_math_sinh(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_DBL_MIN);
}

// Take the hyperbolic sine of x.
float __basic_math_sinhf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_FLT_MIN);
}

// Take the hyperbolic sine of x.
long double __basic_math_sinhl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_SIN), _BASIC_MATH_LDBL_MIN);
}

// Take the hyperbolic tangent of x.
double __basic_math_tanh(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_DBL_MIN);
}

// Take the hyperbolic tangent of x.
float __basic_math_tanhf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_FLT_MIN);
}

// Take the hyperbolic tangent of x.
long double __basic_math_tanhl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeHyperbolic(x, _BASIC_MATH_TRIG_TAN), _BASIC_MATH_LDBL_MIN);
}

// Take e to x.
double __basic_math_exp(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeExp(x, _BASIC_MATH_BASE_E), _BASIC_MATH_DBL_MIN);
}

// Take e to x.
float __basic_math_expf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeExp(x, _BASIC_MATH_BASE_E), _BASIC_MATH_FLT_MIN);
}

// Take e to x.
long double __basic_math_expl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeExp(x, _BASIC_MATH_BASE_E), _BASIC_MATH_LDBL_MIN);
}

// Take 2 to x.
double __basic_math_exp2(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeExp(x, _BASIC_MATH_BASE_2), _BASIC_MATH_DBL_MIN);
}

// Take 2 to x.
float __basic_math_exp2f(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeExp(x, _BASIC_MATH_BASE_2), _BASIC_MATH_FLT_MIN);
}

// Take 2 to x.
long double __basic_math_exp2l(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeExp(x, _BASIC_MATH_BASE_2), _BASIC_MATH_LDBL_MIN);
}

// Take e to x, less 1.
double __basic_math_expm1(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeExpm1(x), _BASIC_MATH_DBL_MIN);
}

// Take e to x, less 1.
float __basic_math_expm1f(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeExpm1(x), _BASIC_MATH_FLT_MIN);
}

// Take e to x, less 1.
long double __basic_math_expm1l(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeExpm1(x), _BASIC_MATH_LDBL_MIN);
}

// Break value into a fraction and a power of 2.
double __basic_math_frexp(double value, int *exp)
{
    return (double) __basic_math_frexpl(value, exp);
}

// Break value into a fraction and a power of 2.
float __basic_math_frexpf(float value, int *exp)
{
    return (float) __basic_math_frexpl(value, exp);
}

// Break value into a fraction and a power of 2.
long double __basic_math_frexpl(long double value, int *exp)
{
    int power;
    unsigned long long sig;

    *exp = 0;
    if (! _BASIC_MATH_IS_FINITE(value) || value == 0) {
        return value + value;
    }
    sig = _Exact_SplitLDouble(value, &power);
    *exp = power - _BASIC_MATH_FRACTION_EXP;
    return __basic_math_copysignl(_Exact_JoinLDouble(_BASIC_MATH_FRACTION_EXP, sig), value);
}

// Extract x's exponent as an int.
int __basic_math_ilogb(double x)
{
    return __basic_math_ilogbl(x);
}

// Extract x's exponent as an int.
int __basic_math_ilogbf(float x)
{
    return __basic_math_ilogbl(x);
}

// Extract x's exponent as an int.
int __basic_math_ilogbl(long double x)
{
    int exp;
    int class = _BASIC_MATH_CLASSIFY(x);

    if (class == _BASIC_MATH_FP_NAN || class == _BASIC_MATH_FP_INFINITE || class == _BASIC_MATH_FP_ZERO) {
        _Err_Errno = _SYS_EDOM;
        _Fpu_RaiseFlags(_FPU_INVALID);
        return class == _BASIC_MATH_FP_INFINITE ? _BASIC_MATH_INT_MAX : class == _BASIC_MATH_FP_ZERO ? _BASIC_MATH_FP_ILOGB0 : _BASIC_MATH_FP_ILOGBNAN;
    }
    _Exact_SplitLDouble(x, &exp);
    return exp;
}

// Multiply x by 2 to exp.
double __basic_math_ldexp(double x, int exp)
{
    return __basic_math_scalbn(x, exp);
}

// Multiply x by 2 to exp.
float __basic_math_ldexpf(float x, int exp)
{
    return __basic_math_scalbnf(x, exp);
}

// Multiply x by 2 to exp.
long double __basic_math_ldexpl(long double x, int exp)
{
    return __basic_math_scalbnl(x, exp);
}

// Take the natural logarithm of x.
double __basic_math_log(double x)
{
    return (double) _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_E);
}

// Take the natural logarithm of x.
float __basic_math_logf(float x)
{
    return (float) _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_E);
}

// Take the natural logarithm of x.
long double __basic_math_logl(long double x)
{
    return _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_E);
}

// Take the base-10 logarithm of x.
double __basic_math_log10(double x)
{
    return (double) _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_10);
}

// Take the base-10 logarithm of x.
float __basic_math_log10f(float x)
{
    return (float) _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_10);
}

// Take the base-10 logarithm of x.
long double __basic_math_log10l(long double x)
{
    return _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_10);
}

// Take the natural logarithm of 1 plus x.
double __basic_math_log1p(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeLog1p(x), _BASIC_MATH_DBL_MIN);
}

// Take the natural logarithm of 1 plus x.
float __basic_math_log1pf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeLog1p(x), _BASIC_MATH_FLT_MIN);
}

// Take the natural logarithm of 1 plus x.
long double __basic_math_log1pl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeLog1p(x), _BASIC_MATH_LDBL_MIN);
}

// Take the base-2 logarithm of x.
double __basic_math_log2(double x)
{
    return (double) _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_2);
}

// Take the base-2 logarithm of x.
float __basic_math_log2f(float x)
{
    return (float) _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_2);
}

// Take the base-2 logarithm of x.
long double __basic_math_log2l(long double x)
{
    return _Basic_Math_TakeLog(x, _BASIC_MATH_BASE_2);
}

// Extract x's exponent as a floating value.
double __basic_math_logb(double x)
{
    return (double) __basic_math_logbl(x);
}

// Extract x's exponent as a floating value.
float __basic_math_logbf(float x)
{
    return (float) __basic_math_logbl(x);
}

// Extract x's exponent as a floating value.
long double __basic_math_logbl(long double x)
{
    int exp;

    if (_BASIC_MATH_IS_NAN(x)) {
        return x + x;
    }
    if (_BASIC_MATH_IS_INF(x)) {
        return _EXACT_INFINITY;
    }
    if (x == 0) {
        _Fpu_RaiseFlags(_FPU_DIVBYZERO);
        return -_EXACT_INFINITY;
    }
    _Exact_SplitLDouble(x, &exp);
    return exp;
}

// Break value into its integral and fractional parts.
double __basic_math_modf(double value, double *iptr)
{
    long double whole;
    double fraction = (double) __basic_math_modfl(value, &whole);

    *iptr = (double) whole;
    return fraction;
}

// Break value into its integral and fractional parts.
float __basic_math_modff(float value, float *iptr)
{
    long double whole;
    float fraction = (float) __basic_math_modfl(value, &whole);

    *iptr = (float) whole;
    return fraction;
}

// Break value into its integral and fractional parts.
long double __basic_math_modfl(long double value, long double *iptr)
{
    *iptr = __basic_math_truncl(value);
    if (_BASIC_MATH_IS_INF(value)) {
        return __basic_math_copysignl(0, value);
    }
    return __basic_math_copysignl(value - *iptr, value);
}

// Scale x by FLT_RADIX to n.
double __basic_math_scalbn(double x, int n)
{
    double y = (double) _Basic_Math_ScaleLDouble(x, n);

    _Basic_Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to n.
float __basic_math_scalbnf(float x, int n)
{
    float y = (float) _Basic_Math_ScaleLDouble(x, n);

    _Basic_Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to n.
long double __basic_math_scalbnl(long double x, int n)
{
    long double y = _Basic_Math_ScaleLDouble(x, n);

    _Basic_Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to a long n.
double __basic_math_scalbln(double x, long int n)
{
    return __basic_math_scalbn(x, _Basic_Math_ClampExponent(n));
}

// Scale x by FLT_RADIX to a long n.
float __basic_math_scalblnf(float x, long int n)
{
    return __basic_math_scalbnf(x, _Basic_Math_ClampExponent(n));
}

// Scale x by FLT_RADIX to a long n.
long double __basic_math_scalblnl(long double x, long int n)
{
    return __basic_math_scalbnl(x, _Basic_Math_ClampExponent(n));
}

// Take the real cube root of x.
double __basic_math_cbrt(double x)
{
    return (double) _Basic_Math_TakeRoot(x, _EXACT_ROOT_CUBE, _EXACT_TYPE_DOUBLE);
}

// Take the real cube root of x.
float __basic_math_cbrtf(float x)
{
    return (float) _Basic_Math_TakeRoot(x, _EXACT_ROOT_CUBE, _EXACT_TYPE_FLOAT);
}

// Take the real cube root of x.
long double __basic_math_cbrtl(long double x)
{
    return _Basic_Math_TakeRoot(x, _EXACT_ROOT_CUBE, _EXACT_TYPE_LDOUBLE);
}

// Clear the sign of x.
double __basic_math_fabs(double x)
{
    ((unsigned char *) &x)[_BASIC_MATH_DOUBLE_SIGN_BYTE] &= _BASIC_MATH_SIGN_CLEAR;
    return x;
}

// Clear the sign of x.
float __basic_math_fabsf(float x)
{
    ((unsigned char *) &x)[_BASIC_MATH_FLOAT_SIGN_BYTE] &= _BASIC_MATH_SIGN_CLEAR;
    return x;
}

// Clear the sign of x.
long double __basic_math_fabsl(long double x)
{
    ((unsigned char *) &x)[_BASIC_MATH_LDOUBLE_SIGN_BYTE] &= _BASIC_MATH_SIGN_CLEAR;
    return x;
}

// Take the square root of the sum of the squares of x and y.
double __basic_math_hypot(double x, double y)
{
    return (double) _Basic_Math_TakeHypot(x, y, _EXACT_TYPE_DOUBLE);
}

// Take the square root of the sum of the squares of x and y.
float __basic_math_hypotf(float x, float y)
{
    return (float) _Basic_Math_TakeHypot(x, y, _EXACT_TYPE_FLOAT);
}

// Take the square root of the sum of the squares of x and y.
long double __basic_math_hypotl(long double x, long double y)
{
    return _Basic_Math_TakeHypot(x, y, _EXACT_TYPE_LDOUBLE);
}

// Raise x to the power y.
double __basic_math_pow(double x, double y)
{
    return (double) _Basic_Math_CheckPower(x, y, (double) _Basic_Math_TakePow(x, y), _BASIC_MATH_DBL_MIN);
}

// Raise x to the power y.
float __basic_math_powf(float x, float y)
{
    return (float) _Basic_Math_CheckPower(x, y, (float) _Basic_Math_TakePow(x, y), _BASIC_MATH_FLT_MIN);
}

// Raise x to the power y.
long double __basic_math_powl(long double x, long double y)
{
    return _Basic_Math_CheckPower(x, y, _Basic_Math_TakePow(x, y), _BASIC_MATH_LDBL_MIN);
}

// Take the nonnegative square root of x.
double __basic_math_sqrt(double x)
{
    return (double) _Basic_Math_TakeRoot(x, _EXACT_ROOT_SQUARE, _EXACT_TYPE_DOUBLE);
}

// Take the nonnegative square root of x.
float __basic_math_sqrtf(float x)
{
    return (float) _Basic_Math_TakeRoot(x, _EXACT_ROOT_SQUARE, _EXACT_TYPE_FLOAT);
}

// Take the nonnegative square root of x.
long double __basic_math_sqrtl(long double x)
{
    return _Basic_Math_TakeRoot(x, _EXACT_ROOT_SQUARE, _EXACT_TYPE_LDOUBLE);
}

// Take the error function of x.
double __basic_math_erf(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeErf(x), _BASIC_MATH_DBL_MIN);
}

// Take the error function of x.
float __basic_math_erff(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeErf(x), _BASIC_MATH_FLT_MIN);
}

// Take the error function of x.
long double __basic_math_erfl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeErf(x), _BASIC_MATH_LDBL_MIN);
}

// Take the complementary error function of x.
double __basic_math_erfc(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeErfc(x), _BASIC_MATH_DBL_MIN);
}

// Take the complementary error function of x.
float __basic_math_erfcf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeErfc(x), _BASIC_MATH_FLT_MIN);
}

// Take the complementary error function of x.
long double __basic_math_erfcl(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeErfc(x), _BASIC_MATH_LDBL_MIN);
}

// Take the natural logarithm of the magnitude of gamma of x.
double __basic_math_lgamma(double x)
{
    return (double) _Basic_Math_CheckOverflow(x, (double) _Basic_Math_TakeNearest(_Basic_Math_TakeLgamma, x));
}

// Take the natural logarithm of the magnitude of gamma of x.
float __basic_math_lgammaf(float x)
{
    return (float) _Basic_Math_CheckOverflow(x, (float) _Basic_Math_TakeNearest(_Basic_Math_TakeLgamma, x));
}

// Take the natural logarithm of the magnitude of gamma of x.
long double __basic_math_lgammal(long double x)
{
    return _Basic_Math_CheckOverflow(x, _Basic_Math_TakeNearest(_Basic_Math_TakeLgamma, x));
}

// Take gamma of x.
double __basic_math_tgamma(double x)
{
    return (double) _Basic_Math_CheckRange(x, (double) _Basic_Math_TakeNearest(_Basic_Math_TakeTgamma, x), _BASIC_MATH_DBL_MIN);
}

// Take gamma of x.
float __basic_math_tgammaf(float x)
{
    return (float) _Basic_Math_CheckRange(x, (float) _Basic_Math_TakeNearest(_Basic_Math_TakeTgamma, x), _BASIC_MATH_FLT_MIN);
}

// Take gamma of x.
long double __basic_math_tgammal(long double x)
{
    return _Basic_Math_CheckRange(x, _Basic_Math_TakeNearest(_Basic_Math_TakeTgamma, x), _BASIC_MATH_LDBL_MIN);
}

// Round x up to an integer.
double __basic_math_ceil(double x)
{
    return (double) __basic_math_ceill(x);
}

// Round x up to an integer.
float __basic_math_ceilf(float x)
{
    return (float) __basic_math_ceill(x);
}

// Round x up to an integer.
long double __basic_math_ceill(long double x)
{
    long double whole = __basic_math_truncl(x);

    return _BASIC_MATH_IS_FINITE(x) && whole < x ? whole + 1 : whole;
}

// Round x down to an integer.
double __basic_math_floor(double x)
{
    return (double) __basic_math_floorl(x);
}

// Round x down to an integer.
float __basic_math_floorf(float x)
{
    return (float) __basic_math_floorl(x);
}

// Round x down to an integer.
long double __basic_math_floorl(long double x)
{
    long double whole = __basic_math_truncl(x);

    return _BASIC_MATH_IS_FINITE(x) && whole > x ? whole - 1 : whole;
}

// Round x to an integer in the current direction without inexact.
double __basic_math_nearbyint(double x)
{
    return (double) __basic_math_nearbyintl(x);
}

// Round x to an integer in the current direction without inexact.
float __basic_math_nearbyintf(float x)
{
    return (float) __basic_math_nearbyintl(x);
}

// Round x to an integer in the current direction without inexact.
long double __basic_math_nearbyintl(long double x)
{
    _Fpu_Env env;
    long double rounded;

    if (_BASIC_MATH_IS_NAN(x)) {
        return x + x;
    }
    _Fpu_HoldEnv(&env);
    rounded = __basic_math_rintl(x);
    _Fpu_WriteEnv(&env);
    return rounded;
}

// Round x to an integer in the current direction.
double __basic_math_rint(double x)
{
    return (double) __basic_math_rintl(x);
}

// Round x to an integer in the current direction.
float __basic_math_rintf(float x)
{
    return (float) __basic_math_rintl(x);
}

// Round x to an integer in the current direction.
long double __basic_math_rintl(long double x)
{
    long double shift = __basic_math_copysignl(_BASIC_MATH_LDOUBLE_INTEGRAL, x);

    if (! _BASIC_MATH_IS_FINITE(x)) {
        return x + x;
    }
    if (__basic_math_fabsl(x) >= _BASIC_MATH_LDOUBLE_INTEGRAL) {
        return x;
    }
    return __basic_math_copysignl((x + shift) - shift, x);
}

// Round x to a long in the current direction.
long int __basic_math_lrint(double x)
{
    return __basic_math_lrintl(x);
}

// Round x to a long in the current direction.
long int __basic_math_lrintf(float x)
{
    return __basic_math_lrintl(x);
}

// Round x to a long in the current direction.
long int __basic_math_lrintl(long double x)
{
    return (long int) __basic_math_rintl(x);
}

// Round x to a long long in the current direction.
long long int __basic_math_llrint(double x)
{
    return __basic_math_llrintl(x);
}

// Round x to a long long in the current direction.
long long int __basic_math_llrintf(float x)
{
    return __basic_math_llrintl(x);
}

// Round x to a long long in the current direction.
long long int __basic_math_llrintl(long double x)
{
    return (long long int) __basic_math_rintl(x);
}

// Round x to the nearest integer, halfway away from zero.
double __basic_math_round(double x)
{
    return (double) __basic_math_roundl(x);
}

// Round x to the nearest integer, halfway away from zero.
float __basic_math_roundf(float x)
{
    return (float) __basic_math_roundl(x);
}

// Round x to the nearest integer, halfway away from zero.
long double __basic_math_roundl(long double x)
{
    long double whole = __basic_math_truncl(x);

    if (_BASIC_MATH_IS_FINITE(x) && __basic_math_fabsl(x - whole) >= _BASIC_MATH_HALF) {
        return whole + __basic_math_copysignl(1, x);
    }
    return whole;
}

// Round x to the nearest long, halfway away from zero.
long int __basic_math_lround(double x)
{
    return __basic_math_lroundl(x);
}

// Round x to the nearest long, halfway away from zero.
long int __basic_math_lroundf(float x)
{
    return __basic_math_lroundl(x);
}

// Round x to the nearest long, halfway away from zero.
long int __basic_math_lroundl(long double x)
{
    return (long int) __basic_math_roundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int __basic_math_llround(double x)
{
    return __basic_math_llroundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int __basic_math_llroundf(float x)
{
    return __basic_math_llroundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int __basic_math_llroundl(long double x)
{
    return (long long int) __basic_math_roundl(x);
}

// Round x toward zero to an integer.
double __basic_math_trunc(double x)
{
    return (double) __basic_math_truncl(x);
}

// Round x toward zero to an integer.
float __basic_math_truncf(float x)
{
    return (float) __basic_math_truncl(x);
}

// Round x toward zero to an integer.
long double __basic_math_truncl(long double x)
{
    int exp;
    unsigned long long sig;

    if (! _BASIC_MATH_IS_FINITE(x) || x == 0) {
        return x + x;
    }
    sig = _Exact_SplitLDouble(x, &exp);
    if (exp >= _EXACT_LDOUBLE_INT_SHIFT) {
        return x;
    }
    if (exp < 0) {
        return __basic_math_copysignl(0, x);
    }
    return __basic_math_copysignl(_Exact_JoinLDouble(exp, sig & _EXACT_ALL_BITS << (_EXACT_LDOUBLE_INT_SHIFT - exp)), x);
}

// Take the remainder of x by y toward zero.
double __basic_math_fmod(double x, double y)
{
    return (double) __basic_math_fmodl(x, y);
}

// Take the remainder of x by y toward zero.
float __basic_math_fmodf(float x, float y)
{
    return (float) __basic_math_fmodl(x, y);
}

// Take the remainder of x by y toward zero.
long double __basic_math_fmodl(long double x, long double y)
{
    unsigned int q;

    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return x + y;
    }
    if (_BASIC_MATH_IS_INF(x) || y == 0) {
        return _Basic_Math_RaiseDomain(x, y);
    }
    if (_BASIC_MATH_IS_INF(y) || x == 0) {
        return x;
    }
    return _Basic_Math_TakeRemainder(x, y, _BASIC_MATH_QUOTIENT_TRUNCATED, &q);
}

// Take IEC 60559's remainder of x by y.
double __basic_math_remainder(double x, double y)
{
    return (double) __basic_math_remainderl(x, y);
}

// Take IEC 60559's remainder of x by y.
float __basic_math_remainderf(float x, float y)
{
    return (float) __basic_math_remainderl(x, y);
}

// Take IEC 60559's remainder of x by y.
long double __basic_math_remainderl(long double x, long double y)
{
    int quo;

    return __basic_math_remquol(x, y, &quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
double __basic_math_remquo(double x, double y, int *quo)
{
    return (double) __basic_math_remquol(x, y, quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
float __basic_math_remquof(float x, float y, int *quo)
{
    return (float) __basic_math_remquol(x, y, quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
long double __basic_math_remquol(long double x, long double y, int *quo)
{
    unsigned int q;
    long double rest;

    *quo = 0;
    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return x + y;
    }
    if (_BASIC_MATH_IS_INF(x) || y == 0) {
        return _Basic_Math_RaiseDomain(x, y);
    }
    if (_BASIC_MATH_IS_INF(y) || x == 0) {
        return x;
    }
    rest = _Basic_Math_TakeRemainder(x, y, _BASIC_MATH_QUOTIENT_NEAREST, &q);
    q &= _BASIC_MATH_QUOTIENT_MASK;
    *quo = _BASIC_MATH_SIGNBIT(x) == _BASIC_MATH_SIGNBIT(y) ? (int) q : -(int) q;
    return rest;
}

// Give x the sign of y.
double __basic_math_copysign(double x, double y)
{
    ((unsigned char *) &x)[_BASIC_MATH_DOUBLE_SIGN_BYTE] &= _BASIC_MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_BASIC_MATH_DOUBLE_SIGN_BYTE] |= ((unsigned char *) &y)[_BASIC_MATH_DOUBLE_SIGN_BYTE] & _BASIC_MATH_SIGN_BIT;
    return x;
}

// Give x the sign of y.
float __basic_math_copysignf(float x, float y)
{
    ((unsigned char *) &x)[_BASIC_MATH_FLOAT_SIGN_BYTE] &= _BASIC_MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_BASIC_MATH_FLOAT_SIGN_BYTE] |= ((unsigned char *) &y)[_BASIC_MATH_FLOAT_SIGN_BYTE] & _BASIC_MATH_SIGN_BIT;
    return x;
}

// Give x the sign of y.
long double __basic_math_copysignl(long double x, long double y)
{
    ((unsigned char *) &x)[_BASIC_MATH_LDOUBLE_SIGN_BYTE] &= _BASIC_MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_BASIC_MATH_LDOUBLE_SIGN_BYTE] |= ((unsigned char *) &y)[_BASIC_MATH_LDOUBLE_SIGN_BYTE] & _BASIC_MATH_SIGN_BIT;
    return x;
}

// Make a quiet NaN of tagp's payload.
double __basic_math_nan(const char *tagp)
{
    double x;
    unsigned long long bits = _BASIC_MATH_DOUBLE_QUIET | (_Basic_Math_ReadPayload(tagp) & _BASIC_MATH_DOUBLE_FRAC_MASK);

    _Str_CopyBytes(&x, &bits, sizeof(x));
    return x;
}

// Make a quiet NaN of tagp's payload.
float __basic_math_nanf(const char *tagp)
{
    float x;
    unsigned int bits = _BASIC_MATH_FLOAT_QUIET | (unsigned int) (_Basic_Math_ReadPayload(tagp) & _BASIC_MATH_FLOAT_FRAC_MASK);

    _Str_CopyBytes(&x, &bits, sizeof(x));
    return x;
}

// Make a quiet NaN of tagp's payload.
long double __basic_math_nanl(const char *tagp)
{
    return _Exact_MakeLDouble(_EXACT_LDOUBLE_EXP_MAX, _BASIC_MATH_LDOUBLE_QUIET | (_Basic_Math_ReadPayload(tagp) & _EXACT_LDOUBLE_FRAC_MASK));
}

// Step x to the next value toward y.
double __basic_math_nextafter(double x, double y)
{
    return __basic_math_nexttoward(x, y);
}

// Step x to the next value toward y.
float __basic_math_nextafterf(float x, float y)
{
    return __basic_math_nexttowardf(x, y);
}

// Step x to the next value toward y.
long double __basic_math_nextafterl(long double x, long double y)
{
    return __basic_math_nexttowardl(x, y);
}

// Step x to the next value toward a long double y.
double __basic_math_nexttoward(double x, long double y)
{
    double next;
    unsigned long long bits;

    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return (double) (x + y);
    }
    if (x == y) {
        return (double) y;
    }
    if (x == 0) {
        next = y < 0 ? -_BASIC_MATH_DOUBLE_TINY : _BASIC_MATH_DOUBLE_TINY;
    } else {
        _Str_CopyBytes(&bits, &x, sizeof(bits));
        if ((x < y) == (x > 0)) {
            bits++;
        } else {
            bits--;
        }
        _Str_CopyBytes(&next, &bits, sizeof(next));
    }
    _Basic_Math_CheckNext(x, _BASIC_MATH_CLASSIFY(next));
    return next;
}

// Step x to the next value toward a long double y.
float __basic_math_nexttowardf(float x, long double y)
{
    float next;
    unsigned int bits;

    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return (float) (x + y);
    }
    if (x == y) {
        return (float) y;
    }
    if (x == 0) {
        next = y < 0 ? -_BASIC_MATH_FLOAT_TINY : _BASIC_MATH_FLOAT_TINY;
    } else {
        _Str_CopyBytes(&bits, &x, sizeof(bits));
        if ((x < y) == (x > 0)) {
            bits++;
        } else {
            bits--;
        }
        _Str_CopyBytes(&next, &bits, sizeof(next));
    }
    _Basic_Math_CheckNext(x, _BASIC_MATH_CLASSIFY(next));
    return next;
}

// Step x to the next value toward a long double y.
long double __basic_math_nexttowardl(long double x, long double y)
{
    long double next;

    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return x + y;
    }
    if (x == y) {
        return y;
    }
    if (x == 0) {
        next = y < 0 ? -_BASIC_MATH_LDOUBLE_TINY : _BASIC_MATH_LDOUBLE_TINY;
    } else {
        next = _Exact_StepLDouble(x, (x < y) == (x > 0) ? _EXACT_STEP_UP : _EXACT_STEP_DOWN);
    }
    _Basic_Math_CheckNext(x, _BASIC_MATH_CLASSIFY(next));
    return next;
}

// Take the positive difference of x and y.
double __basic_math_fdim(double x, double y)
{
    double diff;

    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (_BASIC_MATH_IS_INF(diff) && _BASIC_MATH_IS_FINITE(x) && _BASIC_MATH_IS_FINITE(y)) {
        _Err_Errno = _SYS_ERANGE;
    }
    return diff;
}

// Take the positive difference of x and y.
float __basic_math_fdimf(float x, float y)
{
    float diff;

    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (_BASIC_MATH_IS_INF(diff) && _BASIC_MATH_IS_FINITE(x) && _BASIC_MATH_IS_FINITE(y)) {
        _Err_Errno = _SYS_ERANGE;
    }
    return diff;
}

// Take the positive difference of x and y.
long double __basic_math_fdiml(long double x, long double y)
{
    long double diff;

    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (_BASIC_MATH_IS_INF(diff) && _BASIC_MATH_IS_FINITE(x) && _BASIC_MATH_IS_FINITE(y)) {
        _Err_Errno = _SYS_ERANGE;
    }
    return diff;
}

// Take the greater of x and y.
double __basic_math_fmax(double x, double y)
{
    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return _BASIC_MATH_IS_NAN(x) ? y : x;
    }
    if (x == y) {
        return _BASIC_MATH_SIGNBIT(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the greater of x and y.
float __basic_math_fmaxf(float x, float y)
{
    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return _BASIC_MATH_IS_NAN(x) ? y : x;
    }
    if (x == y) {
        return _BASIC_MATH_SIGNBIT(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the greater of x and y.
long double __basic_math_fmaxl(long double x, long double y)
{
    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return _BASIC_MATH_IS_NAN(x) ? y : x;
    }
    if (x == y) {
        return _BASIC_MATH_SIGNBIT(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the lesser of x and y.
double __basic_math_fmin(double x, double y)
{
    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return _BASIC_MATH_IS_NAN(x) ? y : x;
    }
    if (x == y) {
        return _BASIC_MATH_SIGNBIT(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take the lesser of x and y.
float __basic_math_fminf(float x, float y)
{
    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return _BASIC_MATH_IS_NAN(x) ? y : x;
    }
    if (x == y) {
        return _BASIC_MATH_SIGNBIT(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take the lesser of x and y.
long double __basic_math_fminl(long double x, long double y)
{
    if (_BASIC_MATH_IS_NAN(x) || _BASIC_MATH_IS_NAN(y)) {
        return _BASIC_MATH_IS_NAN(x) ? y : x;
    }
    if (x == y) {
        return _BASIC_MATH_SIGNBIT(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take x times y plus z, rounded once.
double __basic_math_fma(double x, double y, double z)
{
    return (double) _Basic_Math_TakeFma(x, y, z, _EXACT_TYPE_DOUBLE);
}

// Take x times y plus z, rounded once.
float __basic_math_fmaf(float x, float y, float z)
{
    return (float) _Basic_Math_TakeFma(x, y, z, _EXACT_TYPE_FLOAT);
}

// Take x times y plus z, rounded once.
long double __basic_math_fmal(long double x, long double y, long double z)
{
    return _Basic_Math_TakeFma(x, y, z, _EXACT_TYPE_LDOUBLE);
}
