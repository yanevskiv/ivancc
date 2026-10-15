/*
 * C source file for exact values and long double fields.
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
#include <_exact.h>

// Includes.
#include <_fp.h>
#include <_fpu.h>
#include <_str.h>

// The bits of 2/pi past its point, 32 to a limb, the most significant first.
const unsigned int _Exact_TwoOverPi[_EXACT_TWO_OVER_PI_LIMBS] = {
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

// Read a long double's significand.
unsigned long long _Exact_ReadSignificand(long double x)
{
    unsigned long long sig;

    _Str_CopyBytes(&sig, (unsigned char *) &x + _EXACT_LDOUBLE_SIGNIFICAND, sizeof(sig));
    return sig;
}

// Read a long double's sign and biased exponent.
unsigned int _Exact_ReadTop(long double x)
{
    unsigned short top;

    _Str_CopyBytes(&top, (unsigned char *) &x + _EXACT_LDOUBLE_TOP, sizeof(top));
    return top;
}

// Make a long double of its sign and biased exponent and its significand.
long double _Exact_MakeLDouble(unsigned int top, unsigned long long sig)
{
    long double x = 0.0L;
    unsigned long long low = sig;
    unsigned short high = (unsigned short) top;

    _Str_CopyBytes((unsigned char *) &x + _EXACT_LDOUBLE_SIGNIFICAND, &low, sizeof(low));
    _Str_CopyBytes((unsigned char *) &x + _EXACT_LDOUBLE_TOP, &high, sizeof(high));
    return x;
}

// Split a long double into its normalized significand and its exponent.
unsigned long long _Exact_SplitLDouble(long double x, int *exp)
{
    unsigned long long sig = _Exact_ReadSignificand(x);
    unsigned int biased = _Exact_ReadTop(x) & _EXACT_LDOUBLE_EXP_MAX;

    *exp = biased == 0 ? _EXACT_LDOUBLE_EXP_LOW : (int) biased - _EXACT_LDOUBLE_BIAS;
    for (; (sig & _EXACT_LDOUBLE_INT_BIT) == 0; sig <<= 1) {
        (*exp)--;
    }
    return sig;
}

// Join a significand and an exponent into a long double.
long double _Exact_JoinLDouble(int exp, unsigned long long sig)
{
    int shift;
    unsigned long long bits = sig;

    if (bits == 0) {
        return 0.0L;
    }
    for (; (bits & _EXACT_LDOUBLE_INT_BIT) == 0; bits <<= 1) {
        exp--;
    }
    if (exp >= _EXACT_LDOUBLE_EXP_LOW) {
        return _Exact_MakeLDouble((unsigned int) (exp + _EXACT_LDOUBLE_BIAS), bits);
    }
    shift = _EXACT_LDOUBLE_EXP_LOW - exp;
    return _Exact_MakeLDouble(0, shift > _EXACT_LDOUBLE_INT_SHIFT ? 0 : bits >> shift);
}

// Step a long double's magnitude to the next value up or down.
long double _Exact_StepLDouble(long double x, _Exact_Step step)
{
    unsigned int top = _Exact_ReadTop(x);
    unsigned long long sig = _Exact_ReadSignificand(x);

    if (step == _EXACT_STEP_UP && sig == _EXACT_ALL_BITS) {
        sig = _EXACT_LDOUBLE_INT_BIT;
        top++;
    } else if (step == _EXACT_STEP_UP) {
        sig++;
    } else if ((top & _EXACT_LDOUBLE_EXP_MAX) != 0 && sig == _EXACT_LDOUBLE_INT_BIT) {
        top--;
        sig = (top & _EXACT_LDOUBLE_EXP_MAX) == 0 ? _EXACT_LDOUBLE_FRAC_MASK : _EXACT_ALL_BITS;
    } else {
        sig--;
    }
    if ((top & _EXACT_LDOUBLE_EXP_MAX) == 0 && sig >= _EXACT_LDOUBLE_INT_BIT) {
        top++;
    }
    return _Exact_MakeLDouble(top, sig);
}

// Return whether x's sign is negative.
_Bool _Exact_IsNegative(long double x)
{
    return (_Exact_ReadTop(x) & _EXACT_TOP_SIGN) != 0;
}

// Return whether x is a NaN, the x87's refused encodings as glibc takes them.
_Bool _Exact_IsNan(long double x)
{
    unsigned int exp = _Exact_ReadTop(x) & _EXACT_LDOUBLE_EXP_MAX;
    unsigned long long sig = _Exact_ReadSignificand(x);

    return exp != 0 && ((sig & _EXACT_LDOUBLE_INT_BIT) == 0 || (exp == _EXACT_LDOUBLE_EXP_MAX && (sig & _EXACT_LDOUBLE_FRAC_MASK) != 0));
}

// Return whether x is an infinity.
_Bool _Exact_IsInf(long double x)
{
    return (_Exact_ReadTop(x) & _EXACT_LDOUBLE_EXP_MAX) == _EXACT_LDOUBLE_EXP_MAX && _Exact_ReadSignificand(x) == _EXACT_LDOUBLE_INT_BIT;
}

// Clear x's sign.
long double _Exact_ClearSign(long double x)
{
    return _Exact_MakeLDouble(_Exact_ReadTop(x) & ~_EXACT_TOP_SIGN, _Exact_ReadSignificand(x));
}

// Copy y's sign to x.
long double _Exact_CopySign(long double x, long double y)
{
    return _Exact_MakeLDouble((_Exact_ReadTop(x) & ~_EXACT_TOP_SIGN) | (_Exact_ReadTop(y) & _EXACT_TOP_SIGN), _Exact_ReadSignificand(x));
}

// Set w to a 64-bit value.
void _Exact_SetWide(_Exact_Wide *w, unsigned long long low)
{
    _Str_FillBytes(w, 0, sizeof(*w));
    w->ew_limb[0] = (unsigned int) low;
    w->ew_limb[1] = (unsigned int) (low >> _EXACT_LIMB_BITS);
}

// Read w's low 64 bits.
unsigned long long _Exact_ReadWideLow(const _Exact_Wide *w)
{
    return (unsigned long long) w->ew_limb[1] << _EXACT_LIMB_BITS | w->ew_limb[0];
}

// Read w's bit at a place.
unsigned int _Exact_ReadWideBit(const _Exact_Wide *w, int at)
{
    return w->ew_limb[at / _EXACT_LIMB_BITS] >> (at % _EXACT_LIMB_BITS) & _EXACT_LOW_BIT;
}

// Find the place of w's top set bit.
int _Exact_FindWideTop(const _Exact_Wide *w)
{
    for (int i = _EXACT_WIDE_LIMBS - 1; i >= 0; i--) {
        int bit = _EXACT_LIMB_BITS - 1;

        if (w->ew_limb[i] == 0) {
            continue;
        }
        while ((w->ew_limb[i] >> bit) == 0) {
            bit--;
        }
        return i * _EXACT_LIMB_BITS + bit;
    }
    return _EXACT_WIDE_NONE;
}

// Shift w left by n bits, no set bit passing its top.
void _Exact_ShiftWideLeft(_Exact_Wide *w, int n)
{
    int limbs = n / _EXACT_LIMB_BITS;
    int bits = n % _EXACT_LIMB_BITS;

    for (int i = _EXACT_WIDE_LIMBS - 1; i >= 0; i--) {
        unsigned int high = i - limbs >= 0 ? w->ew_limb[i - limbs] : 0;
        unsigned int low = i - limbs - 1 >= 0 ? w->ew_limb[i - limbs - 1] : 0;

        w->ew_limb[i] = bits == 0 ? high : (unsigned int) (high << bits | low >> (_EXACT_LIMB_BITS - bits));
    }
}

// Shift w right by n bits, returning whether a set bit fell off.
_Bool _Exact_ShiftWideRight(_Exact_Wide *w, int n)
{
    _Bool lost = 0;
    int limbs = n / _EXACT_LIMB_BITS;
    int bits = n % _EXACT_LIMB_BITS;

    if (n >= _EXACT_WIDE_BITS) {
        lost = _Exact_FindWideTop(w) != _EXACT_WIDE_NONE;
        _Str_FillBytes(w, 0, sizeof(*w));
        return lost;
    }
    for (int i = 0; i < limbs; i++) {
        lost = lost || w->ew_limb[i] != 0;
    }
    lost = lost || (w->ew_limb[limbs] & ((_EXACT_LOW_BIT << bits) - 1)) != 0;
    for (int i = 0; i < _EXACT_WIDE_LIMBS; i++) {
        unsigned int low = i + limbs < _EXACT_WIDE_LIMBS ? w->ew_limb[i + limbs] : 0;
        unsigned int high = i + limbs + 1 < _EXACT_WIDE_LIMBS ? w->ew_limb[i + limbs + 1] : 0;

        w->ew_limb[i] = bits == 0 ? low : (unsigned int) (low >> bits | high << (_EXACT_LIMB_BITS - bits));
    }
    return lost;
}

// Shift w left by n bits, or right by -n, returning whether a set bit fell off.
_Bool _Exact_AlignWide(_Exact_Wide *w, int n)
{
    if (n >= 0) {
        _Exact_ShiftWideLeft(w, n);
        return 0;
    }
    return _Exact_ShiftWideRight(w, -n);
}

// Compare a with b, negative, zero or positive.
int _Exact_CompareWides(const _Exact_Wide *a, const _Exact_Wide *b)
{
    for (int i = _EXACT_WIDE_LIMBS - 1; i >= 0; i--) {
        if (a->ew_limb[i] != b->ew_limb[i]) {
            return a->ew_limb[i] < b->ew_limb[i] ? -1 : 1;
        }
    }
    return 0;
}

// Add add to w.
void _Exact_AddWide(_Exact_Wide *w, const _Exact_Wide *add)
{
    unsigned long long carry = 0;

    for (int i = 0; i < _EXACT_WIDE_LIMBS; i++) {
        carry += (unsigned long long) w->ew_limb[i] + add->ew_limb[i];
        w->ew_limb[i] = (unsigned int) carry;
        carry >>= _EXACT_LIMB_BITS;
    }
}

// Subtract sub from w, sub not above it.
void _Exact_SubtractWide(_Exact_Wide *w, const _Exact_Wide *sub)
{
    unsigned long long borrow = 0;

    for (int i = 0; i < _EXACT_WIDE_LIMBS; i++) {
        unsigned long long diff = (unsigned long long) w->ew_limb[i] - sub->ew_limb[i] - borrow;

        w->ew_limb[i] = (unsigned int) diff;
        borrow = diff >> _EXACT_LIMB_BITS & _EXACT_LOW_BIT;
    }
}

// Multiply a by b into to, the product below w's top.
void _Exact_MultiplyWides(_Exact_Wide *to, const _Exact_Wide *a, const _Exact_Wide *b)
{
    _Exact_Wide product;

    _Str_FillBytes(&product, 0, sizeof(product));
    for (int i = 0; i < _EXACT_WIDE_LIMBS; i++) {
        unsigned long long carry = 0;

        for (int j = 0; i + j < _EXACT_WIDE_LIMBS; j++) {
            carry += (unsigned long long) a->ew_limb[i] * b->ew_limb[j] + product.ew_limb[i + j];
            product.ew_limb[i + j] = (unsigned int) carry;
            carry >>= _EXACT_LIMB_BITS;
        }
    }
    *to = product;
}

// Raise base to the kth power into to, by squaring.
void _Exact_RaiseWide(_Exact_Wide *to, const _Exact_Wide *base, int k)
{
    _Exact_Wide square = *base;

    _Exact_SetWide(to, _EXACT_LOW_BIT);
    for (; k > 0; k >>= 1) {
        if ((k & 1) != 0) {
            _Exact_MultiplyWides(to, to, &square);
        }
        if (k > 1) {
            _Exact_MultiplyWides(&square, &square, &square);
        }
    }
}

// Divide w by a nonzero d, returning the remainder.
unsigned int _Exact_DivideWideSmall(_Exact_Wide *w, unsigned int d)
{
    unsigned long long rest = 0;

    for (int i = _EXACT_WIDE_LIMBS - 1; i >= 0; i--) {
        rest = rest << _EXACT_LIMB_BITS | w->ew_limb[i];
        w->ew_limb[i] = (unsigned int) (rest / d);
        rest %= d;
    }
    return (unsigned int) rest;
}

// Divide a by a nonzero b into to, the quotient truncated.
void _Exact_DivideWides(_Exact_Wide *to, const _Exact_Wide *a, const _Exact_Wide *b)
{
    _Exact_Wide rest;
    _Exact_Wide quotient;

    _Exact_SetWide(&rest, 0);
    _Exact_SetWide(&quotient, 0);
    for (int i = _Exact_FindWideTop(a); i >= 0; i--) {
        _Exact_ShiftWideLeft(&rest, 1);
        rest.ew_limb[0] |= _Exact_ReadWideBit(a, i);
        if (_Exact_CompareWides(&rest, b) >= 0) {
            _Exact_SubtractWide(&rest, b);
            quotient.ew_limb[i / _EXACT_LIMB_BITS] |= _EXACT_LOW_BIT << (i % _EXACT_LIMB_BITS);
        }
    }
    *to = quotient;
}

// Make the exact value of a finite nonzero long double.
void _Exact_MakeValue(_Exact_Value *exact, long double x)
{
    int exp;

    _Exact_SetWide(&exact->ev_sig, _Exact_SplitLDouble(x, &exp));
    exact->ev_unit = exp - _EXACT_LDOUBLE_INT_SHIFT;
    exact->ev_negative = _Exact_IsNegative(x) != 0;
    exact->ev_sticky = 0;
}

// Multiply exact values a and b, neither sticky, into product.
void _Exact_MultiplyValues(_Exact_Value *product, const _Exact_Value *a, const _Exact_Value *b)
{
    int unit = a->ev_unit + b->ev_unit;
    _Bool negative = a->ev_negative != b->ev_negative;

    _Exact_MultiplyWides(&product->ev_sig, &a->ev_sig, &b->ev_sig);
    product->ev_unit = unit;
    product->ev_negative = negative;
    product->ev_sticky = 0;
}

// Add exact values a and b, neither sticky, into sum, below a window sticky.
void _Exact_AddValues(_Exact_Value *sum, const _Exact_Value *a, const _Exact_Value *b)
{
    int order;
    _Bool lost;
    _Exact_Wide one;
    _Exact_Wide wa = a->ev_sig;
    _Exact_Wide wb = b->ev_sig;
    int top_a = a->ev_unit + _Exact_FindWideTop(&wa);
    int top_b = b->ev_unit + _Exact_FindWideTop(&wb);
    int base = (top_a > top_b ? top_a : top_b) + 1 - _EXACT_SUM_WINDOW;

    lost = _Exact_AlignWide(&wa, a->ev_unit - base);
    lost = _Exact_AlignWide(&wb, b->ev_unit - base) || lost;
    order = _Exact_CompareWides(&wa, &wb);
    sum->ev_unit = base;
    sum->ev_sticky = lost;
    if (a->ev_negative == b->ev_negative) {
        sum->ev_sig = wa;
        _Exact_AddWide(&sum->ev_sig, &wb);
        sum->ev_negative = a->ev_negative;
        return;
    }
    sum->ev_sig = order >= 0 ? wa : wb;
    _Exact_SubtractWide(&sum->ev_sig, order >= 0 ? &wb : &wa);
    sum->ev_negative = order >= 0 ? a->ev_negative : b->ev_negative;
    if (lost) {
        _Exact_SetWide(&one, _EXACT_LOW_BIT);
        _Exact_SubtractWide(&sum->ev_sig, &one);
    }
    if (order == 0) {
        sum->ev_negative = _Fpu_ReadRound() == _FPU_DOWNWARD;
    }
}

// Take the degree's root of an exact radicand into root, a round bit spare.
void _Exact_ExtractRoot(_Exact_Value *root, const _Exact_Value *radicand, _Exact_Root degree)
{
    _Bool lost;
    _Exact_Wide rest;
    _Exact_Wide next;
    _Exact_Wide trial;
    _Exact_Wide power;
    int k = (int) degree;
    _Exact_Wide n = radicand->ev_sig;
    int shift = k * _EXACT_ROOT_BITS - 1 - _Exact_FindWideTop(&n);

    while ((radicand->ev_unit - shift) % k != 0) {
        shift--;
    }
    lost = _Exact_AlignWide(&n, shift);
    _Exact_SetWide(&rest, 0);
    _Exact_SetWide(&root->ev_sig, 0);
    for (int i = _EXACT_ROOT_BITS - 1; i >= 0; i--) {
        _Exact_ShiftWideLeft(&rest, k);
        for (int j = 0; j < k; j++) {
            rest.ew_limb[0] |= _Exact_ReadWideBit(&n, k * i + j) << j;
        }
        _Exact_ShiftWideLeft(&root->ev_sig, 1);
        next = root->ev_sig;
        next.ew_limb[0] |= _EXACT_LOW_BIT;
        _Exact_RaiseWide(&trial, &next, k);
        _Exact_RaiseWide(&power, &root->ev_sig, k);
        _Exact_SubtractWide(&trial, &power);
        if (_Exact_CompareWides(&rest, &trial) >= 0) {
            _Exact_SubtractWide(&rest, &trial);
            root->ev_sig.ew_limb[0] |= _EXACT_LOW_BIT;
        }
    }
    root->ev_unit = (radicand->ev_unit - shift) / k;
    root->ev_negative = radicand->ev_negative;
    root->ev_sticky = lost || radicand->ev_sticky || _Exact_FindWideTop(&rest) != _EXACT_WIDE_NONE;
}

// Round exact's significand to its top keep bits in the current direction.
unsigned long long _Exact_RoundBits(const _Exact_Value *exact, int keep, int *low, _Bool *inexact)
{
    _Bool up;
    unsigned long long kept;
    _Bool half = 0;
    int mode = _Fpu_ReadRound();
    _Exact_Wide rest = exact->ev_sig;
    _Bool below = exact->ev_sticky;

    *low = _Exact_FindWideTop(&rest) - keep + 1;
    if (*low <= 0) {
        _Exact_ShiftWideLeft(&rest, -*low);
    } else {
        below = _Exact_ShiftWideRight(&rest, *low - 1) || below;
        half = (rest.ew_limb[0] & _EXACT_LOW_BIT) != 0;
        _Exact_ShiftWideRight(&rest, 1);
    }
    kept = _Exact_ReadWideLow(&rest);
    *inexact = half || below;
    if (mode == _FPU_TONEAREST) {
        up = half && (below || (kept & _EXACT_LOW_BIT) != 0);
    } else if (mode == _FPU_UPWARD || mode == _FPU_DOWNWARD) {
        up = *inexact && exact->ev_negative == (mode == _FPU_DOWNWARD);
    } else {
        up = 0;
    }
    if (! up) {
        return kept;
    }
    kept++;
    if (keep > 0 && (keep == _FP_LDOUBLE_MANT ? kept == 0 : kept >> keep != 0)) {
        kept = _EXACT_LDOUBLE_INT_BIT >> (_FP_LDOUBLE_MANT - keep);
        (*low)++;
    }
    return kept;
}

// Round an exact value once to type in the current direction, with its flags.
long double _Exact_RoundValue(const _Exact_Value *exact, _Exact_Type type)
{
    int low;
    int min;
    int max;
    int keep;
    _Bool tiny;
    int digits;
    _Bool inexact;
    unsigned long long kept;
    _Bool infinite;
    long double value;
    int mode = _Fpu_ReadRound();
    int top = _Exact_FindWideTop(&exact->ev_sig);
    int exp = exact->ev_unit + top;

    switch (type) {
        case _EXACT_TYPE_FLOAT: {
            digits = _FP_FLOAT_MANT;
            min = _FP_FLOAT_MIN;
            max = _FP_FLOAT_MAX;
        } break;
        case _EXACT_TYPE_DOUBLE: {
            digits = _FP_DOUBLE_MANT;
            min = _FP_DOUBLE_MIN;
            max = _FP_DOUBLE_MAX;
        } break;
        default: {
            digits = _FP_LDOUBLE_MANT;
            min = _FP_LDOUBLE_MIN;
            max = _FP_LDOUBLE_MAX;
        } break;
    }
    if (top == _EXACT_WIDE_NONE) {
        return exact->ev_negative ? -0.0L : 0.0L;
    }
    tiny = exp < min;
    if (exp == min - 1) {
        _Exact_RoundBits(exact, digits, &low, &inexact);
        tiny = low == top - digits + 1;
    }
    keep = exp < min ? digits - (min - exp) : digits;
    kept = _Exact_RoundBits(exact, keep, &low, &inexact);
    if (exp >= min && exact->ev_unit + low + keep - 1 > max) {
        infinite = mode == _FPU_TONEAREST || (mode == _FPU_UPWARD && ! exact->ev_negative) || (mode == _FPU_DOWNWARD && exact->ev_negative);
        _Fpu_RaiseFlags(_FPU_OVERFLOW | _FPU_INEXACT);
        value = infinite ? _EXACT_INFINITY : _Exact_JoinLDouble(max, _EXACT_ALL_BITS << (_FP_LDOUBLE_MANT - digits));
        return exact->ev_negative ? -value : value;
    }
    if (inexact) {
        _Fpu_RaiseFlags(tiny ? _FPU_UNDERFLOW | _FPU_INEXACT : _FPU_INEXACT);
    }
    value = _Exact_JoinLDouble(exact->ev_unit + low + _EXACT_LDOUBLE_INT_SHIFT, kept);
    return exact->ev_negative ? -value : value;
}

// Reduce an x of 1 or more by pi/2 to f, returning the quotient's low bits.
unsigned int _Exact_ReduceQuarters(_Exact_Value *f, long double x)
{
    int exp;
    int point;
    int first = 0;
    _Exact_Wide one;
    _Exact_Wide part;
    _Exact_Wide chunk;
    unsigned int quadrant = 0;
    unsigned long long sig = _Exact_SplitLDouble(x, &exp);
    int unit = exp - _EXACT_LDOUBLE_INT_SHIFT;

    if (unit > _EXACT_QUADRANT_BITS) {
        first = (unit - _EXACT_QUADRANT_BITS) / _EXACT_LIMB_BITS;
    }
    _Exact_SetWide(&chunk, 0);
    for (int i = 0; i < _EXACT_TRIG_WINDOW; i++) {
        chunk.ew_limb[i] = _Exact_TwoOverPi[first + _EXACT_TRIG_WINDOW - 1 - i];
    }
    _Exact_SetWide(&part, sig);
    _Exact_MultiplyWides(&f->ev_sig, &part, &chunk);
    point = _EXACT_LIMB_BITS * (first + _EXACT_TRIG_WINDOW) - unit;
    for (int i = _EXACT_QUADRANT_BITS - 1; i >= 0; i--) {
        if (point + i < _EXACT_WIDE_BITS) {
            quadrant = quadrant << 1 | _Exact_ReadWideBit(&f->ev_sig, point + i);
        }
    }
    _Exact_ShiftWideLeft(&f->ev_sig, _EXACT_WIDE_BITS - point);
    _Exact_ShiftWideRight(&f->ev_sig, _EXACT_WIDE_BITS - point);
    f->ev_unit = -point;
    f->ev_negative = 0;
    f->ev_sticky = 0;
    if (_Exact_ReadWideBit(&f->ev_sig, point - 1) != 0) {
        _Exact_SetWide(&one, _EXACT_LOW_BIT);
        _Exact_ShiftWideLeft(&one, point);
        _Exact_SubtractWide(&one, &f->ev_sig);
        f->ev_sig = one;
        f->ev_negative = 1;
        quadrant++;
    }
    return quadrant & _EXACT_QUADRANT_MASK;
}
