/*
 * C source file for mathematics.
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
#include <math.h>

// Includes.
#include <_fp.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

// The bits of 2/pi past its point, 32 to a limb, the most significant first.
const unsigned int _Math_TwoOverPi[_MATH_TWO_OVER_PI_LIMBS] = {
    0xA2F9836EU, 0x4E441529U, 0xFC2757D1U, 0xF534DDC0U, 0xDB629599U, 0x3C439041U, 0xFE5163ABU, 0xDEBBC561U,
    0xB7246E3AU, 0x424DD2E0U, 0x06492EEAU, 0x09D1921CU, 0xFE1DEB1CU, 0xB129A73EU, 0xE88235F5U, 0x2EBB4484U,
    0xE99C7026U, 0xB45F7E41U, 0x3991D639U, 0x835339F4U, 0x9C845F8BU, 0xBDF9283BU, 0x1FF897FFU, 0xDE05980FU,
    0xEF2F118BU, 0x5A0A6D1FU, 0x6D367ECFU, 0x27CB09B7U, 0x4F463F66U, 0x9E5FEA2DU, 0x7527BAC7U, 0xEBE5F17BU,
    0x3D0739F7U, 0x8A5292EAU, 0x6BFB5FB1U, 0x1F8D5D08U, 0x56033046U, 0xFC7B6BABU, 0xF0CFBC20U, 0x9AF4361DU,
    0xA9E39161U, 0x5EE61B08U, 0x6599855FU, 0x14A06840U, 0x8DFFD880U, 0x4D732731U, 0x06061556U, 0xCA73A8C9U,
    0x60E27BC0U, 0x8C6B47C4U, 0x19C367CDU, 0xDCE8092AU, 0x8359C476U, 0x8B961CA6U, 0xDDAF44D1U, 0x5719053EU,
    0xA5FF0705U, 0x3F7E33E8U, 0x32C2DE4FU, 0x98327DBBU, 0xC33D26EFU, 0x6B1E5EF8U, 0x9F3A1F35U, 0xCAF27F1DU,
    0x87F12190U, 0x7C7C246AU, 0xFA6ED577U, 0x2D30433BU, 0x15C614B5U, 0x9D19C3C2U, 0xC4AD414DU, 0x2C5D000CU,
    0x467D862DU, 0x71E39AC6U, 0x9B006233U, 0x7CD2B497U, 0xA7B4D555U, 0x37F63ED7U, 0x1810A3FCU, 0x764D2A9DU,
    0x64ABD770U, 0xF87C6357U, 0xB07AE715U, 0x175649C0U, 0xD9D63B38U, 0x84A7CB23U, 0x24778AD6U, 0x23545AB9U,
    0x1F001B0AU, 0xF1DFCE19U, 0xFF319F6AU, 0x1E666157U, 0x9947FBACU, 0xD87F7EB7U, 0x652289E8U, 0x3260BFE6U,
    0xCDC4EF09U, 0x366CD43FU, 0x5DD7DE16U, 0xDE3B5892U, 0x9BDE2822U, 0xD2E88628U, 0x4D58E232U, 0xCAC616E3U,
    0x08CB7DE0U, 0x50C017A7U, 0x1DF35BE0U, 0x1834132EU, 0x62128301U, 0x48835B8EU, 0xF57FB0ADU, 0xF2E91E43U,
    0x4A48D367U, 0x10D8DDAAU, 0x425FAECEU, 0x616AA428U, 0x0AB499D3U, 0xF2A6067FU, 0x775C83C2U, 0xA3883C61U,
    0x78738A5AU, 0x8CAFBDD7U, 0x6F63A62DU, 0xCBBFF4EFU, 0x818D67C1U, 0x2645CA55U, 0x36D9CAD2U, 0xA8288D61U,
    0xC277C912U, 0x1426049BU, 0x4612C459U, 0xC444C5C8U, 0x91B24DF3U, 0x1700AD43U, 0xD4E54929U, 0x10D5FDFCU,
    0xBE00CC94U, 0x1EEECE70U, 0xF53E1380U, 0xF1ECC3E7U, 0xB328F8C7U, 0x9405933EU, 0x71C1B309U, 0x2EF3450BU,
    0x9C12887BU, 0x20AB9FB5U, 0x2EC29247U, 0x2F327B6DU, 0x550C90A7U, 0x721FE76BU, 0x96CB314AU, 0x1679E279U,
    0x4189DFF4U, 0x9794E884U, 0xE6E29731U, 0x996BED88U, 0x365F5F0EU, 0xFDBBB49AU, 0x486CA467U, 0x42727132U,
    0x5D8DB815U, 0x9F09E5BCU, 0x25318D39U, 0x74F71C05U, 0x30010C0DU, 0x68084B58U, 0xEE2C90AAU, 0x4702E774U,
    0x24D6BDA6U, 0x7DF77248U, 0x6EEF169FU, 0xA6948EF6U, 0x91B45153U, 0xD1F20ACFU, 0x3398207EU, 0x4BF56863U,
    0xB25F3EDDU, 0x035D407FU, 0x89852952U, 0x55C06437U, 0x10D86D32U, 0x4832754CU, 0x5BD4714EU, 0x6E5445C1U,
    0x090B69F5U, 0x2AD56614U, 0x9D072750U, 0x045DDB3BU, 0xB4C576EAU, 0x17F9877DU, 0x6B49BA27U, 0x1D296996U,
    0xACCCC654U, 0x14AD6AE2U, 0x9089D988U, 0x50722CBEU, 0xA4049407U, 0x777030F3U, 0x27FC00A8U, 0x71EA49C2U,
    0x663DE064U, 0x83DD9797U, 0x3FA3FD94U, 0x438C860DU, 0xDE41319DU, 0x39928C70U, 0xDDE7B717U, 0x3BDF082BU,
    0x3715A080U, 0x5C93805AU, 0x921110D8U, 0xE80FAF80U, 0x6C4BFFDBU, 0x0F903876U, 0x185915A5U, 0x62BBCB61U,
    0xB989C7BDU, 0x401004F2U, 0xD2277549U, 0xF6B6EBBBU, 0x22DBAA14U, 0x0A2F2689U, 0x76836433U, 0x3B091A94U,
    0x0EAA3A51U, 0xC2A31DAEU, 0xEDAF1226U, 0x5C4DC26DU, 0x9C7A2D97U, 0x56C0833FU, 0x03F6F009U, 0x8C402B99U,
    0x316D07B4U, 0x3915200CU, 0x5BC3D8C4U, 0x92F54BADU, 0xC6A5CA4EU, 0xCD37A736U, 0xA9E69492U, 0xAB6842DDU,
    0xDE6319EFU, 0x8C76528BU, 0x6837DBFCU, 0xABA1AE31U, 0x15DFA1AEU, 0x00DAFB0CU, 0x664D64B7U, 0x05ED3065U,
    0x29BF5657U, 0x3AFF47B9U, 0xF96AF3BEU, 0x75DF9328U, 0x3080ABF6U, 0x8C6615CBU, 0x040622FAU, 0x1DE4D9A4U,
    0xB33D8F1BU, 0x5709CD36U, 0xE9424EA4U, 0xBE13B523U, 0x331AAAF0U, 0xA8654FA5U, 0xC1D20F3FU, 0x0BCD785BU,
    0x76F92304U, 0x8B7B7217U, 0x8953A6C6U, 0xE26E6F00U, 0xEBEF584AU, 0x9BB7DAC4U, 0xBA66AACFU, 0xCF761D02U,
    0xD12DF1B1U, 0xC1998C77U, 0xADC3DA48U, 0x86A05DF7U, 0xF480C62FU, 0xF0AC9AECU, 0xDDBC5C3FU, 0x6DDED01FU,
    0xC790B6DBU, 0x2A3A25A3U, 0x9AAF0093U, 0x53AD0457U, 0xB6B42D29U, 0x7E804BA7U, 0x07DA0EAAU, 0x76A1597BU,
    0x2A12162DU, 0xB7DCFDE5U, 0xFAFEDB89U, 0xFDBE896CU, 0x76E4FCA9U, 0x0670803EU, 0x156E85FFU, 0x87FD073EU,
    0x28336761U, 0x86182AEAU, 0xBD4DAFE7U, 0xB36E6D8FU, 0x3967955BU, 0xBF3148D7U, 0x8416DF30U, 0x432DC735U,
    0x6125CE70U, 0xC9B8CB30U, 0xFD6CBFA2U, 0x00A4E46CU, 0x05A0DD5AU, 0x476F21D2U, 0x1262845CU, 0xB9496170U,
    0xE0566B01U, 0x52993755U, 0x50B7D51EU, 0xC4F1335FU, 0x6E13E430U, 0x5DA92E85U, 0xC3B21D36U, 0x32A1A4B7U,
    0x08D4B1EAU, 0x21F716E4U, 0x698F77FFU, 0x2780030CU, 0x2D408DA0U, 0xCD4F99A5U, 0x20D3A2B3U, 0x0A5D2F42U,
    0xF9B4CBDAU, 0x11D0BE7DU, 0xC1DB9BBDU, 0x17AB81A2U, 0xCA5C6A08U, 0x17552E55U, 0x0027F014U, 0x7F8607E1U,
    0x640B148DU, 0x4196DEBEU, 0x872AFDDAU, 0xB6256B34U, 0x897BFEF3U, 0x059EBFB9U, 0x4F6A68A8U, 0x2A4A5AC4U,
    0x4FBCF82DU, 0x985AD795U, 0xC7F48D4DU, 0x0DA63A20U, 0x5F57A4B1U, 0x3F149538U, 0x800120CCU, 0x86DD71B6U,
    0xDEC9F560U, 0xBF11654DU, 0x6B0701ACU, 0xB08CD0C0U, 0xB2485551U, 0x0EFB1EC3U, 0x72953B06U, 0xA33540C0U,
    0x7BDC06CCU, 0x45E0FA29U, 0x4EC8CAD6U, 0x41F3E8DEU, 0x647CD864U, 0x9B31BED9U, 0xC397A4D4U, 0x5877C5E3U,
    0x6913DAF0U, 0x3C3ABA46U, 0x18465F75U, 0x55F5BDD2U, 0xC6926E5DU, 0x2EACED44U, 0x0E423E1CU, 0x87C461E9U,
    0xFD29F3D6U, 0xE7CA7C22U, 0x35916FC5U, 0xE0088DD7U, 0xFFE26A6EU, 0xC6FDB0C1U, 0x0893745DU, 0x7CB2AD6BU,
    0x9D6ECD7BU, 0x723E6A11U, 0xC6A9CFF7U, 0xDF7329BAU, 0xC9B55100U, 0xB70DB2E2U, 0x24BA7460U, 0x7DE58AD8U,
    0x742C150DU, 0x0C188194U, 0x667E1629U, 0x01767A9FU, 0xBEFDFDEFU, 0x4556367EU, 0xD913D9ECU, 0xB9BA8BFCU,
    0x97C427A8U, 0x31C36EF1U, 0x36C59456U, 0xA8D8B5A8U, 0xB40ECCCFU, 0x2D891234U, 0x576F8956U, 0x2CE3CE99U,
    0xB920D6AAU, 0x5E6B9C2AU, 0x3ECC5F11U, 0x4A0BFDFBU, 0xF4E16D3BU, 0x8E2C86E2U, 0x84D4E9A9U, 0xB4FCD1EEU,
    0xEFC9352EU, 0x61392F44U, 0x2138C8D9U, 0x1B0AFC81U, 0x6A4AFBD8U, 0x1C2F84B4U, 0x538C994EU, 0xCC2254DCU,
    0x552AD6C6U, 0xC096190BU, 0xB8701A64U, 0x9569605AU, 0x26EE523FU, 0x0F117F11U, 0xB5F4F5CBU, 0xFC2DBC34U,
    0xEEBC34CCU, 0x5DE8605EU, 0xDD9B8E67U, 0xEF3392B8U, 0x17C99B58U, 0x61BC57E1U, 0xC6835110U, 0x3ED84871U,
    0xDDDD1C2DU, 0xA118AF46U, 0x2C21D7F3U, 0x59987AD9U, 0xC0549EFAU, 0x864FFC06U, 0x56AE79E5U, 0x36228922U,
    0xAD38DC93U, 0x67AAE855U, 0x3826829BU, 0xE7CAA40DU, 0x51B13399U, 0x0ED7A948U, 0x0569F0B2U, 0x65A7887FU,
    0x974C8836U, 0xD1F9B392U, 0x214A827BU, 0x21CF98DCU, 0x9F405547U, 0xDC3A74E1U, 0x42EB67DFU, 0x9DFE5FD4U,
    0x5EA4677BU, 0x7AACBAA2U, 0xF6552388U, 0x2B55BA41U, 0x086E5986U, 0x2A218347U, 0x39E6E389U, 0xD49EE540U,
    0xFB49E956U, 0xFFCA0F1CU, 0x8A59C52BU, 0xFA94C5C1U, 0xD3CFC50FU, 0xAE5ADB86U, 0xC5476243U, 0x853B8621U,
    0x94792C87U, 0x61107B4CU, 0x2A1A2C80U, 0x12BF4390U, 0x2688893CU, 0x78E4C4A8U, 0x7BDBE5C2U, 0x3AC4EAF4U,
    0x268A67F7U, 0xBF920D2BU, 0xA365B193U, 0x3D0B7CBDU, 0xDC51A463U, 0xDD27DDE1U, 0x6919949AU, 0x9529A828U,
    0xCE68B4EDU, 0x09209F44U, 0xCA984E63U, 0x8270237CU, 0x7E32B90FU, 0x8EF5A7E7U, 0x561408F1U, 0x212A9DB5U,
    0x4D7E6F51U, 0x19A5ABF9U, 0xB5D6DF82U, 0x61DD9602U, 0x36169F3AU, 0xC4A1A283U, 0x6DED727AU, 0x8D39A9B8U,
    0x825C326BU, 0x5B2746EDU, 0x34007700U, 0xD255F4FCU, 0x4D590180U, 0x71E0E13FU, 0x89B295F3U
};

// Classify a value by its biased exponent and its fraction.
int _Math_ClassifyFields(unsigned int exp, unsigned int exp_max, unsigned long long frac)
{
    if (exp == exp_max) {
        return frac ? FP_NAN : FP_INFINITE;
    }
    if (exp == 0) {
        return frac ? FP_SUBNORMAL : FP_ZERO;
    }
    return FP_NORMAL;
}

// Classify x as a float.
int _Math_ClassifyFloat(float x)
{
    uint32_t bits;
    memcpy(&bits, &x, sizeof(bits));
    return _Math_ClassifyFields((bits >> _MATH_FLOAT_EXP_SHIFT) & _MATH_FLOAT_EXP_MAX, _MATH_FLOAT_EXP_MAX, bits & _MATH_FLOAT_FRAC_MASK);
}

// Classify x as a double.
int _Math_ClassifyDouble(double x)
{
    uint64_t bits;
    memcpy(&bits, &x, sizeof(bits));
    return _Math_ClassifyFields((uint32_t) (bits >> _MATH_DOUBLE_EXP_SHIFT) & _MATH_DOUBLE_EXP_MAX, _MATH_DOUBLE_EXP_MAX, bits & _MATH_DOUBLE_FRAC_MASK);
}

// Classify x as a long double, the x87's refused encodings as glibc does.
int _Math_ClassifyLDouble(long double x)
{
    uint64_t sig;
    uint16_t top;
    memcpy(&sig, (unsigned char *) &x + _MATH_LDOUBLE_SIGNIFICAND, sizeof(sig));
    memcpy(&top, (unsigned char *) &x + _MATH_LDOUBLE_TOP, sizeof(top));
    uint32_t exp = top & _MATH_LDOUBLE_EXP_MAX;

    if (exp == 0) {
        return sig >> _MATH_LDOUBLE_INT_SHIFT ? FP_NORMAL : _Math_ClassifyFields(exp, _MATH_LDOUBLE_EXP_MAX, sig);
    }
    if (! (sig >> _MATH_LDOUBLE_INT_SHIFT)) {
        return FP_NAN;
    }
    return _Math_ClassifyFields(exp, _MATH_LDOUBLE_EXP_MAX, sig & _MATH_LDOUBLE_FRAC_MASK);
}

// Return 1 if x's sign is negative, else 0.
int _Math_ReadSignFloat(float x)
{
    return ((unsigned char *) &x)[_MATH_FLOAT_SIGN_BYTE] >> _MATH_SIGN_SHIFT;
}

// Return 1 if x's sign is negative, else 0.
int _Math_ReadSignDouble(double x)
{
    return ((unsigned char *) &x)[_MATH_DOUBLE_SIGN_BYTE] >> _MATH_SIGN_SHIFT;
}

// Return 1 if x's sign is negative, else 0.
int _Math_ReadSignLDouble(long double x)
{
    return ((unsigned char *) &x)[_MATH_LDOUBLE_SIGN_BYTE] >> _MATH_SIGN_SHIFT;
}

// Return x < y, compared only once known to be ordered.
int _Math_CompareLess(long double x, long double y)
{
    return ! _Math_CompareUnordered(x, y) && x < y;
}

// Return x <= y, compared only once known to be ordered.
int _Math_CompareLessEqual(long double x, long double y)
{
    return ! _Math_CompareUnordered(x, y) && x <= y;
}

// Return x < y || x > y, compared only once known to be ordered.
int _Math_CompareLessGreater(long double x, long double y)
{
    return ! _Math_CompareUnordered(x, y) && x != y;
}

// Return whether x and y are unordered, by != alone, quiet on a quiet NaN.
int _Math_CompareUnordered(long double x, long double y)
{
    return x != x || y != y;
}

// Read a long double's significand.
unsigned long long _Math_ReadSignificand(long double x)
{
    uint64_t sig;

    memcpy(&sig, (unsigned char *) &x + _MATH_LDOUBLE_SIGNIFICAND, sizeof(sig));
    return sig;
}

// Read a long double's sign and biased exponent.
unsigned int _Math_ReadTop(long double x)
{
    uint16_t top;

    memcpy(&top, (unsigned char *) &x + _MATH_LDOUBLE_TOP, sizeof(top));
    return top;
}

// Make a long double of its sign and biased exponent and its significand.
long double _Math_MakeLDouble(unsigned int top, unsigned long long sig)
{
    long double x = 0.0L;
    uint64_t low = sig;
    uint16_t high = (uint16_t) top;

    memcpy((unsigned char *) &x + _MATH_LDOUBLE_SIGNIFICAND, &low, sizeof(low));
    memcpy((unsigned char *) &x + _MATH_LDOUBLE_TOP, &high, sizeof(high));
    return x;
}

// Split a long double into its normalized significand and its exponent.
unsigned long long _Math_SplitLDouble(long double x, int *exp)
{
    uint64_t sig = _Math_ReadSignificand(x);
    uint32_t biased = _Math_ReadTop(x) & _MATH_LDOUBLE_EXP_MAX;

    *exp = biased == 0 ? _MATH_LDOUBLE_EXP_LOW : (int) biased - _MATH_LDOUBLE_BIAS;
    for (; (sig & _MATH_LDOUBLE_INT_BIT) == 0; sig <<= 1) {
        (*exp)--;
    }
    return sig;
}

// Join a significand and an exponent into a long double.
long double _Math_JoinLDouble(int exp, unsigned long long sig)
{
    int shift;
    uint64_t bits = sig;

    if (bits == 0) {
        return 0.0L;
    }
    for (; (bits & _MATH_LDOUBLE_INT_BIT) == 0; bits <<= 1) {
        exp--;
    }
    if (exp >= _MATH_LDOUBLE_EXP_LOW) {
        return _Math_MakeLDouble((unsigned int) (exp + _MATH_LDOUBLE_BIAS), bits);
    }
    shift = _MATH_LDOUBLE_EXP_LOW - exp;
    return _Math_MakeLDouble(0, shift > _MATH_LDOUBLE_INT_SHIFT ? 0 : bits >> shift);
}

// Step a long double's magnitude to the next value up or down.
long double _Math_StepLDouble(long double x, _Math_Step step)
{
    uint32_t top = _Math_ReadTop(x);
    uint64_t sig = _Math_ReadSignificand(x);

    if (step == _MATH_STEP_UP && sig == UINT64_MAX) {
        sig = _MATH_LDOUBLE_INT_BIT;
        top++;
    } else if (step == _MATH_STEP_UP) {
        sig++;
    } else if ((top & _MATH_LDOUBLE_EXP_MAX) != 0 && sig == _MATH_LDOUBLE_INT_BIT) {
        top--;
        sig = (top & _MATH_LDOUBLE_EXP_MAX) == 0 ? _MATH_LDOUBLE_FRAC_MASK : UINT64_MAX;
    } else {
        sig--;
    }
    if ((top & _MATH_LDOUBLE_EXP_MAX) == 0 && sig >= _MATH_LDOUBLE_INT_BIT) {
        top++;
    }
    return _Math_MakeLDouble(top, sig);
}

// Scale x by 2 to n, rounding once.
long double _Math_ScaleLDouble(long double x, int n)
{
    for (int i = 0; i < _MATH_SCALE_STEPS && n > _MATH_LDOUBLE_EXP_HIGH; i++) {
        x *= _MATH_LDOUBLE_SCALE_UP;
        n -= _MATH_LDOUBLE_EXP_HIGH;
    }
    for (int i = 0; i < _MATH_SCALE_STEPS && n < _MATH_LDOUBLE_EXP_LOW; i++) {
        x *= _MATH_LDOUBLE_SCALE_DOWN;
        n += _MATH_LDOUBLE_STEP_DOWN;
    }
    n = n > _MATH_LDOUBLE_EXP_HIGH ? _MATH_LDOUBLE_EXP_HIGH : n < _MATH_LDOUBLE_EXP_LOW ? _MATH_LDOUBLE_EXP_LOW : n;
    return x * _Math_MakeLDouble((unsigned int) (n + _MATH_LDOUBLE_BIAS), _MATH_LDOUBLE_INT_BIT);
}

// Clamp a long exponent to an int.
int _Math_ClampExponent(long n)
{
    return n > INT_MAX ? INT_MAX : n < INT_MIN ? INT_MIN : (int) n;
}

// Take the remainder of x by y by a quotient of kind.
long double _Math_TakeRemainder(long double x, long double y, _Math_Quotient kind, unsigned int *quo)
{
    int ex;
    int ey;
    uint32_t q = 0;
    uint64_t carry = 0;
    long double sign = x;
    uint64_t rest = _Math_SplitLDouble(x, &ex);
    uint64_t div = _Math_SplitLDouble(y, &ey);

    for (int shift = ex - ey; shift >= 0; shift--) {
        if (carry != 0 || rest >= div) {
            rest -= div;
            q++;
        }
        if (shift > 0) {
            carry = rest & _MATH_LDOUBLE_INT_BIT;
            rest <<= 1;
            q <<= 1;
        }
    }
    if (kind == _MATH_QUOTIENT_NEAREST && ex >= ey && (rest > div - rest || (rest == div - rest && (q & _MATH_QUOTIENT_ODD) != 0))) {
        rest = div - rest;
        sign = -x;
        q++;
    } else if (kind == _MATH_QUOTIENT_NEAREST && ex == ey - 1 && rest > div) {
        rest = div - (rest - div);
        sign = -x;
        q = _MATH_QUOTIENT_ODD;
    }
    *quo = q;
    return copysignl(_Math_JoinLDouble(ex < ey ? ex : ey, rest), sign);
}

// Read nan's payload from tagp as strtod reads one from "NAN(tagp)".
unsigned long long _Math_ReadPayload(const char *tagp)
{
    unsigned long len = 0;

    while (tagp[len] == '_' || _Fp_IsAlnum((unsigned char) tagp[len])) {
        len++;
    }
    return tagp[len] == '\0' ? _Fp_ReadPayload(tagp, _FP_TEXT_NARROW, 0, len) : 0;
}

// Raise the domain error of x and y.
long double _Math_RaiseDomain(long double x, long double y)
{
    errno = EDOM;
    return (x * y) / (x * y);
}

// Check y, x scaled, for a range error.
void _Math_CheckScaled(long double x, long double y)
{
    if (isfinite(x) && x != 0 && (isinf(y) || y == 0)) {
        errno = ERANGE;
    }
}

// Check a step from x to a value of class for a range error.
void _Math_CheckNext(long double x, int class)
{
    if (class == FP_INFINITE) {
        feraiseexcept(FE_OVERFLOW | FE_INEXACT);
        errno = ERANGE;
    } else if ((class == FP_SUBNORMAL || class == FP_ZERO) && x != 0) {
        feraiseexcept(FE_UNDERFLOW | FE_INEXACT);
        errno = ERANGE;
    } else if (class == FP_SUBNORMAL) {
        feraiseexcept(FE_UNDERFLOW | FE_INEXACT);
    }
}

// Set w to a 64-bit value.
void _Math_WideSet(_Math_Wide *w, unsigned long long low)
{
    memset(w, 0, sizeof(*w));
    w->mw_limb[0] = (uint32_t) low;
    w->mw_limb[1] = (uint32_t) (low >> _MATH_LIMB_BITS);
}

// Read w's low 64 bits.
unsigned long long _Math_WideReadLow(const _Math_Wide *w)
{
    return (uint64_t) w->mw_limb[1] << _MATH_LIMB_BITS | w->mw_limb[0];
}

// Read w's bit at a place.
unsigned int _Math_WideReadBit(const _Math_Wide *w, int at)
{
    return w->mw_limb[at / _MATH_LIMB_BITS] >> (at % _MATH_LIMB_BITS) & _MATH_LOW_BIT;
}

// Find the place of w's top set bit.
int _Math_WideTop(const _Math_Wide *w)
{
    for (int i = _MATH_WIDE_LIMBS - 1; i >= 0; i--) {
        int bit = _MATH_LIMB_BITS - 1;

        if (w->mw_limb[i] == 0) {
            continue;
        }
        while ((w->mw_limb[i] >> bit) == 0) {
            bit--;
        }
        return i * _MATH_LIMB_BITS + bit;
    }
    return _MATH_WIDE_NONE;
}

// Shift w left by n bits, no set bit passing its top.
void _Math_WideShiftLeft(_Math_Wide *w, int n)
{
    int limbs = n / _MATH_LIMB_BITS;
    int bits = n % _MATH_LIMB_BITS;

    for (int i = _MATH_WIDE_LIMBS - 1; i >= 0; i--) {
        uint32_t high = i - limbs >= 0 ? w->mw_limb[i - limbs] : 0;
        uint32_t low = i - limbs - 1 >= 0 ? w->mw_limb[i - limbs - 1] : 0;

        w->mw_limb[i] = bits == 0 ? high : (uint32_t) (high << bits | low >> (_MATH_LIMB_BITS - bits));
    }
}

// Shift w right by n bits, returning whether a set bit fell off.
_Bool _Math_WideShiftRight(_Math_Wide *w, int n)
{
    _Bool lost = 0;
    int limbs = n / _MATH_LIMB_BITS;
    int bits = n % _MATH_LIMB_BITS;

    if (n >= _MATH_WIDE_BITS) {
        lost = _Math_WideTop(w) != _MATH_WIDE_NONE;
        memset(w, 0, sizeof(*w));
        return lost;
    }
    for (int i = 0; i < limbs; i++) {
        lost = lost || w->mw_limb[i] != 0;
    }
    lost = lost || (w->mw_limb[limbs] & ((_MATH_LOW_BIT << bits) - 1)) != 0;
    for (int i = 0; i < _MATH_WIDE_LIMBS; i++) {
        uint32_t low = i + limbs < _MATH_WIDE_LIMBS ? w->mw_limb[i + limbs] : 0;
        uint32_t high = i + limbs + 1 < _MATH_WIDE_LIMBS ? w->mw_limb[i + limbs + 1] : 0;

        w->mw_limb[i] = bits == 0 ? low : (uint32_t) (low >> bits | high << (_MATH_LIMB_BITS - bits));
    }
    return lost;
}

// Shift w left by n bits, or right by -n, returning whether a set bit fell off.
_Bool _Math_WideAlign(_Math_Wide *w, int n)
{
    if (n >= 0) {
        _Math_WideShiftLeft(w, n);
        return 0;
    }
    return _Math_WideShiftRight(w, -n);
}

// Compare a with b, negative, zero or positive.
int _Math_WideCompare(const _Math_Wide *a, const _Math_Wide *b)
{
    for (int i = _MATH_WIDE_LIMBS - 1; i >= 0; i--) {
        if (a->mw_limb[i] != b->mw_limb[i]) {
            return a->mw_limb[i] < b->mw_limb[i] ? -1 : 1;
        }
    }
    return 0;
}

// Add add to w.
void _Math_WideAdd(_Math_Wide *w, const _Math_Wide *add)
{
    uint64_t carry = 0;

    for (int i = 0; i < _MATH_WIDE_LIMBS; i++) {
        carry += (uint64_t) w->mw_limb[i] + add->mw_limb[i];
        w->mw_limb[i] = (uint32_t) carry;
        carry >>= _MATH_LIMB_BITS;
    }
}

// Subtract sub from w, sub not above it.
void _Math_WideSubtract(_Math_Wide *w, const _Math_Wide *sub)
{
    uint64_t borrow = 0;

    for (int i = 0; i < _MATH_WIDE_LIMBS; i++) {
        uint64_t diff = (uint64_t) w->mw_limb[i] - sub->mw_limb[i] - borrow;

        w->mw_limb[i] = (uint32_t) diff;
        borrow = diff >> _MATH_LIMB_BITS & _MATH_LOW_BIT;
    }
}

// Multiply a by b into to, the product below w's top.
void _Math_WideMultiply(_Math_Wide *to, const _Math_Wide *a, const _Math_Wide *b)
{
    _Math_Wide product;

    memset(&product, 0, sizeof(product));
    for (int i = 0; i < _MATH_WIDE_LIMBS; i++) {
        uint64_t carry = 0;

        for (int j = 0; i + j < _MATH_WIDE_LIMBS; j++) {
            carry += (uint64_t) a->mw_limb[i] * b->mw_limb[j] + product.mw_limb[i + j];
            product.mw_limb[i + j] = (uint32_t) carry;
            carry >>= _MATH_LIMB_BITS;
        }
    }
    *to = product;
}

// Raise base to the kth power into to, by squaring.
void _Math_WidePower(_Math_Wide *to, const _Math_Wide *base, int k)
{
    _Math_Wide square = *base;

    _Math_WideSet(to, _MATH_LOW_BIT);
    for (; k > 0; k >>= 1) {
        if ((k & 1) != 0) {
            _Math_WideMultiply(to, to, &square);
        }
        if (k > 1) {
            _Math_WideMultiply(&square, &square, &square);
        }
    }
}

// Divide w by a nonzero d, returning the remainder.
unsigned int _Math_WideDivideSmall(_Math_Wide *w, unsigned int d)
{
    uint64_t rest = 0;

    for (int i = _MATH_WIDE_LIMBS - 1; i >= 0; i--) {
        rest = rest << _MATH_LIMB_BITS | w->mw_limb[i];
        w->mw_limb[i] = (uint32_t) (rest / d);
        rest %= d;
    }
    return (unsigned int) rest;
}

// Divide a by a nonzero b into to, the quotient truncated.
void _Math_WideDivide(_Math_Wide *to, const _Math_Wide *a, const _Math_Wide *b)
{
    _Math_Wide rest;
    _Math_Wide quotient;

    _Math_WideSet(&rest, 0);
    _Math_WideSet(&quotient, 0);
    for (int i = _Math_WideTop(a); i >= 0; i--) {
        _Math_WideShiftLeft(&rest, 1);
        rest.mw_limb[0] |= _Math_WideReadBit(a, i);
        if (_Math_WideCompare(&rest, b) >= 0) {
            _Math_WideSubtract(&rest, b);
            quotient.mw_limb[i / _MATH_LIMB_BITS] |= _MATH_LOW_BIT << (i % _MATH_LIMB_BITS);
        }
    }
    *to = quotient;
}

// Make the exact value of a finite nonzero long double.
void _Math_MakeExact(_Math_Exact *exact, long double x)
{
    int exp;

    _Math_WideSet(&exact->me_sig, _Math_SplitLDouble(x, &exp));
    exact->me_unit = exp - _MATH_LDOUBLE_INT_SHIFT;
    exact->me_negative = signbit(x) != 0;
    exact->me_sticky = 0;
}

// Multiply exact values a and b, neither sticky, into product.
void _Math_MultiplyExact(_Math_Exact *product, const _Math_Exact *a, const _Math_Exact *b)
{
    int unit = a->me_unit + b->me_unit;
    _Bool negative = a->me_negative != b->me_negative;

    _Math_WideMultiply(&product->me_sig, &a->me_sig, &b->me_sig);
    product->me_unit = unit;
    product->me_negative = negative;
    product->me_sticky = 0;
}

// Add exact values a and b, neither sticky, into sum, below a window sticky.
void _Math_AddExact(_Math_Exact *sum, const _Math_Exact *a, const _Math_Exact *b)
{
    int order;
    _Bool lost;
    _Math_Wide one;
    _Math_Wide wa = a->me_sig;
    _Math_Wide wb = b->me_sig;
    int top_a = a->me_unit + _Math_WideTop(&wa);
    int top_b = b->me_unit + _Math_WideTop(&wb);
    int base = (top_a > top_b ? top_a : top_b) + 1 - _MATH_SUM_WINDOW;

    lost = _Math_WideAlign(&wa, a->me_unit - base);
    lost = _Math_WideAlign(&wb, b->me_unit - base) || lost;
    order = _Math_WideCompare(&wa, &wb);
    sum->me_unit = base;
    sum->me_sticky = lost;
    if (a->me_negative == b->me_negative) {
        sum->me_sig = wa;
        _Math_WideAdd(&sum->me_sig, &wb);
        sum->me_negative = a->me_negative;
        return;
    }
    sum->me_sig = order >= 0 ? wa : wb;
    _Math_WideSubtract(&sum->me_sig, order >= 0 ? &wb : &wa);
    sum->me_negative = order >= 0 ? a->me_negative : b->me_negative;
    if (lost) {
        _Math_WideSet(&one, _MATH_LOW_BIT);
        _Math_WideSubtract(&sum->me_sig, &one);
    }
    if (order == 0) {
        sum->me_negative = fegetround() == FE_DOWNWARD;
    }
}

// Take the degree's root of an exact radicand into root, a round bit spare.
void _Math_RootExact(_Math_Exact *root, const _Math_Exact *radicand, _Math_Root degree)
{
    _Bool lost;
    _Math_Wide rest;
    _Math_Wide next;
    _Math_Wide trial;
    _Math_Wide power;
    int k = (int) degree;
    _Math_Wide n = radicand->me_sig;
    int shift = k * _MATH_ROOT_BITS - 1 - _Math_WideTop(&n);

    while ((radicand->me_unit - shift) % k != 0) {
        shift--;
    }
    lost = _Math_WideAlign(&n, shift);
    _Math_WideSet(&rest, 0);
    _Math_WideSet(&root->me_sig, 0);
    for (int i = _MATH_ROOT_BITS - 1; i >= 0; i--) {
        _Math_WideShiftLeft(&rest, k);
        for (int j = 0; j < k; j++) {
            rest.mw_limb[0] |= _Math_WideReadBit(&n, k * i + j) << j;
        }
        _Math_WideShiftLeft(&root->me_sig, 1);
        next = root->me_sig;
        next.mw_limb[0] |= _MATH_LOW_BIT;
        _Math_WidePower(&trial, &next, k);
        _Math_WidePower(&power, &root->me_sig, k);
        _Math_WideSubtract(&trial, &power);
        if (_Math_WideCompare(&rest, &trial) >= 0) {
            _Math_WideSubtract(&rest, &trial);
            root->me_sig.mw_limb[0] |= _MATH_LOW_BIT;
        }
    }
    root->me_unit = (radicand->me_unit - shift) / k;
    root->me_negative = radicand->me_negative;
    root->me_sticky = lost || radicand->me_sticky || _Math_WideTop(&rest) != _MATH_WIDE_NONE;
}

// Round exact's significand to its top keep bits in the current direction.
unsigned long long _Math_RoundBits(const _Math_Exact *exact, int keep, int *low, _Bool *inexact)
{
    _Bool up;
    uint64_t kept;
    _Bool half = 0;
    int mode = fegetround();
    _Math_Wide rest = exact->me_sig;
    _Bool below = exact->me_sticky;

    *low = _Math_WideTop(&rest) - keep + 1;
    if (*low <= 0) {
        _Math_WideShiftLeft(&rest, -*low);
    } else {
        below = _Math_WideShiftRight(&rest, *low - 1) || below;
        half = (rest.mw_limb[0] & _MATH_LOW_BIT) != 0;
        _Math_WideShiftRight(&rest, 1);
    }
    kept = _Math_WideReadLow(&rest);
    *inexact = half || below;
    if (mode == FE_TONEAREST) {
        up = half && (below || (kept & _MATH_LOW_BIT) != 0);
    } else if (mode == FE_UPWARD || mode == FE_DOWNWARD) {
        up = *inexact && exact->me_negative == (mode == FE_DOWNWARD);
    } else {
        up = 0;
    }
    if (! up) {
        return kept;
    }
    kept++;
    if (keep > 0 && (keep == LDBL_MANT_DIG ? kept == 0 : kept >> keep != 0)) {
        kept = _MATH_LDOUBLE_INT_BIT >> (LDBL_MANT_DIG - keep);
        (*low)++;
    }
    return kept;
}

// Round an exact value once to type in the current direction, with its flags.
long double _Math_RoundExact(const _Math_Exact *exact, _Math_Type type)
{
    int low;
    int min;
    int max;
    int keep;
    _Bool tiny;
    int digits;
    _Bool inexact;
    uint64_t kept;
    _Bool infinite;
    long double value;
    int mode = fegetround();
    int top = _Math_WideTop(&exact->me_sig);
    int exp = exact->me_unit + top;

    switch (type) {
        case _MATH_TYPE_FLOAT: {
            digits = FLT_MANT_DIG;
            min = FLT_MIN_EXP + _MATH_FRACTION_EXP;
            max = FLT_MAX_EXP + _MATH_FRACTION_EXP;
        } break;
        case _MATH_TYPE_DOUBLE: {
            digits = DBL_MANT_DIG;
            min = DBL_MIN_EXP + _MATH_FRACTION_EXP;
            max = DBL_MAX_EXP + _MATH_FRACTION_EXP;
        } break;
        default: {
            digits = LDBL_MANT_DIG;
            min = LDBL_MIN_EXP + _MATH_FRACTION_EXP;
            max = LDBL_MAX_EXP + _MATH_FRACTION_EXP;
        } break;
    }
    if (top == _MATH_WIDE_NONE) {
        return exact->me_negative ? -0.0L : 0.0L;
    }
    tiny = exp < min;
    if (exp == min - 1) {
        _Math_RoundBits(exact, digits, &low, &inexact);
        tiny = low == top - digits + 1;
    }
    keep = exp < min ? digits - (min - exp) : digits;
    kept = _Math_RoundBits(exact, keep, &low, &inexact);
    if (exp >= min && exact->me_unit + low + keep - 1 > max) {
        infinite = mode == FE_TONEAREST || (mode == FE_UPWARD && ! exact->me_negative) || (mode == FE_DOWNWARD && exact->me_negative);
        feraiseexcept(FE_OVERFLOW | FE_INEXACT);
        value = infinite ? HUGE_VALL : _Math_JoinLDouble(max, UINT64_MAX << (LDBL_MANT_DIG - digits));
        return exact->me_negative ? -value : value;
    }
    if (inexact) {
        feraiseexcept(tiny ? FE_UNDERFLOW | FE_INEXACT : FE_INEXACT);
    }
    value = _Math_JoinLDouble(exact->me_unit + low + _MATH_LDOUBLE_INT_SHIFT, kept);
    return exact->me_negative ? -value : value;
}

// Take x's square or cube root, correctly rounded to type.
long double _Math_TakeRoot(long double x, _Math_Root degree, _Math_Type type)
{
    _Math_Exact root;
    _Math_Exact radicand;

    if (isnan(x)) {
        return x + x;
    }
    if (x == 0 || (isinf(x) && (degree == _MATH_ROOT_CUBE || x > 0))) {
        return x;
    }
    if (x < 0 && degree == _MATH_ROOT_SQUARE) {
        return _Math_RaiseDomain(x, 0);
    }
    _Math_MakeExact(&radicand, x);
    _Math_RootExact(&root, &radicand, degree);
    return _Math_RoundExact(&root, type);
}

// Take the hypotenuse of x and y, correctly rounded to type.
long double _Math_TakeHypot(long double x, long double y, _Math_Type type)
{
    _Math_Exact sum;
    _Math_Exact root;
    _Math_Exact side_x;
    _Math_Exact side_y;
    long double result;

    if (isinf(x) || isinf(y)) {
        return HUGE_VALL;
    }
    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x == 0 || y == 0) {
        return fabsl(x) + fabsl(y);
    }
    _Math_MakeExact(&side_x, x);
    _Math_MakeExact(&side_y, y);
    _Math_MultiplyExact(&side_x, &side_x, &side_x);
    _Math_MultiplyExact(&side_y, &side_y, &side_y);
    _Math_AddExact(&sum, &side_x, &side_y);
    _Math_RootExact(&root, &sum, _MATH_ROOT_SQUARE);
    result = _Math_RoundExact(&root, type);
    if (isinf(result)) {
        errno = ERANGE;
    }
    return result;
}

// Take x times y plus z, correctly rounded once to type.
long double _Math_TakeFma(long double x, long double y, long double z, _Math_Type type)
{
    _Math_Exact sum;
    _Math_Exact addend;
    _Math_Exact product;

    if (isnan(x) || isnan(y)) {
        return x * y + z;
    }
    if (isnan(z)) {
        return z + z;
    }
    if (isinf(x) || isinf(y)) {
        return x * y + z;
    }
    if (isinf(z)) {
        return z;
    }
    if (x == 0 || y == 0) {
        return x * y + z;
    }
    _Math_MakeExact(&product, x);
    _Math_MakeExact(&addend, y);
    _Math_MultiplyExact(&product, &product, &addend);
    if (z == 0) {
        return _Math_RoundExact(&product, type);
    }
    _Math_MakeExact(&addend, z);
    _Math_AddExact(&sum, &product, &addend);
    return _Math_RoundExact(&sum, type);
}

// Keep a working value's top bits, the rest truncated and sticky.
void _Math_TrimExact(_Math_Exact *e)
{
    int drop = _Math_WideTop(&e->me_sig) + 1 - _MATH_WORK_BITS;

    if (drop > 0) {
        e->me_sticky = _Math_WideShiftRight(&e->me_sig, drop) || e->me_sticky;
        e->me_unit += drop;
    }
}

// Multiply working values a and b into product, trimmed.
void _Math_WorkMultiply(_Math_Exact *product, const _Math_Exact *a, const _Math_Exact *b)
{
    _Math_MultiplyExact(product, a, b);
    _Math_TrimExact(product);
}

// Add nonzero working values a and b into sum, trimmed.
void _Math_WorkAdd(_Math_Exact *sum, const _Math_Exact *a, const _Math_Exact *b)
{
    _Math_AddExact(sum, a, b);
    _Math_TrimExact(sum);
}

// Divide working value a by a nonzero b, a sticky b a unit up, trimmed.
void _Math_WorkDivide(_Math_Exact *quotient, const _Math_Exact *a, const _Math_Exact *b)
{
    int shift;
    _Math_Wide one;
    _Math_Exact top = *a;
    _Math_Exact bottom = *b;
    _Bool negative = a->me_negative != b->me_negative;

    _Math_TrimExact(&top);
    _Math_TrimExact(&bottom);
    if (bottom.me_sticky) {
        _Math_WideSet(&one, _MATH_LOW_BIT);
        _Math_WideAdd(&bottom.me_sig, &one);
    }
    shift = 2 * _MATH_WORK_BITS - 1 - _Math_WideTop(&top.me_sig);
    _Math_WideShiftLeft(&top.me_sig, shift);
    _Math_WideDivide(&quotient->me_sig, &top.me_sig, &bottom.me_sig);
    quotient->me_unit = top.me_unit - shift - bottom.me_unit;
    quotient->me_negative = negative;
    quotient->me_sticky = 0;
    _Math_TrimExact(quotient);
}

// Divide working value e by a nonzero d, trimmed.
void _Math_WorkDivideSmall(_Math_Exact *e, unsigned int d)
{
    _Math_WideShiftLeft(&e->me_sig, _MATH_LIMB_BITS);
    e->me_unit -= _MATH_LIMB_BITS;
    _Math_WideDivideSmall(&e->me_sig, d);
    _Math_TrimExact(e);
}

// Make the reciprocal of a nonzero d as a working value.
void _Math_MakeReciprocal(_Math_Exact *e, unsigned int d)
{
    _Math_WideSet(&e->me_sig, _MATH_LOW_BIT);
    _Math_WideShiftLeft(&e->me_sig, _MATH_WORK_BITS);
    e->me_unit = -_MATH_WORK_BITS;
    e->me_negative = 0;
    e->me_sticky = 0;
    _Math_WorkDivideSmall(e, d);
}

// Make a constant as a value, untrimmed.
void _Math_MakeConstant(_Math_Exact *e, _Math_Constant which)
{
    const unsigned int limbs[_MATH_CONSTANTS][_MATH_CONST_LIMBS] = {{_MATH_LN2_LIMBS}, {_MATH_LOG2E_LIMBS}, {_MATH_LOG10E_LIMBS}, {_MATH_PI_LIMBS}};
    const int units[_MATH_CONSTANTS] = {_MATH_LN2_UNIT, _MATH_LOG2E_UNIT, _MATH_LOG10E_UNIT, _MATH_PI_UNIT};

    _Math_WideSet(&e->me_sig, 0);
    memcpy(e->me_sig.mw_limb, limbs[which], sizeof(limbs[which]));
    e->me_unit = units[which];
    e->me_negative = 0;
    e->me_sticky = 0;
}

// Read the integer nearest a working value, ties away from zero.
long _Math_ReadNearest(const _Math_Exact *e)
{
    long n;
    _Math_Wide w = e->me_sig;

    _Math_WideAlign(&w, e->me_unit + 1);
    n = (long) ((_Math_WideReadLow(&w) + 1) >> 1);
    return e->me_negative ? -n : n;
}

// Reduce t to r, t less n times ln 2, returning n.
long _Math_ReduceExp(_Math_Exact *r, const _Math_Exact *t)
{
    long n;
    _Math_Exact step;
    _Math_Exact constant;

    _Math_MakeConstant(&constant, _MATH_CONST_LOG2E);
    _Math_WorkMultiply(&step, t, &constant);
    n = _Math_ReadNearest(&step);
    if (n == 0) {
        *r = *t;
        return 0;
    }
    _Math_MakeExact(&step, (long double) -n);
    _Math_MakeConstant(&constant, _MATH_CONST_LN2);
    _Math_MultiplyExact(&step, &step, &constant);
    _Math_WorkAdd(r, t, &step);
    return n;
}

// Take e to r less 1, r reduced, by halvings, a series and doublings.
void _Math_ExpM1Exact(_Math_Exact *em1, const _Math_Exact *r)
{
    _Math_Exact sum;
    _Math_Exact one;
    _Math_Exact two;
    _Math_Exact series;
    _Math_Exact y = *r;

    y.me_unit -= _MATH_EXP_HALVINGS;
    _Math_MakeExact(&one, 1.0L);
    _Math_MakeExact(&two, 2.0L);
    series = one;
    for (unsigned int k = _MATH_EXP_TERMS; k >= 2; k--) {
        _Math_WorkMultiply(&series, &y, &series);
        _Math_WorkDivideSmall(&series, k);
        _Math_WorkAdd(&series, &one, &series);
    }
    _Math_WorkMultiply(em1, &y, &series);
    for (int i = 0; i < _MATH_EXP_HALVINGS; i++) {
        _Math_WorkAdd(&sum, em1, &two);
        _Math_WorkMultiply(em1, em1, &sum);
    }
}

// Take e to a nonzero working value t, clamped past every range.
void _Math_ExpExact(_Math_Exact *value, const _Math_Exact *t)
{
    long n;
    _Math_Exact r;
    _Math_Exact one;
    _Math_Exact clamped = *t;

    if (t->me_unit + _Math_WideTop(&t->me_sig) > _MATH_EXP_CLAMP_EXP) {
        _Math_MakeExact(&clamped, t->me_negative ? -_MATH_EXP_CLAMP : _MATH_EXP_CLAMP);
    }
    n = _Math_ReduceExp(&r, &clamped);
    _Math_ExpM1Exact(value, &r);
    _Math_MakeExact(&one, 1.0L);
    _Math_AddExact(value, &one, value);
    value->me_unit += (int) n;
}

// Take e to a nonzero working value t less 1, clamped past every range.
void _Math_ExpM1Clamped(_Math_Exact *em1, const _Math_Exact *t)
{
    long n;
    _Math_Exact r;
    _Math_Exact one;
    _Math_Exact clamped = *t;

    if (t->me_unit + _Math_WideTop(&t->me_sig) > _MATH_EXP_CLAMP_EXP) {
        _Math_MakeExact(&clamped, t->me_negative ? -_MATH_EXP_CLAMP : _MATH_EXP_CLAMP);
    }
    n = _Math_ReduceExp(&r, &clamped);
    _Math_ExpM1Exact(em1, &r);
    if (n != 0) {
        _Math_MakeExact(&one, 1.0L);
        _Math_AddExact(em1, &one, em1);
        em1->me_unit += (int) n;
        one.me_negative = 1;
        _Math_AddExact(em1, em1, &one);
    }
    _Math_TrimExact(em1);
}

// Split u into m - 1 and m + 1, u being m times 2 to the returned e.
int _Math_SplitLog(_Math_Exact *num, _Math_Exact *den, const _Math_Exact *u)
{
    _Math_Exact m = *u;
    _Math_Exact one;
    int top = _Math_WideTop(&m.me_sig);
    int e = m.me_unit + top;

    if (_Math_WideReadBit(&m.me_sig, top - 1) != 0) {
        e++;
    }
    m.me_unit -= e;
    _Math_TrimExact(&m);
    _Math_MakeExact(&one, 1.0L);
    _Math_WorkAdd(den, &m, &one);
    one.me_negative = 1;
    _Math_WorkAdd(num, &m, &one);
    return e;
}

// Take e times ln 2 plus 2 atanh of num over den, num over den small.
void _Math_LogExact(_Math_Exact *log, const _Math_Exact *num, const _Math_Exact *den, int e)
{
    _Math_Exact s;
    _Math_Exact z;
    _Math_Exact part;
    _Math_Exact series;
    _Math_Exact constant;

    if (e != 0) {
        _Math_MakeExact(&part, (long double) e);
        _Math_MakeConstant(&constant, _MATH_CONST_LN2);
        _Math_WorkMultiply(&part, &part, &constant);
    }
    if (_Math_WideTop(&num->me_sig) == _MATH_WIDE_NONE) {
        *log = part;
        return;
    }
    _Math_WorkDivide(&s, num, den);
    _Math_WorkMultiply(&z, &s, &s);
    _Math_MakeReciprocal(&series, 2 * _MATH_LOG_TERMS - 1);
    for (int k = _MATH_LOG_TERMS - 2; k >= 0; k--) {
        _Math_WorkMultiply(&series, &z, &series);
        _Math_MakeReciprocal(&constant, (unsigned int) (2 * k + 1));
        _Math_WorkAdd(&series, &constant, &series);
    }
    _Math_WorkMultiply(log, &s, &series);
    log->me_unit++;
    if (e != 0) {
        _Math_WorkAdd(log, &part, log);
    }
}

// Take the natural logarithm of 1 plus a nonzero working value u above -1.
void _Math_Log1pExact(_Math_Exact *log, const _Math_Exact *u)
{
    int e = 0;
    _Math_Exact w;
    _Math_Exact one;
    _Math_Exact num;
    _Math_Exact den;
    int top = u->me_unit + _Math_WideTop(&u->me_sig);

    _Math_MakeExact(&one, 1.0L);
    if (top <= (u->me_negative ? _MATH_LOG1P_LOW_EXP : _MATH_LOG1P_HIGH_EXP)) {
        num = *u;
        _Math_WorkAdd(&den, &one, &one);
        _Math_WorkAdd(&den, &den, &num);
    } else {
        _Math_AddExact(&w, &one, u);
        e = _Math_SplitLog(&num, &den, &w);
    }
    _Math_LogExact(log, &num, &den, e);
}

// Find whether x is 10 to the power k, k positive.
_Bool _Math_IsPowerTen(long double x, long k)
{
    int exp;
    _Math_Wide ten;
    _Math_Wide value;
    _Math_Wide power;

    if (k < 1 || k > _MATH_LDOUBLE_TENS) {
        return 0;
    }
    _Math_WideSet(&value, _Math_SplitLDouble(x, &exp));
    if (_Math_WideAlign(&value, exp - _MATH_LDOUBLE_INT_SHIFT)) {
        return 0;
    }
    _Math_WideSet(&ten, _MATH_TEN);
    _Math_WidePower(&power, &ten, (int) k);
    return _Math_WideCompare(&power, &value) == 0;
}

// Read whether a finite nonzero y is an integer, and which.
_Math_Parity _Math_ReadParity(long double y)
{
    int exp;
    uint64_t sig = _Math_SplitLDouble(y, &exp);
    int point = _MATH_LDOUBLE_INT_SHIFT - exp;

    if (point < 0) {
        return _MATH_PARITY_EVEN;
    }
    if (point > _MATH_LDOUBLE_INT_SHIFT || (sig & ~(UINT64_MAX << point)) != 0) {
        return _MATH_PARITY_NONE;
    }
    return (sig >> point & _MATH_LOW_BIT) != 0 ? _MATH_PARITY_ODD : _MATH_PARITY_EVEN;
}

// Make a positive x to a nonzero y, both finite, when it is exact.
_Bool _Math_PowerExact(_Math_Exact *value, long double x, long double y)
{
    int exp;
    _Math_Wide odd;
    _Math_Wide rest;
    _Math_Exact power;
    uint64_t sig = _Math_SplitLDouble(x, &exp);
    int unit = exp - _MATH_LDOUBLE_INT_SHIFT;

    for (; (sig & _MATH_LOW_BIT) == 0; sig >>= 1) {
        unit++;
    }
    _Math_WideSet(&odd, sig);
    value->me_negative = 0;
    value->me_sticky = 0;
    if (sig == _MATH_LOW_BIT && unit == 0) {
        _Math_WideSet(&value->me_sig, _MATH_LOW_BIT);
        value->me_unit = 0;
        return 1;
    }
    if (sig == _MATH_LOW_BIT) {
        _Math_MakeExact(&power, (long double) unit);
        _Math_MakeExact(value, y);
        _Math_MultiplyExact(&power, &power, value);
        rest = power.me_sig;
        if (_Math_WideAlign(&rest, power.me_unit)) {
            return 0;
        }
        _Math_WideSet(&value->me_sig, _MATH_LOW_BIT);
        value->me_negative = 0;
        if (power.me_unit + _Math_WideTop(&power.me_sig) > _MATH_EXP_CLAMP_EXP) {
            value->me_unit = power.me_negative ? -(int) _MATH_EXP_CLAMP : (int) _MATH_EXP_CLAMP;
        } else {
            value->me_unit = (int) _Math_ReadNearest(&power);
        }
        return 1;
    }
    if (y < 1 || y >= _MATH_WIDE_BITS || _Math_ReadParity(y) == _MATH_PARITY_NONE) {
        return 0;
    }
    if ((int) y * (_Math_WideTop(&odd) + 1) >= _MATH_WIDE_BITS) {
        return 0;
    }
    _Math_WidePower(&value->me_sig, &odd, (int) y);
    value->me_unit = unit * (int) y;
    return 1;
}

// Raise the pole error, an exact -inf from a finite argument.
long double _Math_RaisePole(void)
{
    errno = ERANGE;
    feraiseexcept(FE_DIVBYZERO);
    return -HUGE_VALL;
}

// Round a value to type, a range error if it rounds to an infinity or zero.
long double _Math_RoundRange(const _Math_Exact *value, _Math_Type type)
{
    long double result = _Math_RoundExact(value, type);

    if (isinf(result) || result == 0) {
        errno = ERANGE;
    }
    return result;
}

// Take e or 2 to x, within 2 ulps of type.
long double _Math_TakeExp(long double x, _Math_Base base, _Math_Type type)
{
    _Math_Exact t;
    _Math_Exact value;
    _Math_Exact constant;

    if (isnan(x)) {
        return x + x;
    }
    if (isinf(x)) {
        return x > 0 ? x : 0.0L;
    }
    if (x == 0) {
        return 1.0L;
    }
    if (fabsl(x) > _MATH_EXP_CLAMP) {
        x = copysignl(_MATH_EXP_CLAMP, x);
    }
    if (base == _MATH_BASE_2 && x == truncl(x)) {
        _Math_WideSet(&value.me_sig, _MATH_LOW_BIT);
        value.me_unit = (int) x;
        value.me_negative = 0;
        value.me_sticky = 0;
        return _Math_RoundRange(&value, type);
    }
    _Math_MakeExact(&t, x);
    if (base == _MATH_BASE_2) {
        _Math_MakeConstant(&constant, _MATH_CONST_LN2);
        _Math_WorkMultiply(&t, &t, &constant);
    }
    _Math_ExpExact(&value, &t);
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take e to x less 1, within 2 ulps of type.
long double _Math_TakeExpm1(long double x, _Math_Type type)
{
    _Math_Exact t;
    _Math_Exact value;

    if (isnan(x)) {
        return x + x;
    }
    if (isinf(x)) {
        return x > 0 ? x : -1.0L;
    }
    if (x == 0) {
        return x;
    }
    _Math_MakeExact(&t, x);
    _Math_ExpM1Clamped(&value, &t);
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take the logarithm of x to base, within 2 ulps of type.
long double _Math_TakeLog(long double x, _Math_Base base, _Math_Type type)
{
    int e;
    long k;
    _Math_Exact u;
    _Math_Exact num;
    _Math_Exact den;
    _Math_Exact value;
    _Math_Exact constant;

    if (isnan(x)) {
        return x + x;
    }
    if (x < 0) {
        return _Math_RaiseDomain(x, 0);
    }
    if (x == 0) {
        return _Math_RaisePole();
    }
    if (isinf(x)) {
        return x;
    }
    if (x == 1) {
        return 0.0L;
    }
    _Math_MakeExact(&u, x);
    if (base == _MATH_BASE_2 && _Math_WideReadLow(&u.me_sig) == _MATH_LDOUBLE_INT_BIT) {
        return (long double) (u.me_unit + _MATH_LDOUBLE_INT_SHIFT);
    }
    e = _Math_SplitLog(&num, &den, &u);
    _Math_LogExact(&value, &num, &den, e);
    if (base != _MATH_BASE_E) {
        _Math_MakeConstant(&constant, base == _MATH_BASE_2 ? _MATH_CONST_LOG2E : _MATH_CONST_LOG10E);
        _Math_WorkMultiply(&value, &value, &constant);
    }
    k = _Math_ReadNearest(&value);
    if (base == _MATH_BASE_10 && _Math_IsPowerTen(x, k)) {
        return (long double) k;
    }
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take the natural logarithm of 1 plus x, within 2 ulps of type.
long double _Math_TakeLog1p(long double x, _Math_Type type)
{
    _Math_Exact u;
    _Math_Exact value;

    if (isnan(x)) {
        return x + x;
    }
    if (x < -1) {
        return _Math_RaiseDomain(x, 0);
    }
    if (x == -1) {
        return _Math_RaisePole();
    }
    if (isinf(x) || x == 0) {
        return x;
    }
    _Math_MakeExact(&u, x);
    _Math_Log1pExact(&value, &u);
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take x to y, within 2 ulps of type.
long double _Math_TakePow(long double x, long double y, _Math_Type type)
{
    int e;
    _Bool negative;
    _Math_Exact u;
    _Math_Exact num;
    _Math_Exact den;
    _Math_Exact log;
    _Math_Exact value;
    long double result;
    _Math_Parity parity;

    if (y == 0 || x == 1) {
        return 1.0L;
    }
    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (isinf(y)) {
        return fabsl(x) == 1 ? 1.0L : (fabsl(x) < 1) == (y < 0) ? HUGE_VALL : 0.0L;
    }
    parity = _Math_ReadParity(y);
    negative = signbit(x) && parity == _MATH_PARITY_ODD;
    if (x == 0 && y < 0) {
        return negative ? _Math_RaisePole() : -_Math_RaisePole();
    }
    if (x == 0 || isinf(x)) {
        result = (x == 0) == (y < 0) ? HUGE_VALL : 0.0L;
        return negative ? -result : result;
    }
    if (x < 0 && parity == _MATH_PARITY_NONE) {
        return _Math_RaiseDomain(x, 0);
    }
    if (x > 0 && y == _MATH_POW_ROOT) {
        return _Math_TakeRoot(x, _MATH_ROOT_SQUARE, type);
    }
    if (! _Math_PowerExact(&value, fabsl(x), y)) {
        _Math_MakeExact(&u, fabsl(x));
        e = _Math_SplitLog(&num, &den, &u);
        _Math_LogExact(&log, &num, &den, e);
        _Math_MakeExact(&u, y);
        _Math_WorkMultiply(&log, &u, &log);
        _Math_ExpExact(&value, &log);
        value.me_sticky = 1;
    }
    value.me_negative = negative;
    return _Math_RoundRange(&value, type);
}

// Nudge a working value toward its true magnitude, a unit below or above.
void _Math_NudgeExact(_Math_Exact *e, _Math_Step step)
{
    int shift;
    _Math_Wide one;

    if (step == _MATH_STEP_DOWN) {
        shift = _MATH_WORK_BITS - 1 - _Math_WideTop(&e->me_sig);
        _Math_WideShiftLeft(&e->me_sig, shift);
        e->me_unit -= shift;
        _Math_WideSet(&one, _MATH_LOW_BIT);
        _Math_WideSubtract(&e->me_sig, &one);
    }
    e->me_sticky = 1;
}

// Take the square root of a positive working value a by its reciprocal.
void _Math_WorkRoot(_Math_Exact *root, const _Math_Exact *a)
{
    long double m;
    long double s;
    _Math_Wide top;
    _Math_Exact y;
    _Math_Exact one;
    _Math_Exact step;
    _Math_Exact scaled = *a;
    int exp = a->me_unit + _Math_WideTop(&a->me_sig);
    int half = (exp - (exp & 1)) / 2;

    scaled.me_unit -= 2 * half;
    top = scaled.me_sig;
    _Math_WideAlign(&top, _MATH_LDOUBLE_INT_SHIFT - _Math_WideTop(&top));
    m = _Math_JoinLDouble(exp - 2 * half, _Math_WideReadLow(&top));
    s = m;
    for (int i = 0; i < _MATH_ROOT_GUESSES; i++) {
        s = (s + m / s) / 2;
    }
    _Math_MakeExact(&y, 1 / s);
    _Math_MakeExact(&one, 1.0L);
    for (int i = 0; i < _MATH_ROOT_STEPS; i++) {
        _Math_WorkMultiply(&step, &y, &y);
        _Math_WorkMultiply(&step, &scaled, &step);
        step.me_negative = 1;
        _Math_WorkAdd(&step, &one, &step);
        _Math_WorkMultiply(&step, &y, &step);
        step.me_unit--;
        _Math_WorkAdd(&y, &y, &step);
    }
    _Math_WorkMultiply(root, &scaled, &y);
    root->me_unit += half;
}

// Make quarters of pi as a working value.
void _Math_MakeQuarters(_Math_Exact *e, unsigned int quarters)
{
    _Math_Exact count;

    _Math_MakeConstant(e, _MATH_CONST_PI);
    _Math_MakeExact(&count, (long double) quarters);
    _Math_WorkMultiply(e, e, &count);
    e->me_unit += _MATH_QUARTER_EXP;
}

// Reduce an x of 1 or more to r in [-pi/4, pi/4], returning its quadrant.
unsigned int _Math_ReduceTrig(_Math_Exact *r, long double x)
{
    int exp;
    int point;
    int first = 0;
    _Math_Wide one;
    _Math_Wide part;
    _Math_Wide chunk;
    _Math_Exact half_pi;
    unsigned int quadrant = 0;
    uint64_t sig = _Math_SplitLDouble(x, &exp);
    int unit = exp - _MATH_LDOUBLE_INT_SHIFT;

    if (unit > _MATH_QUADRANT_BITS) {
        first = (unit - _MATH_QUADRANT_BITS) / _MATH_LIMB_BITS;
    }
    _Math_WideSet(&chunk, 0);
    for (int i = 0; i < _MATH_TRIG_WINDOW; i++) {
        chunk.mw_limb[i] = _Math_TwoOverPi[first + _MATH_TRIG_WINDOW - 1 - i];
    }
    _Math_WideSet(&part, sig);
    _Math_WideMultiply(&r->me_sig, &part, &chunk);
    point = _MATH_LIMB_BITS * (first + _MATH_TRIG_WINDOW) - unit;
    for (int i = _MATH_QUADRANT_BITS - 1; i >= 0; i--) {
        if (point + i < _MATH_WIDE_BITS) {
            quadrant = quadrant << 1 | _Math_WideReadBit(&r->me_sig, point + i);
        }
    }
    _Math_WideShiftLeft(&r->me_sig, _MATH_WIDE_BITS - point);
    _Math_WideShiftRight(&r->me_sig, _MATH_WIDE_BITS - point);
    r->me_unit = -point;
    r->me_negative = 0;
    r->me_sticky = 0;
    if (_Math_WideReadBit(&r->me_sig, point - 1) != 0) {
        _Math_WideSet(&one, _MATH_LOW_BIT);
        _Math_WideShiftLeft(&one, point);
        _Math_WideSubtract(&one, &r->me_sig);
        r->me_sig = one;
        r->me_negative = 1;
        quadrant++;
    }
    _Math_TrimExact(r);
    _Math_MakeQuarters(&half_pi, _MATH_HALF_PI);
    _Math_WorkMultiply(r, r, &half_pi);
    return quadrant & _MATH_QUADRANT_MASK;
}

// Take the sine or the cosine of a reduced r by its series.
void _Math_TrigExact(_Math_Exact *value, const _Math_Exact *r, _Math_Trig which)
{
    _Math_Exact z;
    _Math_Exact one;
    _Math_Exact series;
    unsigned int n = which == _MATH_TRIG_SIN ? _MATH_TRIG_TERMS - 1 : _MATH_TRIG_TERMS;

    _Math_WorkMultiply(&z, r, r);
    _Math_MakeExact(&one, 1.0L);
    series = one;
    for (; n >= 2; n -= 2) {
        _Math_WorkMultiply(&series, &z, &series);
        _Math_WorkDivideSmall(&series, n * (n - 1));
        series.me_negative = 1;
        _Math_WorkAdd(&series, &one, &series);
    }
    if (which == _MATH_TRIG_SIN) {
        _Math_WorkMultiply(value, r, &series);
    } else {
        *value = series;
    }
}

// Take the arc tangent of num over den, both positive.
void _Math_AtanExact(_Math_Exact *value, const _Math_Exact *num, const _Math_Exact *den)
{
    _Math_Exact t;
    _Math_Exact z;
    _Math_Exact one;
    _Math_Exact root;
    _Math_Exact term;
    _Math_Exact quarter;
    _Bool swap = 0;

    _Math_WorkDivide(&t, num, den);
    if (t.me_unit + _Math_WideTop(&t.me_sig) >= 0) {
        _Math_WorkDivide(&t, den, num);
        swap = 1;
    }
    if (t.me_unit + _Math_WideTop(&t.me_sig) < _MATH_TINY_EXP) {
        *value = t;
        _Math_NudgeExact(value, _MATH_STEP_DOWN);
    } else {
        _Math_MakeExact(&one, 1.0L);
        for (int i = 0; i < _MATH_ATAN_HALVINGS; i++) {
            _Math_WorkMultiply(&z, &t, &t);
            _Math_WorkAdd(&z, &z, &one);
            _Math_WorkRoot(&root, &z);
            _Math_WorkAdd(&root, &root, &one);
            _Math_WorkDivide(&t, &t, &root);
        }
        _Math_WorkMultiply(&z, &t, &t);
        _Math_MakeReciprocal(value, 2 * _MATH_ATAN_TERMS - 1);
        for (int k = _MATH_ATAN_TERMS - 2; k >= 0; k--) {
            _Math_WorkMultiply(value, &z, value);
            value->me_negative = 1;
            _Math_MakeReciprocal(&term, (unsigned int) (2 * k + 1));
            _Math_WorkAdd(value, &term, value);
        }
        _Math_WorkMultiply(value, &t, value);
        value->me_unit += _MATH_ATAN_HALVINGS;
    }
    if (swap) {
        _Math_MakeQuarters(&quarter, _MATH_HALF_PI);
        value->me_negative = 1;
        _Math_WorkAdd(value, &quarter, value);
    }
}

// Take a tiny x's function, x nudged by step, to type.
long double _Math_TakeTiny(long double x, _Math_Step step, _Math_Type type)
{
    _Math_Exact value;

    _Math_MakeExact(&value, x);
    _Math_NudgeExact(&value, step);
    return _Math_RoundRange(&value, type);
}

// Take quarters of pi to type, with sign's sign.
long double _Math_TakeQuarters(unsigned int quarters, long double sign, _Math_Type type)
{
    _Math_Exact value;

    _Math_MakeQuarters(&value, quarters);
    value.me_negative = signbit(sign) != 0;
    value.me_sticky = 1;
    return _Math_RoundExact(&value, type);
}

// Take the sine, the cosine or the tangent of x, within 2 ulps of type.
long double _Math_TakeTrig(long double x, _Math_Trig which, _Math_Type type)
{
    _Bool odd;
    _Bool flip;
    _Math_Exact r;
    _Math_Exact sine;
    _Math_Exact value;
    _Math_Exact cosine;
    unsigned int quadrant = 0;

    if (isnan(x)) {
        return x + x;
    }
    if (isinf(x)) {
        return _Math_RaiseDomain(x, 0);
    }
    if (x == 0) {
        return which == _MATH_TRIG_COS ? 1.0L : x;
    }
    if (fabsl(x) < _MATH_TINY && which == _MATH_TRIG_COS) {
        return _Math_TakeTiny(1.0L, _MATH_STEP_DOWN, type);
    }
    if (fabsl(x) < _MATH_TINY) {
        return _Math_TakeTiny(x, which == _MATH_TRIG_SIN ? _MATH_STEP_DOWN : _MATH_STEP_UP, type);
    }
    if (fabsl(x) < 1) {
        _Math_MakeExact(&r, fabsl(x));
    } else {
        quadrant = _Math_ReduceTrig(&r, fabsl(x));
    }
    odd = (quadrant & _MATH_QUADRANT_ODD) != 0;
    switch (which) {
        case _MATH_TRIG_SIN: {
            _Math_TrigExact(&value, &r, odd ? _MATH_TRIG_COS : _MATH_TRIG_SIN);
            flip = ((quadrant & _MATH_QUADRANT_HALF) != 0) != (signbit(x) != 0);
        } break;
        case _MATH_TRIG_COS: {
            _Math_TrigExact(&value, &r, odd ? _MATH_TRIG_SIN : _MATH_TRIG_COS);
            flip = ((quadrant + _MATH_QUADRANT_ODD) & _MATH_QUADRANT_HALF) != 0;
        } break;
        default: {
            _Math_TrigExact(&sine, &r, _MATH_TRIG_SIN);
            _Math_TrigExact(&cosine, &r, _MATH_TRIG_COS);
            _Math_WorkDivide(&value, odd ? &cosine : &sine, odd ? &sine : &cosine);
            flip = odd != (signbit(x) != 0);
        } break;
    }
    if (flip) {
        value.me_negative = ! value.me_negative;
    }
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take the arc sine or the arc cosine of x, within 2 ulps of type.
long double _Math_TakeArc(long double x, _Math_Trig which, _Math_Type type)
{
    _Math_Exact one;
    _Math_Exact low;
    _Math_Exact high;
    _Math_Exact side;
    _Math_Exact root;
    _Math_Exact value;
    _Math_Exact whole;

    if (isnan(x)) {
        return x + x;
    }
    if (fabsl(x) > 1) {
        return _Math_RaiseDomain(x, 0);
    }
    if (which == _MATH_TRIG_SIN && x == 0) {
        return x;
    }
    if (which == _MATH_TRIG_SIN && fabsl(x) < _MATH_TINY) {
        return _Math_TakeTiny(x, _MATH_STEP_UP, type);
    }
    if (which == _MATH_TRIG_SIN && fabsl(x) == 1) {
        return _Math_TakeQuarters(_MATH_HALF_PI, x, type);
    }
    if (which == _MATH_TRIG_COS && x == 1) {
        return 0.0L;
    }
    if (which == _MATH_TRIG_COS && (x == 0 || x == -1)) {
        return _Math_TakeQuarters(x == 0 ? _MATH_HALF_PI : _MATH_WHOLE_PI, 1.0L, type);
    }
    _Math_MakeExact(&side, fabsl(x));
    _Math_MakeExact(&one, 1.0L);
    _Math_WorkAdd(&high, &one, &side);
    side.me_negative = 1;
    _Math_WorkAdd(&low, &one, &side);
    side.me_negative = 0;
    _Math_WorkMultiply(&low, &low, &high);
    _Math_WorkRoot(&root, &low);
    if (which == _MATH_TRIG_SIN) {
        _Math_AtanExact(&value, &side, &root);
        value.me_negative = x < 0;
    } else {
        _Math_AtanExact(&value, &root, &side);
    }
    if (which == _MATH_TRIG_COS && x < 0) {
        _Math_MakeQuarters(&whole, _MATH_WHOLE_PI);
        value.me_negative = 1;
        _Math_WorkAdd(&value, &whole, &value);
    }
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take the arc tangent of y over x by both signs, within 2 ulps of type.
long double _Math_TakeAtan2(long double y, long double x, _Math_Type type)
{
    _Math_Exact num;
    _Math_Exact den;
    _Math_Exact value;
    _Math_Exact whole;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (y == 0) {
        return signbit(x) ? _Math_TakeQuarters(_MATH_WHOLE_PI, y, type) : y;
    }
    if (isinf(y) && isinf(x)) {
        return _Math_TakeQuarters(x > 0 ? _MATH_QUARTER_PI : _MATH_THREE_QUARTER_PI, y, type);
    }
    if (isinf(y) || x == 0) {
        return _Math_TakeQuarters(_MATH_HALF_PI, y, type);
    }
    if (isinf(x)) {
        return x > 0 ? copysignl(0.0L, y) : _Math_TakeQuarters(_MATH_WHOLE_PI, y, type);
    }
    _Math_MakeExact(&num, fabsl(y));
    _Math_MakeExact(&den, fabsl(x));
    _Math_AtanExact(&value, &num, &den);
    if (x < 0) {
        _Math_MakeQuarters(&whole, _MATH_WHOLE_PI);
        value.me_negative = 1;
        _Math_WorkAdd(&value, &whole, &value);
    }
    value.me_negative = signbit(y) != 0;
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take the hyperbolic sine, cosine or tangent of x, within 2 ulps of type.
long double _Math_TakeHyperbolic(long double x, _Math_Trig which, _Math_Type type)
{
    _Math_Exact t;
    _Math_Exact em1;
    _Math_Exact one;
    _Math_Exact sum;
    _Math_Exact part;
    _Math_Exact value;

    if (isnan(x)) {
        return x + x;
    }
    if (isinf(x)) {
        return which == _MATH_TRIG_COS ? HUGE_VALL : which == _MATH_TRIG_SIN ? x : copysignl(1.0L, x);
    }
    if (x == 0) {
        return which == _MATH_TRIG_COS ? 1.0L : x;
    }
    if (fabsl(x) < _MATH_TINY && which == _MATH_TRIG_COS) {
        return _Math_TakeTiny(1.0L, _MATH_STEP_UP, type);
    }
    if (fabsl(x) < _MATH_TINY) {
        return _Math_TakeTiny(x, which == _MATH_TRIG_SIN ? _MATH_STEP_UP : _MATH_STEP_DOWN, type);
    }
    _Math_MakeExact(&t, fabsl(x));
    if (which == _MATH_TRIG_TAN) {
        t.me_unit++;
    }
    _Math_ExpM1Clamped(&em1, &t);
    _Math_MakeExact(&one, 1.0L);
    switch (which) {
        case _MATH_TRIG_SIN: {
            _Math_WorkAdd(&sum, &em1, &one);
            _Math_WorkDivide(&part, &em1, &sum);
            _Math_WorkAdd(&value, &em1, &part);
            value.me_unit--;
        } break;
        case _MATH_TRIG_COS: {
            _Math_WorkAdd(&sum, &em1, &one);
            _Math_WorkMultiply(&part, &em1, &em1);
            _Math_WorkDivide(&value, &part, &sum);
            value.me_unit--;
            _Math_WorkAdd(&value, &one, &value);
        } break;
        default: {
            _Math_WorkAdd(&sum, &one, &one);
            _Math_WorkAdd(&sum, &em1, &sum);
            _Math_WorkDivide(&value, &em1, &sum);
        } break;
    }
    value.me_negative = which != _MATH_TRIG_COS && x < 0;
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take the inverse hyperbolic sine, cosine or tangent of x, within 2 ulps.
long double _Math_TakeArcHyperbolic(long double x, _Math_Trig which, _Math_Type type)
{
    _Math_Exact a;
    _Math_Exact u;
    _Math_Exact one;
    _Math_Exact sum;
    _Math_Exact part;
    _Math_Exact root;
    _Math_Exact value;

    if (isnan(x)) {
        return x + x;
    }
    if ((which == _MATH_TRIG_COS && x < 1) || (which == _MATH_TRIG_TAN && fabsl(x) > 1)) {
        return _Math_RaiseDomain(x, 0);
    }
    if (which == _MATH_TRIG_TAN && fabsl(x) == 1) {
        return x > 0 ? -_Math_RaisePole() : _Math_RaisePole();
    }
    if (isinf(x) || x == 0) {
        return x;
    }
    if (which == _MATH_TRIG_COS && x == 1) {
        return 0.0L;
    }
    if (fabsl(x) < _MATH_TINY) {
        return _Math_TakeTiny(x, which == _MATH_TRIG_SIN ? _MATH_STEP_DOWN : _MATH_STEP_UP, type);
    }
    _Math_MakeExact(&a, fabsl(x));
    _Math_MakeExact(&one, 1.0L);
    switch (which) {
        case _MATH_TRIG_SIN: {
            _Math_MultiplyExact(&part, &a, &a);
            _Math_WorkAdd(&sum, &part, &one);
            _Math_WorkRoot(&root, &sum);
            _Math_WorkAdd(&root, &root, &one);
            _Math_WorkDivide(&part, &part, &root);
            _Math_WorkAdd(&u, &a, &part);
        } break;
        case _MATH_TRIG_COS: {
            _Math_WorkAdd(&sum, &a, &one);
            one.me_negative = 1;
            _Math_WorkAdd(&u, &a, &one);
            _Math_WorkMultiply(&part, &u, &sum);
            _Math_WorkRoot(&root, &part);
            _Math_WorkAdd(&u, &u, &root);
        } break;
        default: {
            a.me_negative = 1;
            _Math_AddExact(&sum, &one, &a);
            a.me_negative = 0;
            _Math_WorkDivide(&u, &a, &sum);
            u.me_unit++;
        } break;
    }
    _Math_Log1pExact(&value, &u);
    if (which == _MATH_TRIG_TAN) {
        value.me_unit--;
    }
    value.me_negative = which != _MATH_TRIG_COS && x < 0;
    value.me_sticky = 1;
    return _Math_RoundRange(&value, type);
}

// Take the arc cosine of x.
double acos(double x)
{
    return (double) _Math_TakeArc(x, _MATH_TRIG_COS, _MATH_TYPE_DOUBLE);
}

// Take the arc cosine of x.
float acosf(float x)
{
    return (float) _Math_TakeArc(x, _MATH_TRIG_COS, _MATH_TYPE_FLOAT);
}

// Take the arc cosine of x.
long double acosl(long double x)
{
    return _Math_TakeArc(x, _MATH_TRIG_COS, _MATH_TYPE_LDOUBLE);
}

// Take the arc sine of x.
double asin(double x)
{
    return (double) _Math_TakeArc(x, _MATH_TRIG_SIN, _MATH_TYPE_DOUBLE);
}

// Take the arc sine of x.
float asinf(float x)
{
    return (float) _Math_TakeArc(x, _MATH_TRIG_SIN, _MATH_TYPE_FLOAT);
}

// Take the arc sine of x.
long double asinl(long double x)
{
    return _Math_TakeArc(x, _MATH_TRIG_SIN, _MATH_TYPE_LDOUBLE);
}

// Take the arc tangent of x.
double atan(double x)
{
    return (double) _Math_TakeAtan2(x, 1.0L, _MATH_TYPE_DOUBLE);
}

// Take the arc tangent of x.
float atanf(float x)
{
    return (float) _Math_TakeAtan2(x, 1.0L, _MATH_TYPE_FLOAT);
}

// Take the arc tangent of x.
long double atanl(long double x)
{
    return _Math_TakeAtan2(x, 1.0L, _MATH_TYPE_LDOUBLE);
}

// Take the arc tangent of y over x by both signs.
double atan2(double y, double x)
{
    return (double) _Math_TakeAtan2(y, x, _MATH_TYPE_DOUBLE);
}

// Take the arc tangent of y over x by both signs.
float atan2f(float y, float x)
{
    return (float) _Math_TakeAtan2(y, x, _MATH_TYPE_FLOAT);
}

// Take the arc tangent of y over x by both signs.
long double atan2l(long double y, long double x)
{
    return _Math_TakeAtan2(y, x, _MATH_TYPE_LDOUBLE);
}

// Take the cosine of x.
double cos(double x)
{
    return (double) _Math_TakeTrig(x, _MATH_TRIG_COS, _MATH_TYPE_DOUBLE);
}

// Take the cosine of x.
float cosf(float x)
{
    return (float) _Math_TakeTrig(x, _MATH_TRIG_COS, _MATH_TYPE_FLOAT);
}

// Take the cosine of x.
long double cosl(long double x)
{
    return _Math_TakeTrig(x, _MATH_TRIG_COS, _MATH_TYPE_LDOUBLE);
}

// Take the sine of x.
double sin(double x)
{
    return (double) _Math_TakeTrig(x, _MATH_TRIG_SIN, _MATH_TYPE_DOUBLE);
}

// Take the sine of x.
float sinf(float x)
{
    return (float) _Math_TakeTrig(x, _MATH_TRIG_SIN, _MATH_TYPE_FLOAT);
}

// Take the sine of x.
long double sinl(long double x)
{
    return _Math_TakeTrig(x, _MATH_TRIG_SIN, _MATH_TYPE_LDOUBLE);
}

// Take the tangent of x.
double tan(double x)
{
    return (double) _Math_TakeTrig(x, _MATH_TRIG_TAN, _MATH_TYPE_DOUBLE);
}

// Take the tangent of x.
float tanf(float x)
{
    return (float) _Math_TakeTrig(x, _MATH_TRIG_TAN, _MATH_TYPE_FLOAT);
}

// Take the tangent of x.
long double tanl(long double x)
{
    return _Math_TakeTrig(x, _MATH_TRIG_TAN, _MATH_TYPE_LDOUBLE);
}

// Take the inverse hyperbolic cosine of x.
double acosh(double x)
{
    return (double) _Math_TakeArcHyperbolic(x, _MATH_TRIG_COS, _MATH_TYPE_DOUBLE);
}

// Take the inverse hyperbolic cosine of x.
float acoshf(float x)
{
    return (float) _Math_TakeArcHyperbolic(x, _MATH_TRIG_COS, _MATH_TYPE_FLOAT);
}

// Take the inverse hyperbolic cosine of x.
long double acoshl(long double x)
{
    return _Math_TakeArcHyperbolic(x, _MATH_TRIG_COS, _MATH_TYPE_LDOUBLE);
}

// Take the inverse hyperbolic sine of x.
double asinh(double x)
{
    return (double) _Math_TakeArcHyperbolic(x, _MATH_TRIG_SIN, _MATH_TYPE_DOUBLE);
}

// Take the inverse hyperbolic sine of x.
float asinhf(float x)
{
    return (float) _Math_TakeArcHyperbolic(x, _MATH_TRIG_SIN, _MATH_TYPE_FLOAT);
}

// Take the inverse hyperbolic sine of x.
long double asinhl(long double x)
{
    return _Math_TakeArcHyperbolic(x, _MATH_TRIG_SIN, _MATH_TYPE_LDOUBLE);
}

// Take the inverse hyperbolic tangent of x.
double atanh(double x)
{
    return (double) _Math_TakeArcHyperbolic(x, _MATH_TRIG_TAN, _MATH_TYPE_DOUBLE);
}

// Take the inverse hyperbolic tangent of x.
float atanhf(float x)
{
    return (float) _Math_TakeArcHyperbolic(x, _MATH_TRIG_TAN, _MATH_TYPE_FLOAT);
}

// Take the inverse hyperbolic tangent of x.
long double atanhl(long double x)
{
    return _Math_TakeArcHyperbolic(x, _MATH_TRIG_TAN, _MATH_TYPE_LDOUBLE);
}

// Take the hyperbolic cosine of x.
double cosh(double x)
{
    return (double) _Math_TakeHyperbolic(x, _MATH_TRIG_COS, _MATH_TYPE_DOUBLE);
}

// Take the hyperbolic cosine of x.
float coshf(float x)
{
    return (float) _Math_TakeHyperbolic(x, _MATH_TRIG_COS, _MATH_TYPE_FLOAT);
}

// Take the hyperbolic cosine of x.
long double coshl(long double x)
{
    return _Math_TakeHyperbolic(x, _MATH_TRIG_COS, _MATH_TYPE_LDOUBLE);
}

// Take the hyperbolic sine of x.
double sinh(double x)
{
    return (double) _Math_TakeHyperbolic(x, _MATH_TRIG_SIN, _MATH_TYPE_DOUBLE);
}

// Take the hyperbolic sine of x.
float sinhf(float x)
{
    return (float) _Math_TakeHyperbolic(x, _MATH_TRIG_SIN, _MATH_TYPE_FLOAT);
}

// Take the hyperbolic sine of x.
long double sinhl(long double x)
{
    return _Math_TakeHyperbolic(x, _MATH_TRIG_SIN, _MATH_TYPE_LDOUBLE);
}

// Take the hyperbolic tangent of x.
double tanh(double x)
{
    return (double) _Math_TakeHyperbolic(x, _MATH_TRIG_TAN, _MATH_TYPE_DOUBLE);
}

// Take the hyperbolic tangent of x.
float tanhf(float x)
{
    return (float) _Math_TakeHyperbolic(x, _MATH_TRIG_TAN, _MATH_TYPE_FLOAT);
}

// Take the hyperbolic tangent of x.
long double tanhl(long double x)
{
    return _Math_TakeHyperbolic(x, _MATH_TRIG_TAN, _MATH_TYPE_LDOUBLE);
}

// Take e to x.
double exp(double x)
{
    return (double) _Math_TakeExp(x, _MATH_BASE_E, _MATH_TYPE_DOUBLE);
}

// Take e to x.
float expf(float x)
{
    return (float) _Math_TakeExp(x, _MATH_BASE_E, _MATH_TYPE_FLOAT);
}

// Take e to x.
long double expl(long double x)
{
    return _Math_TakeExp(x, _MATH_BASE_E, _MATH_TYPE_LDOUBLE);
}

// Take 2 to x.
double exp2(double x)
{
    return (double) _Math_TakeExp(x, _MATH_BASE_2, _MATH_TYPE_DOUBLE);
}

// Take 2 to x.
float exp2f(float x)
{
    return (float) _Math_TakeExp(x, _MATH_BASE_2, _MATH_TYPE_FLOAT);
}

// Take 2 to x.
long double exp2l(long double x)
{
    return _Math_TakeExp(x, _MATH_BASE_2, _MATH_TYPE_LDOUBLE);
}

// Take e to x, less 1.
double expm1(double x)
{
    return (double) _Math_TakeExpm1(x, _MATH_TYPE_DOUBLE);
}

// Take e to x, less 1.
float expm1f(float x)
{
    return (float) _Math_TakeExpm1(x, _MATH_TYPE_FLOAT);
}

// Take e to x, less 1.
long double expm1l(long double x)
{
    return _Math_TakeExpm1(x, _MATH_TYPE_LDOUBLE);
}

// Break value into a fraction and a power of 2.
double frexp(double value, int *exp)
{
    return (double) frexpl(value, exp);
}

// Break value into a fraction and a power of 2.
float frexpf(float value, int *exp)
{
    return (float) frexpl(value, exp);
}

// Break value into a fraction and a power of 2.
long double frexpl(long double value, int *exp)
{
    int power;
    uint64_t sig;

    *exp = 0;
    if (! isfinite(value) || value == 0) {
        return value + value;
    }
    sig = _Math_SplitLDouble(value, &power);
    *exp = power - _MATH_FRACTION_EXP;
    return copysignl(_Math_JoinLDouble(_MATH_FRACTION_EXP, sig), value);
}

// Extract x's exponent as an int.
int ilogb(double x)
{
    return ilogbl(x);
}

// Extract x's exponent as an int.
int ilogbf(float x)
{
    return ilogbl(x);
}

// Extract x's exponent as an int.
int ilogbl(long double x)
{
    int exp;
    int class = fpclassify(x);

    if (class == FP_NAN || class == FP_INFINITE || class == FP_ZERO) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        return class == FP_INFINITE ? INT_MAX : class == FP_ZERO ? FP_ILOGB0 : FP_ILOGBNAN;
    }
    _Math_SplitLDouble(x, &exp);
    return exp;
}

// Multiply x by 2 to exp.
double ldexp(double x, int exp)
{
    return scalbn(x, exp);
}

// Multiply x by 2 to exp.
float ldexpf(float x, int exp)
{
    return scalbnf(x, exp);
}

// Multiply x by 2 to exp.
long double ldexpl(long double x, int exp)
{
    return scalbnl(x, exp);
}

// Take the natural logarithm of x.
double log(double x)
{
    return (double) _Math_TakeLog(x, _MATH_BASE_E, _MATH_TYPE_DOUBLE);
}

// Take the natural logarithm of x.
float logf(float x)
{
    return (float) _Math_TakeLog(x, _MATH_BASE_E, _MATH_TYPE_FLOAT);
}

// Take the natural logarithm of x.
long double logl(long double x)
{
    return _Math_TakeLog(x, _MATH_BASE_E, _MATH_TYPE_LDOUBLE);
}

// Take the base-10 logarithm of x.
double log10(double x)
{
    return (double) _Math_TakeLog(x, _MATH_BASE_10, _MATH_TYPE_DOUBLE);
}

// Take the base-10 logarithm of x.
float log10f(float x)
{
    return (float) _Math_TakeLog(x, _MATH_BASE_10, _MATH_TYPE_FLOAT);
}

// Take the base-10 logarithm of x.
long double log10l(long double x)
{
    return _Math_TakeLog(x, _MATH_BASE_10, _MATH_TYPE_LDOUBLE);
}

// Take the natural logarithm of 1 plus x.
double log1p(double x)
{
    return (double) _Math_TakeLog1p(x, _MATH_TYPE_DOUBLE);
}

// Take the natural logarithm of 1 plus x.
float log1pf(float x)
{
    return (float) _Math_TakeLog1p(x, _MATH_TYPE_FLOAT);
}

// Take the natural logarithm of 1 plus x.
long double log1pl(long double x)
{
    return _Math_TakeLog1p(x, _MATH_TYPE_LDOUBLE);
}

// Take the base-2 logarithm of x.
double log2(double x)
{
    return (double) _Math_TakeLog(x, _MATH_BASE_2, _MATH_TYPE_DOUBLE);
}

// Take the base-2 logarithm of x.
float log2f(float x)
{
    return (float) _Math_TakeLog(x, _MATH_BASE_2, _MATH_TYPE_FLOAT);
}

// Take the base-2 logarithm of x.
long double log2l(long double x)
{
    return _Math_TakeLog(x, _MATH_BASE_2, _MATH_TYPE_LDOUBLE);
}

// Extract x's exponent as a floating value.
double logb(double x)
{
    return (double) logbl(x);
}

// Extract x's exponent as a floating value.
float logbf(float x)
{
    return (float) logbl(x);
}

// Extract x's exponent as a floating value.
long double logbl(long double x)
{
    int exp;

    if (isnan(x)) {
        return x + x;
    }
    if (isinf(x)) {
        return HUGE_VALL;
    }
    if (x == 0) {
        feraiseexcept(FE_DIVBYZERO);
        return -HUGE_VALL;
    }
    _Math_SplitLDouble(x, &exp);
    return exp;
}

// Break value into its integral and fractional parts.
double modf(double value, double *iptr)
{
    long double whole;
    double fraction = (double) modfl(value, &whole);

    *iptr = (double) whole;
    return fraction;
}

// Break value into its integral and fractional parts.
float modff(float value, float *iptr)
{
    long double whole;
    float fraction = (float) modfl(value, &whole);

    *iptr = (float) whole;
    return fraction;
}

// Break value into its integral and fractional parts.
long double modfl(long double value, long double *iptr)
{
    *iptr = truncl(value);
    if (isinf(value)) {
        return copysignl(0, value);
    }
    return copysignl(value - *iptr, value);
}

// Scale x by FLT_RADIX to n.
double scalbn(double x, int n)
{
    double y = (double) _Math_ScaleLDouble(x, n);

    _Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to n.
float scalbnf(float x, int n)
{
    float y = (float) _Math_ScaleLDouble(x, n);

    _Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to n.
long double scalbnl(long double x, int n)
{
    long double y = _Math_ScaleLDouble(x, n);

    _Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to a long n.
double scalbln(double x, long int n)
{
    return scalbn(x, _Math_ClampExponent(n));
}

// Scale x by FLT_RADIX to a long n.
float scalblnf(float x, long int n)
{
    return scalbnf(x, _Math_ClampExponent(n));
}

// Scale x by FLT_RADIX to a long n.
long double scalblnl(long double x, long int n)
{
    return scalbnl(x, _Math_ClampExponent(n));
}

// Take the real cube root of x.
double cbrt(double x)
{
    return (double) _Math_TakeRoot(x, _MATH_ROOT_CUBE, _MATH_TYPE_DOUBLE);
}

// Take the real cube root of x.
float cbrtf(float x)
{
    return (float) _Math_TakeRoot(x, _MATH_ROOT_CUBE, _MATH_TYPE_FLOAT);
}

// Take the real cube root of x.
long double cbrtl(long double x)
{
    return _Math_TakeRoot(x, _MATH_ROOT_CUBE, _MATH_TYPE_LDOUBLE);
}

// Clear the sign of x.
double fabs(double x)
{
    ((unsigned char *) &x)[_MATH_DOUBLE_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    return x;
}

// Clear the sign of x.
float fabsf(float x)
{
    ((unsigned char *) &x)[_MATH_FLOAT_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    return x;
}

// Clear the sign of x.
long double fabsl(long double x)
{
    ((unsigned char *) &x)[_MATH_LDOUBLE_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    return x;
}

// Take the square root of the sum of the squares of x and y.
double hypot(double x, double y)
{
    return (double) _Math_TakeHypot(x, y, _MATH_TYPE_DOUBLE);
}

// Take the square root of the sum of the squares of x and y.
float hypotf(float x, float y)
{
    return (float) _Math_TakeHypot(x, y, _MATH_TYPE_FLOAT);
}

// Take the square root of the sum of the squares of x and y.
long double hypotl(long double x, long double y)
{
    return _Math_TakeHypot(x, y, _MATH_TYPE_LDOUBLE);
}

// Raise x to the power y.
double pow(double x, double y)
{
    return (double) _Math_TakePow(x, y, _MATH_TYPE_DOUBLE);
}

// Raise x to the power y.
float powf(float x, float y)
{
    return (float) _Math_TakePow(x, y, _MATH_TYPE_FLOAT);
}

// Raise x to the power y.
long double powl(long double x, long double y)
{
    return _Math_TakePow(x, y, _MATH_TYPE_LDOUBLE);
}

// Take the nonnegative square root of x.
double sqrt(double x)
{
    return (double) _Math_TakeRoot(x, _MATH_ROOT_SQUARE, _MATH_TYPE_DOUBLE);
}

// Take the nonnegative square root of x.
float sqrtf(float x)
{
    return (float) _Math_TakeRoot(x, _MATH_ROOT_SQUARE, _MATH_TYPE_FLOAT);
}

// Take the nonnegative square root of x.
long double sqrtl(long double x)
{
    return _Math_TakeRoot(x, _MATH_ROOT_SQUARE, _MATH_TYPE_LDOUBLE);
}

// Round x up to an integer.
double ceil(double x)
{
    return (double) ceill(x);
}

// Round x up to an integer.
float ceilf(float x)
{
    return (float) ceill(x);
}

// Round x up to an integer.
long double ceill(long double x)
{
    long double whole = truncl(x);

    return isfinite(x) && whole < x ? whole + 1 : whole;
}

// Round x down to an integer.
double floor(double x)
{
    return (double) floorl(x);
}

// Round x down to an integer.
float floorf(float x)
{
    return (float) floorl(x);
}

// Round x down to an integer.
long double floorl(long double x)
{
    long double whole = truncl(x);

    return isfinite(x) && whole > x ? whole - 1 : whole;
}

// Round x to an integer in the current direction without inexact.
double nearbyint(double x)
{
    return (double) nearbyintl(x);
}

// Round x to an integer in the current direction without inexact.
float nearbyintf(float x)
{
    return (float) nearbyintl(x);
}

// Round x to an integer in the current direction without inexact.
long double nearbyintl(long double x)
{
    fenv_t env;
    long double rounded;

    if (isnan(x)) {
        return x + x;
    }
    feholdexcept(&env);
    rounded = rintl(x);
    fesetenv(&env);
    return rounded;
}

// Round x to an integer in the current direction.
double rint(double x)
{
    return (double) rintl(x);
}

// Round x to an integer in the current direction.
float rintf(float x)
{
    return (float) rintl(x);
}

// Round x to an integer in the current direction.
long double rintl(long double x)
{
    long double shift = copysignl(_MATH_LDOUBLE_INTEGRAL, x);

    if (! isfinite(x)) {
        return x + x;
    }
    if (fabsl(x) >= _MATH_LDOUBLE_INTEGRAL) {
        return x;
    }
    return copysignl((x + shift) - shift, x);
}

// Round x to a long in the current direction.
long int lrint(double x)
{
    return lrintl(x);
}

// Round x to a long in the current direction.
long int lrintf(float x)
{
    return lrintl(x);
}

// Round x to a long in the current direction.
long int lrintl(long double x)
{
    return (long int) rintl(x);
}

// Round x to a long long in the current direction.
long long int llrint(double x)
{
    return llrintl(x);
}

// Round x to a long long in the current direction.
long long int llrintf(float x)
{
    return llrintl(x);
}

// Round x to a long long in the current direction.
long long int llrintl(long double x)
{
    return (long long int) rintl(x);
}

// Round x to the nearest integer, halfway away from zero.
double round(double x)
{
    return (double) roundl(x);
}

// Round x to the nearest integer, halfway away from zero.
float roundf(float x)
{
    return (float) roundl(x);
}

// Round x to the nearest integer, halfway away from zero.
long double roundl(long double x)
{
    long double whole = truncl(x);

    if (isfinite(x) && fabsl(x - whole) >= _MATH_HALF) {
        return whole + copysignl(1, x);
    }
    return whole;
}

// Round x to the nearest long, halfway away from zero.
long int lround(double x)
{
    return lroundl(x);
}

// Round x to the nearest long, halfway away from zero.
long int lroundf(float x)
{
    return lroundl(x);
}

// Round x to the nearest long, halfway away from zero.
long int lroundl(long double x)
{
    return (long int) roundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llround(double x)
{
    return llroundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llroundf(float x)
{
    return llroundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llroundl(long double x)
{
    return (long long int) roundl(x);
}

// Round x toward zero to an integer.
double trunc(double x)
{
    return (double) truncl(x);
}

// Round x toward zero to an integer.
float truncf(float x)
{
    return (float) truncl(x);
}

// Round x toward zero to an integer.
long double truncl(long double x)
{
    int exp;
    uint64_t sig;

    if (! isfinite(x) || x == 0) {
        return x + x;
    }
    sig = _Math_SplitLDouble(x, &exp);
    if (exp >= _MATH_LDOUBLE_INT_SHIFT) {
        return x;
    }
    if (exp < 0) {
        return copysignl(0, x);
    }
    return copysignl(_Math_JoinLDouble(exp, sig & UINT64_MAX << (_MATH_LDOUBLE_INT_SHIFT - exp)), x);
}

// Take the remainder of x by y toward zero.
double fmod(double x, double y)
{
    return (double) fmodl(x, y);
}

// Take the remainder of x by y toward zero.
float fmodf(float x, float y)
{
    return (float) fmodl(x, y);
}

// Take the remainder of x by y toward zero.
long double fmodl(long double x, long double y)
{
    uint32_t q;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (isinf(x) || y == 0) {
        return _Math_RaiseDomain(x, y);
    }
    if (isinf(y) || x == 0) {
        return x;
    }
    return _Math_TakeRemainder(x, y, _MATH_QUOTIENT_TRUNCATED, &q);
}

// Take IEC 60559's remainder of x by y.
double remainder(double x, double y)
{
    return (double) remainderl(x, y);
}

// Take IEC 60559's remainder of x by y.
float remainderf(float x, float y)
{
    return (float) remainderl(x, y);
}

// Take IEC 60559's remainder of x by y.
long double remainderl(long double x, long double y)
{
    int quo;

    return remquol(x, y, &quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
double remquo(double x, double y, int *quo)
{
    return (double) remquol(x, y, quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
float remquof(float x, float y, int *quo)
{
    return (float) remquol(x, y, quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
long double remquol(long double x, long double y, int *quo)
{
    uint32_t q;
    long double rest;

    *quo = 0;
    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (isinf(x) || y == 0) {
        return _Math_RaiseDomain(x, y);
    }
    if (isinf(y) || x == 0) {
        return x;
    }
    rest = _Math_TakeRemainder(x, y, _MATH_QUOTIENT_NEAREST, &q);
    q &= _MATH_QUOTIENT_MASK;
    *quo = signbit(x) == signbit(y) ? (int) q : -(int) q;
    return rest;
}

// Give x the sign of y.
double copysign(double x, double y)
{
    ((unsigned char *) &x)[_MATH_DOUBLE_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_MATH_DOUBLE_SIGN_BYTE] |= ((unsigned char *) &y)[_MATH_DOUBLE_SIGN_BYTE] & _MATH_SIGN_BIT;
    return x;
}

// Give x the sign of y.
float copysignf(float x, float y)
{
    ((unsigned char *) &x)[_MATH_FLOAT_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_MATH_FLOAT_SIGN_BYTE] |= ((unsigned char *) &y)[_MATH_FLOAT_SIGN_BYTE] & _MATH_SIGN_BIT;
    return x;
}

// Give x the sign of y.
long double copysignl(long double x, long double y)
{
    ((unsigned char *) &x)[_MATH_LDOUBLE_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_MATH_LDOUBLE_SIGN_BYTE] |= ((unsigned char *) &y)[_MATH_LDOUBLE_SIGN_BYTE] & _MATH_SIGN_BIT;
    return x;
}

// Make a quiet NaN of tagp's payload.
double nan(const char *tagp)
{
    double x;
    uint64_t bits = _MATH_DOUBLE_QUIET | (_Math_ReadPayload(tagp) & _MATH_DOUBLE_FRAC_MASK);

    memcpy(&x, &bits, sizeof(x));
    return x;
}

// Make a quiet NaN of tagp's payload.
float nanf(const char *tagp)
{
    float x;
    uint32_t bits = _MATH_FLOAT_QUIET | (uint32_t) (_Math_ReadPayload(tagp) & _MATH_FLOAT_FRAC_MASK);

    memcpy(&x, &bits, sizeof(x));
    return x;
}

// Make a quiet NaN of tagp's payload.
long double nanl(const char *tagp)
{
    return _Math_MakeLDouble(_MATH_LDOUBLE_EXP_MAX, _MATH_LDOUBLE_QUIET | (_Math_ReadPayload(tagp) & _MATH_LDOUBLE_FRAC_MASK));
}

// Step x to the next value toward y.
double nextafter(double x, double y)
{
    return nexttoward(x, y);
}

// Step x to the next value toward y.
float nextafterf(float x, float y)
{
    return nexttowardf(x, y);
}

// Step x to the next value toward y.
long double nextafterl(long double x, long double y)
{
    return nexttowardl(x, y);
}

// Step x to the next value toward a long double y.
double nexttoward(double x, long double y)
{
    double next;
    uint64_t bits;

    if (isnan(x) || isnan(y)) {
        return (double) (x + y);
    }
    if (x == y) {
        return (double) y;
    }
    if (x == 0) {
        next = y < 0 ? -_MATH_DOUBLE_TINY : _MATH_DOUBLE_TINY;
    } else {
        memcpy(&bits, &x, sizeof(bits));
        if ((x < y) == (x > 0)) {
            bits++;
        } else {
            bits--;
        }
        memcpy(&next, &bits, sizeof(next));
    }
    _Math_CheckNext(x, fpclassify(next));
    return next;
}

// Step x to the next value toward a long double y.
float nexttowardf(float x, long double y)
{
    float next;
    uint32_t bits;

    if (isnan(x) || isnan(y)) {
        return (float) (x + y);
    }
    if (x == y) {
        return (float) y;
    }
    if (x == 0) {
        next = y < 0 ? -_MATH_FLOAT_TINY : _MATH_FLOAT_TINY;
    } else {
        memcpy(&bits, &x, sizeof(bits));
        if ((x < y) == (x > 0)) {
            bits++;
        } else {
            bits--;
        }
        memcpy(&next, &bits, sizeof(next));
    }
    _Math_CheckNext(x, fpclassify(next));
    return next;
}

// Step x to the next value toward a long double y.
long double nexttowardl(long double x, long double y)
{
    long double next;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x == y) {
        return y;
    }
    if (x == 0) {
        next = y < 0 ? -_MATH_LDOUBLE_TINY : _MATH_LDOUBLE_TINY;
    } else {
        next = _Math_StepLDouble(x, (x < y) == (x > 0) ? _MATH_STEP_UP : _MATH_STEP_DOWN);
    }
    _Math_CheckNext(x, fpclassify(next));
    return next;
}

// Take the positive difference of x and y.
double fdim(double x, double y)
{
    double diff;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (isinf(diff) && isfinite(x) && isfinite(y)) {
        errno = ERANGE;
    }
    return diff;
}

// Take the positive difference of x and y.
float fdimf(float x, float y)
{
    float diff;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (isinf(diff) && isfinite(x) && isfinite(y)) {
        errno = ERANGE;
    }
    return diff;
}

// Take the positive difference of x and y.
long double fdiml(long double x, long double y)
{
    long double diff;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (isinf(diff) && isfinite(x) && isfinite(y)) {
        errno = ERANGE;
    }
    return diff;
}

// Take the greater of x and y.
double fmax(double x, double y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the greater of x and y.
float fmaxf(float x, float y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the greater of x and y.
long double fmaxl(long double x, long double y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the lesser of x and y.
double fmin(double x, double y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take the lesser of x and y.
float fminf(float x, float y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take the lesser of x and y.
long double fminl(long double x, long double y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take x times y plus z, rounded once.
double fma(double x, double y, double z)
{
    return (double) _Math_TakeFma(x, y, z, _MATH_TYPE_DOUBLE);
}

// Take x times y plus z, rounded once.
float fmaf(float x, float y, float z)
{
    return (float) _Math_TakeFma(x, y, z, _MATH_TYPE_FLOAT);
}

// Take x times y plus z, rounded once.
long double fmal(long double x, long double y, long double z)
{
    return _Math_TakeFma(x, y, z, _MATH_TYPE_LDOUBLE);
}
