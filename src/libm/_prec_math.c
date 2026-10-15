/*
 * C source file for the precise mathematical functions.
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
#include <_prec_math.h>

// Includes.
#include <_err.h>
#include <_exact.h>
#include <_fp.h>
#include <_fpu.h>
#include <_str.h>
#include <_sys.h>

// The bits of 2/pi past its point, 32 to a limb, the most significant first.
const unsigned int _Prec_Math_TwoOverPi[_PREC_MATH_TWO_OVER_PI_LIMBS] = {
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

// The magnitudes of Stirling's coefficients as limbs of constants.
const unsigned int _Prec_Math_Stirling[_PREC_MATH_STIRLING_TERMS][_PREC_MATH_CONST_LIMBS] = {
    {0xAAAAAAAAU, 0xAAAAAAAAU, 0xAAAAAAAAU, 0xAAAAAAAAU, 0xAAAAAAAAU, 0xAAAAAAAAU},
    {0x60B60B60U, 0x0B60B60BU, 0xB60B60B6U, 0x60B60B60U, 0x0B60B60BU, 0xB60B60B6U},
    {0x00D00D00U, 0x0D00D00DU, 0xD00D00D0U, 0x00D00D00U, 0x0D00D00DU, 0xD00D00D0U},
    {0xC09C09C0U, 0x09C09C09U, 0x9C09C09CU, 0xC09C09C0U, 0x09C09C09U, 0x9C09C09CU},
    {0x4E700DCAU, 0xFE46AE1DU, 0x2A3C5631U, 0x7539C037U, 0xC7F91AB8U, 0xDCA8F158U},
    {0xE3E40FB5U, 0x5586CCC9U, 0x9E3E40FBU, 0xB5586CCCU, 0xC9E3E40FU, 0xFB5586CCU},
    {0x20D20D20U, 0x0D20D20DU, 0xD20D20D2U, 0x20D20D20U, 0x0D20D20DU, 0xD20D20D2U},
    {0x9CBEE103U, 0x1436587AU, 0x8BADCFF2U, 0x03254769U, 0x7A9CBEE1U, 0xF2143658U},
    {0xB1C0F033U, 0xD0C3B7F4U, 0xC0F033FFU, 0xC3B7F4B1U, 0xF033FFD0U, 0xB7F4B1C0U},
    {0x4AA70223U, 0x48B14C1FU, 0xCC3EA607U, 0xEDCE7312U, 0xC0F9CF6DU, 0xB23B3808U},
    {0x167002D3U, 0x9EA72219U, 0x59C00B4EU, 0x7A9C8864U, 0x67002D3AU, 0xD6722191U},
    {0xE6660D55U, 0xB44A9292U, 0xE07CA39DU, 0x3F712EB9U, 0x6660D55BU, 0x9CD9292EU},
    {0x740DA740U, 0x0DA740DAU, 0xA740DA74U, 0x40DA740DU, 0xDA740DA7U, 0x8911A740U},
    {0xD250842CU, 0x94D92E2FU, 0xB48FF1B3U, 0xFF6EEC24U, 0xE255BF59U, 0x8D0CC570U},
    {0x234D582BU, 0xDCB00698U, 0x4469CCAEU, 0x19EE4FDCU, 0x3708D1C2U, 0xA8D1044DU},
    {0x9ABBC406U, 0xBBC40616U, 0xC406169AU, 0x06169ABBU, 0x169ABBC4U, 0xE8844D8AU},
    {0x9DBCC486U, 0x676F3121U, 0x19DBCC48U, 0x8676F312U, 0x219DBCC4U, 0xB694D07BU},
    {0x3F07A1F8U, 0xDA88C079U, 0x49761634U, 0xA6024D5CU, 0xF23376AEU, 0xA2288CECU},
    {0x7350B881U, 0x350B8812U, 0x50B88127U, 0x0B881273U, 0xEA012735U, 0xA1BBCDE4U},
    {0xAF642A24U, 0x43581714U, 0x4AF642A2U, 0x24358171U, 0x03D4642AU, 0xB4005BDEU},
    {0x8573ED1EU, 0xEEECAC9EU, 0xEAE46D98U, 0xC3A9E6DAU, 0x78FBAAE3U, 0xDE466B7CU},
    {0x151661B4U, 0x72037C5EU, 0xD884644BU, 0x40509F4FU, 0x77729BCBU, 0x977D7628U},
    {0xD3226A6FU, 0x83DA6597U, 0x2D4F335CU, 0x0B6B0A35U, 0x5AF0BED9U, 0xE2E1337FU},
    {0xAD457156U, 0x09D8D881U, 0x86CDEEA2U, 0xB4F92FF9U, 0x8AD89016U, 0xB9E09405U},
    {0xB08640E9U, 0x478F4715U, 0xC8BF3F7CU, 0x0326AB4CU, 0xE71AC7C8U, 0xA5F7EEF9U},
    {0x8CB9CEBDU, 0x694DEEF0U, 0x6C86491AU, 0xDA64925CU, 0x7954084CU, 0xA0EF80E5U},
    {0xF4826900U, 0xC758797BU, 0x0CE0256EU, 0x9790760BU, 0xDA17DD99U, 0xA8EBFE48U},
    {0x128B1478U, 0x3BA1B5EFU, 0x6CBEC2EFU, 0x24C95AB5U, 0x3556FB17U, 0xBF582A43U},
    {0xE86D69D5U, 0x45C0EB78U, 0x04316DCBU, 0x7770E671U, 0x3E6C7D0EU, 0xE940B372U},
    {0xBD60A7D4U, 0x174B18C9U, 0x451835E1U, 0xF8CC3B4FU, 0x89672663U, 0x989A1506U}
};

// The units of Stirling's coefficients.
const int _Prec_Math_StirlingUnits[_PREC_MATH_STIRLING_TERMS] = {
    -195, -200, -202, -202, -202, -201, -199, -197, -194, -191, -188, -184, -180, -176, -172,
    -168, -163, -158, -153, -148, -143, -137, -132, -126, -120, -114, -108, -102, -96, -89
};

// Keep a working value's top bits, the rest truncated and sticky.
void _Prec_Math_TrimExact(_Exact_Value *e)
{
    int drop = _Exact_FindWideTop(&e->ev_sig) + 1 - _PREC_MATH_WORK_BITS;

    if (drop > 0) {
        e->ev_sticky = _Exact_ShiftWideRight(&e->ev_sig, drop) || e->ev_sticky;
        e->ev_unit += drop;
    }
}

// Multiply working values a and b into product, trimmed.
void _Prec_Math_WorkMultiply(_Exact_Value *product, const _Exact_Value *a, const _Exact_Value *b)
{
    _Exact_MultiplyValues(product, a, b);
    _Prec_Math_TrimExact(product);
}

// Add nonzero working values a and b into sum, trimmed.
void _Prec_Math_WorkAdd(_Exact_Value *sum, const _Exact_Value *a, const _Exact_Value *b)
{
    _Exact_AddValues(sum, a, b);
    _Prec_Math_TrimExact(sum);
}

// Divide working value a by a nonzero b, a sticky b a unit up, trimmed.
void _Prec_Math_WorkDivide(_Exact_Value *quotient, const _Exact_Value *a, const _Exact_Value *b)
{
    int shift;
    _Exact_Wide one;
    _Exact_Value top = *a;
    _Exact_Value bottom = *b;
    _Bool negative = a->ev_negative != b->ev_negative;

    _Prec_Math_TrimExact(&top);
    _Prec_Math_TrimExact(&bottom);
    if (bottom.ev_sticky) {
        _Exact_SetWide(&one, _EXACT_LOW_BIT);
        _Exact_AddWide(&bottom.ev_sig, &one);
    }
    shift = 2 * _PREC_MATH_WORK_BITS - 1 - _Exact_FindWideTop(&top.ev_sig);
    _Exact_ShiftWideLeft(&top.ev_sig, shift);
    _Exact_DivideWides(&quotient->ev_sig, &top.ev_sig, &bottom.ev_sig);
    quotient->ev_unit = top.ev_unit - shift - bottom.ev_unit;
    quotient->ev_negative = negative;
    quotient->ev_sticky = 0;
    _Prec_Math_TrimExact(quotient);
}

// Divide working value e by a nonzero d, trimmed.
void _Prec_Math_WorkDivideSmall(_Exact_Value *e, unsigned int d)
{
    _Exact_ShiftWideLeft(&e->ev_sig, _EXACT_LIMB_BITS);
    e->ev_unit -= _EXACT_LIMB_BITS;
    _Exact_DivideWideSmall(&e->ev_sig, d);
    _Prec_Math_TrimExact(e);
}

// Make the reciprocal of a nonzero d as a working value.
void _Prec_Math_MakeReciprocal(_Exact_Value *e, unsigned int d)
{
    _Exact_SetWide(&e->ev_sig, _EXACT_LOW_BIT);
    _Exact_ShiftWideLeft(&e->ev_sig, _PREC_MATH_WORK_BITS);
    e->ev_unit = -_PREC_MATH_WORK_BITS;
    e->ev_negative = 0;
    e->ev_sticky = 0;
    _Prec_Math_WorkDivideSmall(e, d);
}

// Make a constant as a value, untrimmed.
void _Prec_Math_MakeConstant(_Exact_Value *e, _Prec_Math_Constant which)
{
    const unsigned int limbs[_PREC_MATH_CONSTANTS][_PREC_MATH_CONST_LIMBS] = {{_PREC_MATH_LN2_LIMBS}, {_PREC_MATH_LOG2E_LIMBS}, {_PREC_MATH_LOG10E_LIMBS}, {_PREC_MATH_PI_LIMBS}, {_PREC_MATH_INV_ROOT_PI_LIMBS}, {_PREC_MATH_LN_ROOT_2PI_LIMBS}};
    const int units[_PREC_MATH_CONSTANTS] = {_PREC_MATH_LN2_UNIT, _PREC_MATH_LOG2E_UNIT, _PREC_MATH_LOG10E_UNIT, _PREC_MATH_PI_UNIT, _PREC_MATH_INV_ROOT_PI_UNIT, _PREC_MATH_LN_ROOT_2PI_UNIT};

    _Exact_SetWide(&e->ev_sig, 0);
    _Str_CopyBytes(e->ev_sig.ew_limb, limbs[which], sizeof(limbs[which]));
    e->ev_unit = units[which];
    e->ev_negative = 0;
    e->ev_sticky = 0;
}

// Read the integer nearest a working value, ties away from zero.
long _Prec_Math_ReadNearest(const _Exact_Value *e)
{
    long n;
    _Exact_Wide w = e->ev_sig;

    _Exact_AlignWide(&w, e->ev_unit + 1);
    n = (long) ((_Exact_ReadWideLow(&w) + 1) >> 1);
    return e->ev_negative ? -n : n;
}

// Reduce t to r, t less n times ln 2, returning n.
long _Prec_Math_ReduceExp(_Exact_Value *r, const _Exact_Value *t)
{
    long n;
    _Exact_Value step;
    _Exact_Value constant;

    _Prec_Math_MakeConstant(&constant, _PREC_MATH_CONST_LOG2E);
    _Prec_Math_WorkMultiply(&step, t, &constant);
    n = _Prec_Math_ReadNearest(&step);
    if (n == 0) {
        *r = *t;
        return 0;
    }
    _Exact_MakeValue(&step, (long double) -n);
    _Prec_Math_MakeConstant(&constant, _PREC_MATH_CONST_LN2);
    _Exact_MultiplyValues(&step, &step, &constant);
    _Prec_Math_WorkAdd(r, t, &step);
    return n;
}

// Take e to r less 1, r reduced, by halvings, a series and doublings.
void _Prec_Math_ExpM1Exact(_Exact_Value *em1, const _Exact_Value *r)
{
    _Exact_Value sum;
    _Exact_Value one;
    _Exact_Value two;
    _Exact_Value series;
    _Exact_Value y = *r;

    y.ev_unit -= _PREC_MATH_EXP_HALVINGS;
    _Exact_MakeValue(&one, 1.0L);
    _Exact_MakeValue(&two, 2.0L);
    series = one;
    for (unsigned int k = _PREC_MATH_EXP_TERMS; k >= 2; k--) {
        _Prec_Math_WorkMultiply(&series, &y, &series);
        _Prec_Math_WorkDivideSmall(&series, k);
        _Prec_Math_WorkAdd(&series, &one, &series);
    }
    _Prec_Math_WorkMultiply(em1, &y, &series);
    for (int i = 0; i < _PREC_MATH_EXP_HALVINGS; i++) {
        _Prec_Math_WorkAdd(&sum, em1, &two);
        _Prec_Math_WorkMultiply(em1, em1, &sum);
    }
}

// Take e to a nonzero working value t, clamped past every range.
void _Prec_Math_ExpExact(_Exact_Value *value, const _Exact_Value *t)
{
    long n;
    _Exact_Value r;
    _Exact_Value one;
    _Exact_Value clamped = *t;

    if (t->ev_unit + _Exact_FindWideTop(&t->ev_sig) > _PREC_MATH_EXP_CLAMP_EXP) {
        _Exact_MakeValue(&clamped, t->ev_negative ? -_PREC_MATH_EXP_CLAMP : _PREC_MATH_EXP_CLAMP);
    }
    n = _Prec_Math_ReduceExp(&r, &clamped);
    _Prec_Math_ExpM1Exact(value, &r);
    _Exact_MakeValue(&one, 1.0L);
    _Exact_AddValues(value, &one, value);
    value->ev_unit += (int) n;
}

// Take e to a nonzero working value t less 1, clamped past every range.
void _Prec_Math_ExpM1Clamped(_Exact_Value *em1, const _Exact_Value *t)
{
    long n;
    _Exact_Value r;
    _Exact_Value one;
    _Exact_Value clamped = *t;

    if (t->ev_unit + _Exact_FindWideTop(&t->ev_sig) > _PREC_MATH_EXP_CLAMP_EXP) {
        _Exact_MakeValue(&clamped, t->ev_negative ? -_PREC_MATH_EXP_CLAMP : _PREC_MATH_EXP_CLAMP);
    }
    n = _Prec_Math_ReduceExp(&r, &clamped);
    _Prec_Math_ExpM1Exact(em1, &r);
    if (n != 0) {
        _Exact_MakeValue(&one, 1.0L);
        _Exact_AddValues(em1, &one, em1);
        em1->ev_unit += (int) n;
        one.ev_negative = 1;
        _Exact_AddValues(em1, em1, &one);
    }
    _Prec_Math_TrimExact(em1);
}

// Split u into m - 1 and m + 1, u being m times 2 to the returned e.
int _Prec_Math_SplitLog(_Exact_Value *num, _Exact_Value *den, const _Exact_Value *u)
{
    _Exact_Value m = *u;
    _Exact_Value one;
    int top = _Exact_FindWideTop(&m.ev_sig);
    int e = m.ev_unit + top;

    if (_Exact_ReadWideBit(&m.ev_sig, top - 1) != 0) {
        e++;
    }
    m.ev_unit -= e;
    _Prec_Math_TrimExact(&m);
    _Exact_MakeValue(&one, 1.0L);
    _Prec_Math_WorkAdd(den, &m, &one);
    one.ev_negative = 1;
    _Prec_Math_WorkAdd(num, &m, &one);
    return e;
}

// Take e times ln 2 plus 2 atanh of num over den, num over den small.
void _Prec_Math_LogExact(_Exact_Value *log, const _Exact_Value *num, const _Exact_Value *den, int e)
{
    _Exact_Value s;
    _Exact_Value z;
    _Exact_Value part;
    _Exact_Value series;
    _Exact_Value constant;

    if (e != 0) {
        _Exact_MakeValue(&part, (long double) e);
        _Prec_Math_MakeConstant(&constant, _PREC_MATH_CONST_LN2);
        _Prec_Math_WorkMultiply(&part, &part, &constant);
    }
    if (_Exact_FindWideTop(&num->ev_sig) == _EXACT_WIDE_NONE) {
        *log = part;
        return;
    }
    _Prec_Math_WorkDivide(&s, num, den);
    _Prec_Math_WorkMultiply(&z, &s, &s);
    _Prec_Math_MakeReciprocal(&series, 2 * _PREC_MATH_LOG_TERMS - 1);
    for (int k = _PREC_MATH_LOG_TERMS - 2; k >= 0; k--) {
        _Prec_Math_WorkMultiply(&series, &z, &series);
        _Prec_Math_MakeReciprocal(&constant, (unsigned int) (2 * k + 1));
        _Prec_Math_WorkAdd(&series, &constant, &series);
    }
    _Prec_Math_WorkMultiply(log, &s, &series);
    log->ev_unit++;
    if (e != 0) {
        _Prec_Math_WorkAdd(log, &part, log);
    }
}

// Take the natural logarithm of 1 plus a nonzero working value u above -1.
void _Prec_Math_Log1pExact(_Exact_Value *log, const _Exact_Value *u)
{
    int e = 0;
    _Exact_Value w;
    _Exact_Value one;
    _Exact_Value num;
    _Exact_Value den;
    int top = u->ev_unit + _Exact_FindWideTop(&u->ev_sig);

    _Exact_MakeValue(&one, 1.0L);
    if (top <= (u->ev_negative ? _PREC_MATH_LOG1P_LOW_EXP : _PREC_MATH_LOG1P_HIGH_EXP)) {
        num = *u;
        _Prec_Math_WorkAdd(&den, &one, &one);
        _Prec_Math_WorkAdd(&den, &den, &num);
    } else {
        _Exact_AddValues(&w, &one, u);
        e = _Prec_Math_SplitLog(&num, &den, &w);
    }
    _Prec_Math_LogExact(log, &num, &den, e);
}

// Find whether x is 10 to the power k, k positive.
_Bool _Prec_Math_IsPowerTen(long double x, long k)
{
    int exp;
    _Exact_Wide ten;
    _Exact_Wide value;
    _Exact_Wide power;

    if (k < 1 || k > _PREC_MATH_LDOUBLE_TENS) {
        return 0;
    }
    _Exact_SetWide(&value, _Exact_SplitLDouble(x, &exp));
    if (_Exact_AlignWide(&value, exp - _EXACT_LDOUBLE_INT_SHIFT)) {
        return 0;
    }
    _Exact_SetWide(&ten, _PREC_MATH_TEN);
    _Exact_RaiseWide(&power, &ten, (int) k);
    return _Exact_CompareWides(&power, &value) == 0;
}

// Read whether a finite nonzero y is an integer, and which.
_Prec_Math_Parity _Prec_Math_ReadParity(long double y)
{
    int exp;
    unsigned long long sig = _Exact_SplitLDouble(y, &exp);
    int point = _EXACT_LDOUBLE_INT_SHIFT - exp;

    if (point < 0) {
        return _PREC_MATH_PARITY_EVEN;
    }
    if (point > _EXACT_LDOUBLE_INT_SHIFT || (sig & ~(_EXACT_ALL_BITS << point)) != 0) {
        return _PREC_MATH_PARITY_NONE;
    }
    return (sig >> point & _EXACT_LOW_BIT) != 0 ? _PREC_MATH_PARITY_ODD : _PREC_MATH_PARITY_EVEN;
}

// Make a positive x to a nonzero y, both finite, when it is exact.
_Bool _Prec_Math_PowerExact(_Exact_Value *value, long double x, long double y)
{
    int exp;
    _Exact_Wide odd;
    _Exact_Wide rest;
    _Exact_Value power;
    unsigned long long sig = _Exact_SplitLDouble(x, &exp);
    int unit = exp - _EXACT_LDOUBLE_INT_SHIFT;

    for (; (sig & _EXACT_LOW_BIT) == 0; sig >>= 1) {
        unit++;
    }
    _Exact_SetWide(&odd, sig);
    value->ev_negative = 0;
    value->ev_sticky = 0;
    if (sig == _EXACT_LOW_BIT && unit == 0) {
        _Exact_SetWide(&value->ev_sig, _EXACT_LOW_BIT);
        value->ev_unit = 0;
        return 1;
    }
    if (sig == _EXACT_LOW_BIT) {
        _Exact_MakeValue(&power, (long double) unit);
        _Exact_MakeValue(value, y);
        _Exact_MultiplyValues(&power, &power, value);
        rest = power.ev_sig;
        if (_Exact_AlignWide(&rest, power.ev_unit)) {
            return 0;
        }
        _Exact_SetWide(&value->ev_sig, _EXACT_LOW_BIT);
        value->ev_negative = 0;
        if (power.ev_unit + _Exact_FindWideTop(&power.ev_sig) > _PREC_MATH_EXP_CLAMP_EXP) {
            value->ev_unit = power.ev_negative ? -(int) _PREC_MATH_EXP_CLAMP : (int) _PREC_MATH_EXP_CLAMP;
        } else {
            value->ev_unit = (int) _Prec_Math_ReadNearest(&power);
        }
        return 1;
    }
    if (y < 1 || y >= _EXACT_WIDE_BITS || _Prec_Math_ReadParity(y) == _PREC_MATH_PARITY_NONE) {
        return 0;
    }
    if ((int) y * (_Exact_FindWideTop(&odd) + 1) >= _EXACT_WIDE_BITS) {
        return 0;
    }
    _Exact_RaiseWide(&value->ev_sig, &odd, (int) y);
    value->ev_unit = unit * (int) y;
    return 1;
}

// Raise the domain error of x and y.
long double _Prec_Math_RaiseDomain(long double x, long double y)
{
    _Err_Errno = _SYS_EDOM;
    return (x * y) / (x * y);
}

// Raise the pole error, an exact -inf from a finite argument.
long double _Prec_Math_RaisePole(void)
{
    _Err_Errno = _SYS_ERANGE;
    _Fpu_RaiseFlags(_FPU_DIVBYZERO);
    return -_EXACT_INFINITY;
}

// Round a value to type, a range error if it rounds to an infinity or zero.
long double _Prec_Math_RoundRange(const _Exact_Value *value, _Exact_Type type)
{
    long double result = _Exact_RoundValue(value, type);

    if (_Exact_IsInf(result) || result == 0) {
        _Err_Errno = _SYS_ERANGE;
    }
    return result;
}

// Take e or 2 to x, within 2 ulps of type.
long double _Prec_Math_TakeExp(long double x, _Prec_Math_Base base, _Exact_Type type)
{
    _Exact_Value t;
    _Exact_Value value;
    _Exact_Value constant;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (_Exact_IsInf(x)) {
        return x > 0 ? x : 0.0L;
    }
    if (x == 0) {
        return 1.0L;
    }
    if (_Exact_ClearSign(x) > _PREC_MATH_EXP_CLAMP) {
        x = _Exact_CopySign(_PREC_MATH_EXP_CLAMP, x);
    }
    if (base == _PREC_MATH_BASE_2 && _Prec_Math_ReadParity(x) != _PREC_MATH_PARITY_NONE) {
        _Exact_SetWide(&value.ev_sig, _EXACT_LOW_BIT);
        value.ev_unit = (int) x;
        value.ev_negative = 0;
        value.ev_sticky = 0;
        return _Prec_Math_RoundRange(&value, type);
    }
    _Exact_MakeValue(&t, x);
    if (base == _PREC_MATH_BASE_2) {
        _Prec_Math_MakeConstant(&constant, _PREC_MATH_CONST_LN2);
        _Prec_Math_WorkMultiply(&t, &t, &constant);
    }
    _Prec_Math_ExpExact(&value, &t);
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take e to x less 1, within 2 ulps of type.
long double _Prec_Math_TakeExpm1(long double x, _Exact_Type type)
{
    _Exact_Value t;
    _Exact_Value value;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (_Exact_IsInf(x)) {
        return x > 0 ? x : -1.0L;
    }
    if (x == 0) {
        return x;
    }
    _Exact_MakeValue(&t, x);
    _Prec_Math_ExpM1Clamped(&value, &t);
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take the logarithm of x to base, within 2 ulps of type.
long double _Prec_Math_TakeLog(long double x, _Prec_Math_Base base, _Exact_Type type)
{
    int e;
    long k;
    _Exact_Value u;
    _Exact_Value num;
    _Exact_Value den;
    _Exact_Value value;
    _Exact_Value constant;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (x < 0) {
        return _Prec_Math_RaiseDomain(x, 0);
    }
    if (x == 0) {
        return _Prec_Math_RaisePole();
    }
    if (_Exact_IsInf(x)) {
        return x;
    }
    if (x == 1) {
        return 0.0L;
    }
    _Exact_MakeValue(&u, x);
    if (base == _PREC_MATH_BASE_2 && _Exact_ReadWideLow(&u.ev_sig) == _EXACT_LDOUBLE_INT_BIT) {
        return (long double) (u.ev_unit + _EXACT_LDOUBLE_INT_SHIFT);
    }
    e = _Prec_Math_SplitLog(&num, &den, &u);
    _Prec_Math_LogExact(&value, &num, &den, e);
    if (base != _PREC_MATH_BASE_E) {
        _Prec_Math_MakeConstant(&constant, base == _PREC_MATH_BASE_2 ? _PREC_MATH_CONST_LOG2E : _PREC_MATH_CONST_LOG10E);
        _Prec_Math_WorkMultiply(&value, &value, &constant);
    }
    k = _Prec_Math_ReadNearest(&value);
    if (base == _PREC_MATH_BASE_10 && _Prec_Math_IsPowerTen(x, k)) {
        return (long double) k;
    }
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take the natural logarithm of 1 plus x, within 2 ulps of type.
long double _Prec_Math_TakeLog1p(long double x, _Exact_Type type)
{
    _Exact_Value u;
    _Exact_Value value;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (x < -1) {
        return _Prec_Math_RaiseDomain(x, 0);
    }
    if (x == -1) {
        return _Prec_Math_RaisePole();
    }
    if (_Exact_IsInf(x) || x == 0) {
        return x;
    }
    _Exact_MakeValue(&u, x);
    _Prec_Math_Log1pExact(&value, &u);
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take x to y, within 2 ulps of type.
long double _Prec_Math_TakePow(long double x, long double y, _Exact_Type type)
{
    int e;
    _Bool negative;
    _Exact_Value u;
    _Exact_Value num;
    _Exact_Value den;
    _Exact_Value log;
    _Exact_Value value;
    long double result;
    _Prec_Math_Parity parity;

    if (y == 0 || x == 1) {
        return 1.0L;
    }
    if (_Exact_IsNan(x) || _Exact_IsNan(y)) {
        return x + y;
    }
    if (_Exact_IsInf(y)) {
        return _Exact_ClearSign(x) == 1 ? 1.0L : (_Exact_ClearSign(x) < 1) == (y < 0) ? _EXACT_INFINITY : 0.0L;
    }
    parity = _Prec_Math_ReadParity(y);
    negative = _Exact_IsNegative(x) && parity == _PREC_MATH_PARITY_ODD;
    if (x == 0 && y < 0) {
        return negative ? _Prec_Math_RaisePole() : -_Prec_Math_RaisePole();
    }
    if (x == 0 || _Exact_IsInf(x)) {
        result = (x == 0) == (y < 0) ? _EXACT_INFINITY : 0.0L;
        return negative ? -result : result;
    }
    if (x < 0 && parity == _PREC_MATH_PARITY_NONE) {
        return _Prec_Math_RaiseDomain(x, 0);
    }
    if (x > 0 && y == _PREC_MATH_POW_ROOT) {
        _Exact_MakeValue(&u, x);
        _Exact_ExtractRoot(&value, &u, _EXACT_ROOT_SQUARE);
        return _Exact_RoundValue(&value, type);
    }
    if (! _Prec_Math_PowerExact(&value, _Exact_ClearSign(x), y)) {
        _Exact_MakeValue(&u, _Exact_ClearSign(x));
        e = _Prec_Math_SplitLog(&num, &den, &u);
        _Prec_Math_LogExact(&log, &num, &den, e);
        _Exact_MakeValue(&u, y);
        _Prec_Math_WorkMultiply(&log, &u, &log);
        _Prec_Math_ExpExact(&value, &log);
        value.ev_sticky = 1;
    }
    value.ev_negative = negative;
    return _Prec_Math_RoundRange(&value, type);
}

// Nudge a working value toward its true magnitude, a unit below or above.
void _Prec_Math_NudgeExact(_Exact_Value *e, _Exact_Step step)
{
    int shift;
    _Exact_Wide one;

    if (step == _EXACT_STEP_DOWN) {
        shift = _PREC_MATH_WORK_BITS - 1 - _Exact_FindWideTop(&e->ev_sig);
        _Exact_ShiftWideLeft(&e->ev_sig, shift);
        e->ev_unit -= shift;
        _Exact_SetWide(&one, _EXACT_LOW_BIT);
        _Exact_SubtractWide(&e->ev_sig, &one);
    }
    e->ev_sticky = 1;
}

// Take the square root of a positive working value a by its reciprocal.
void _Prec_Math_WorkRoot(_Exact_Value *root, const _Exact_Value *a)
{
    long double m;
    long double s;
    _Exact_Wide top;
    _Exact_Value y;
    _Exact_Value one;
    _Exact_Value step;
    _Exact_Value scaled = *a;
    int exp = a->ev_unit + _Exact_FindWideTop(&a->ev_sig);
    int half = (exp - (exp & 1)) / 2;

    scaled.ev_unit -= 2 * half;
    top = scaled.ev_sig;
    _Exact_AlignWide(&top, _EXACT_LDOUBLE_INT_SHIFT - _Exact_FindWideTop(&top));
    m = _Exact_JoinLDouble(exp - 2 * half, _Exact_ReadWideLow(&top));
    s = m;
    for (int i = 0; i < _PREC_MATH_ROOT_GUESSES; i++) {
        s = (s + m / s) / 2;
    }
    _Exact_MakeValue(&y, 1 / s);
    _Exact_MakeValue(&one, 1.0L);
    for (int i = 0; i < _PREC_MATH_ROOT_STEPS; i++) {
        _Prec_Math_WorkMultiply(&step, &y, &y);
        _Prec_Math_WorkMultiply(&step, &scaled, &step);
        step.ev_negative = 1;
        _Prec_Math_WorkAdd(&step, &one, &step);
        _Prec_Math_WorkMultiply(&step, &y, &step);
        step.ev_unit--;
        _Prec_Math_WorkAdd(&y, &y, &step);
    }
    _Prec_Math_WorkMultiply(root, &scaled, &y);
    root->ev_unit += half;
}

// Make quarters of pi as a working value.
void _Prec_Math_MakeQuarters(_Exact_Value *e, unsigned int quarters)
{
    _Exact_Value count;

    _Prec_Math_MakeConstant(e, _PREC_MATH_CONST_PI);
    _Exact_MakeValue(&count, (long double) quarters);
    _Prec_Math_WorkMultiply(e, e, &count);
    e->ev_unit += _PREC_MATH_QUARTER_EXP;
}

// Reduce an x of 1 or more to r in [-pi/4, pi/4], returning its quadrant.
unsigned int _Prec_Math_ReduceTrig(_Exact_Value *r, long double x)
{
    int exp;
    int point;
    int first = 0;
    _Exact_Wide one;
    _Exact_Wide part;
    _Exact_Wide chunk;
    _Exact_Value half_pi;
    unsigned int quadrant = 0;
    unsigned long long sig = _Exact_SplitLDouble(x, &exp);
    int unit = exp - _EXACT_LDOUBLE_INT_SHIFT;

    if (unit > _PREC_MATH_QUADRANT_BITS) {
        first = (unit - _PREC_MATH_QUADRANT_BITS) / _EXACT_LIMB_BITS;
    }
    _Exact_SetWide(&chunk, 0);
    for (int i = 0; i < _PREC_MATH_TRIG_WINDOW; i++) {
        chunk.ew_limb[i] = _Prec_Math_TwoOverPi[first + _PREC_MATH_TRIG_WINDOW - 1 - i];
    }
    _Exact_SetWide(&part, sig);
    _Exact_MultiplyWides(&r->ev_sig, &part, &chunk);
    point = _EXACT_LIMB_BITS * (first + _PREC_MATH_TRIG_WINDOW) - unit;
    for (int i = _PREC_MATH_QUADRANT_BITS - 1; i >= 0; i--) {
        if (point + i < _EXACT_WIDE_BITS) {
            quadrant = quadrant << 1 | _Exact_ReadWideBit(&r->ev_sig, point + i);
        }
    }
    _Exact_ShiftWideLeft(&r->ev_sig, _EXACT_WIDE_BITS - point);
    _Exact_ShiftWideRight(&r->ev_sig, _EXACT_WIDE_BITS - point);
    r->ev_unit = -point;
    r->ev_negative = 0;
    r->ev_sticky = 0;
    if (_Exact_ReadWideBit(&r->ev_sig, point - 1) != 0) {
        _Exact_SetWide(&one, _EXACT_LOW_BIT);
        _Exact_ShiftWideLeft(&one, point);
        _Exact_SubtractWide(&one, &r->ev_sig);
        r->ev_sig = one;
        r->ev_negative = 1;
        quadrant++;
    }
    _Prec_Math_TrimExact(r);
    _Prec_Math_MakeQuarters(&half_pi, _PREC_MATH_HALF_PI);
    _Prec_Math_WorkMultiply(r, r, &half_pi);
    return quadrant & _PREC_MATH_QUADRANT_MASK;
}

// Take the sine or the cosine of a reduced r by its series.
void _Prec_Math_TrigExact(_Exact_Value *value, const _Exact_Value *r, _Prec_Math_Trig which)
{
    _Exact_Value z;
    _Exact_Value one;
    _Exact_Value series;
    unsigned int n = which == _PREC_MATH_TRIG_SIN ? _PREC_MATH_TRIG_TERMS - 1 : _PREC_MATH_TRIG_TERMS;

    _Prec_Math_WorkMultiply(&z, r, r);
    _Exact_MakeValue(&one, 1.0L);
    series = one;
    for (; n >= 2; n -= 2) {
        _Prec_Math_WorkMultiply(&series, &z, &series);
        _Prec_Math_WorkDivideSmall(&series, n * (n - 1));
        series.ev_negative = 1;
        _Prec_Math_WorkAdd(&series, &one, &series);
    }
    if (which == _PREC_MATH_TRIG_SIN) {
        _Prec_Math_WorkMultiply(value, r, &series);
    } else {
        *value = series;
    }
}

// Take the arc tangent of num over den, both positive.
void _Prec_Math_AtanExact(_Exact_Value *value, const _Exact_Value *num, const _Exact_Value *den)
{
    _Exact_Value t;
    _Exact_Value z;
    _Exact_Value one;
    _Exact_Value root;
    _Exact_Value term;
    _Exact_Value quarter;
    _Bool swap = 0;

    _Prec_Math_WorkDivide(&t, num, den);
    if (t.ev_unit + _Exact_FindWideTop(&t.ev_sig) >= 0) {
        _Prec_Math_WorkDivide(&t, den, num);
        swap = 1;
    }
    if (t.ev_unit + _Exact_FindWideTop(&t.ev_sig) < _PREC_MATH_TINY_EXP) {
        *value = t;
        _Prec_Math_NudgeExact(value, _EXACT_STEP_DOWN);
    } else {
        _Exact_MakeValue(&one, 1.0L);
        for (int i = 0; i < _PREC_MATH_ATAN_HALVINGS; i++) {
            _Prec_Math_WorkMultiply(&z, &t, &t);
            _Prec_Math_WorkAdd(&z, &z, &one);
            _Prec_Math_WorkRoot(&root, &z);
            _Prec_Math_WorkAdd(&root, &root, &one);
            _Prec_Math_WorkDivide(&t, &t, &root);
        }
        _Prec_Math_WorkMultiply(&z, &t, &t);
        _Prec_Math_MakeReciprocal(value, 2 * _PREC_MATH_ATAN_TERMS - 1);
        for (int k = _PREC_MATH_ATAN_TERMS - 2; k >= 0; k--) {
            _Prec_Math_WorkMultiply(value, &z, value);
            value->ev_negative = 1;
            _Prec_Math_MakeReciprocal(&term, (unsigned int) (2 * k + 1));
            _Prec_Math_WorkAdd(value, &term, value);
        }
        _Prec_Math_WorkMultiply(value, &t, value);
        value->ev_unit += _PREC_MATH_ATAN_HALVINGS;
    }
    if (swap) {
        _Prec_Math_MakeQuarters(&quarter, _PREC_MATH_HALF_PI);
        value->ev_negative = 1;
        _Prec_Math_WorkAdd(value, &quarter, value);
    }
}

// Take a tiny x's function, x nudged by step, to type.
long double _Prec_Math_TakeTiny(long double x, _Exact_Step step, _Exact_Type type)
{
    _Exact_Value value;

    _Exact_MakeValue(&value, x);
    _Prec_Math_NudgeExact(&value, step);
    return _Prec_Math_RoundRange(&value, type);
}

// Take quarters of pi to type, with sign's sign.
long double _Prec_Math_TakeQuarters(unsigned int quarters, long double sign, _Exact_Type type)
{
    _Exact_Value value;

    _Prec_Math_MakeQuarters(&value, quarters);
    value.ev_negative = _Exact_IsNegative(sign) != 0;
    value.ev_sticky = 1;
    return _Exact_RoundValue(&value, type);
}

// Take the sine, the cosine or the tangent of x, within 2 ulps of type.
long double _Prec_Math_TakeTrig(long double x, _Prec_Math_Trig which, _Exact_Type type)
{
    _Bool odd;
    _Bool flip;
    _Exact_Value r;
    _Exact_Value sine;
    _Exact_Value value;
    _Exact_Value cosine;
    unsigned int quadrant = 0;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (_Exact_IsInf(x)) {
        return _Prec_Math_RaiseDomain(x, 0);
    }
    if (x == 0) {
        return which == _PREC_MATH_TRIG_COS ? 1.0L : x;
    }
    if (_Exact_ClearSign(x) < _PREC_MATH_TINY && which == _PREC_MATH_TRIG_COS) {
        return _Prec_Math_TakeTiny(1.0L, _EXACT_STEP_DOWN, type);
    }
    if (_Exact_ClearSign(x) < _PREC_MATH_TINY) {
        return _Prec_Math_TakeTiny(x, which == _PREC_MATH_TRIG_SIN ? _EXACT_STEP_DOWN : _EXACT_STEP_UP, type);
    }
    if (_Exact_ClearSign(x) < 1) {
        _Exact_MakeValue(&r, _Exact_ClearSign(x));
    } else {
        quadrant = _Prec_Math_ReduceTrig(&r, _Exact_ClearSign(x));
    }
    odd = (quadrant & _PREC_MATH_QUADRANT_ODD) != 0;
    switch (which) {
        case _PREC_MATH_TRIG_SIN: {
            _Prec_Math_TrigExact(&value, &r, odd ? _PREC_MATH_TRIG_COS : _PREC_MATH_TRIG_SIN);
            flip = ((quadrant & _PREC_MATH_QUADRANT_HALF) != 0) != (_Exact_IsNegative(x) != 0);
        } break;
        case _PREC_MATH_TRIG_COS: {
            _Prec_Math_TrigExact(&value, &r, odd ? _PREC_MATH_TRIG_SIN : _PREC_MATH_TRIG_COS);
            flip = ((quadrant + _PREC_MATH_QUADRANT_ODD) & _PREC_MATH_QUADRANT_HALF) != 0;
        } break;
        default: {
            _Prec_Math_TrigExact(&sine, &r, _PREC_MATH_TRIG_SIN);
            _Prec_Math_TrigExact(&cosine, &r, _PREC_MATH_TRIG_COS);
            _Prec_Math_WorkDivide(&value, odd ? &cosine : &sine, odd ? &sine : &cosine);
            flip = odd != (_Exact_IsNegative(x) != 0);
        } break;
    }
    if (flip) {
        value.ev_negative = ! value.ev_negative;
    }
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take the arc sine or the arc cosine of x, within 2 ulps of type.
long double _Prec_Math_TakeArc(long double x, _Prec_Math_Trig which, _Exact_Type type)
{
    _Exact_Value one;
    _Exact_Value low;
    _Exact_Value high;
    _Exact_Value side;
    _Exact_Value root;
    _Exact_Value value;
    _Exact_Value whole;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (_Exact_ClearSign(x) > 1) {
        return _Prec_Math_RaiseDomain(x, 0);
    }
    if (which == _PREC_MATH_TRIG_SIN && x == 0) {
        return x;
    }
    if (which == _PREC_MATH_TRIG_SIN && _Exact_ClearSign(x) < _PREC_MATH_TINY) {
        return _Prec_Math_TakeTiny(x, _EXACT_STEP_UP, type);
    }
    if (which == _PREC_MATH_TRIG_SIN && _Exact_ClearSign(x) == 1) {
        return _Prec_Math_TakeQuarters(_PREC_MATH_HALF_PI, x, type);
    }
    if (which == _PREC_MATH_TRIG_COS && x == 1) {
        return 0.0L;
    }
    if (which == _PREC_MATH_TRIG_COS && (x == 0 || x == -1)) {
        return _Prec_Math_TakeQuarters(x == 0 ? _PREC_MATH_HALF_PI : _PREC_MATH_WHOLE_PI, 1.0L, type);
    }
    _Exact_MakeValue(&side, _Exact_ClearSign(x));
    _Exact_MakeValue(&one, 1.0L);
    _Prec_Math_WorkAdd(&high, &one, &side);
    side.ev_negative = 1;
    _Prec_Math_WorkAdd(&low, &one, &side);
    side.ev_negative = 0;
    _Prec_Math_WorkMultiply(&low, &low, &high);
    _Prec_Math_WorkRoot(&root, &low);
    if (which == _PREC_MATH_TRIG_SIN) {
        _Prec_Math_AtanExact(&value, &side, &root);
        value.ev_negative = x < 0;
    } else {
        _Prec_Math_AtanExact(&value, &root, &side);
    }
    if (which == _PREC_MATH_TRIG_COS && x < 0) {
        _Prec_Math_MakeQuarters(&whole, _PREC_MATH_WHOLE_PI);
        value.ev_negative = 1;
        _Prec_Math_WorkAdd(&value, &whole, &value);
    }
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take the arc tangent of y over x by both signs, within 2 ulps of type.
long double _Prec_Math_TakeAtan2(long double y, long double x, _Exact_Type type)
{
    _Exact_Value num;
    _Exact_Value den;
    _Exact_Value value;
    _Exact_Value whole;

    if (_Exact_IsNan(x) || _Exact_IsNan(y)) {
        return x + y;
    }
    if (y == 0) {
        return _Exact_IsNegative(x) ? _Prec_Math_TakeQuarters(_PREC_MATH_WHOLE_PI, y, type) : y;
    }
    if (_Exact_IsInf(y) && _Exact_IsInf(x)) {
        return _Prec_Math_TakeQuarters(x > 0 ? _PREC_MATH_QUARTER_PI : _PREC_MATH_THREE_QUARTER_PI, y, type);
    }
    if (_Exact_IsInf(y) || x == 0) {
        return _Prec_Math_TakeQuarters(_PREC_MATH_HALF_PI, y, type);
    }
    if (_Exact_IsInf(x)) {
        return x > 0 ? _Exact_CopySign(0.0L, y) : _Prec_Math_TakeQuarters(_PREC_MATH_WHOLE_PI, y, type);
    }
    _Exact_MakeValue(&num, _Exact_ClearSign(y));
    _Exact_MakeValue(&den, _Exact_ClearSign(x));
    _Prec_Math_AtanExact(&value, &num, &den);
    if (x < 0) {
        _Prec_Math_MakeQuarters(&whole, _PREC_MATH_WHOLE_PI);
        value.ev_negative = 1;
        _Prec_Math_WorkAdd(&value, &whole, &value);
    }
    value.ev_negative = _Exact_IsNegative(y) != 0;
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take the hyperbolic sine, cosine or tangent of x, within 2 ulps of type.
long double _Prec_Math_TakeHyperbolic(long double x, _Prec_Math_Trig which, _Exact_Type type)
{
    _Exact_Value t;
    _Exact_Value em1;
    _Exact_Value one;
    _Exact_Value sum;
    _Exact_Value part;
    _Exact_Value value;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (_Exact_IsInf(x)) {
        return which == _PREC_MATH_TRIG_COS ? _EXACT_INFINITY : which == _PREC_MATH_TRIG_SIN ? x : _Exact_CopySign(1.0L, x);
    }
    if (x == 0) {
        return which == _PREC_MATH_TRIG_COS ? 1.0L : x;
    }
    if (_Exact_ClearSign(x) < _PREC_MATH_TINY && which == _PREC_MATH_TRIG_COS) {
        return _Prec_Math_TakeTiny(1.0L, _EXACT_STEP_UP, type);
    }
    if (_Exact_ClearSign(x) < _PREC_MATH_TINY) {
        return _Prec_Math_TakeTiny(x, which == _PREC_MATH_TRIG_SIN ? _EXACT_STEP_UP : _EXACT_STEP_DOWN, type);
    }
    _Exact_MakeValue(&t, _Exact_ClearSign(x));
    if (which == _PREC_MATH_TRIG_TAN) {
        t.ev_unit++;
    }
    _Prec_Math_ExpM1Clamped(&em1, &t);
    _Exact_MakeValue(&one, 1.0L);
    switch (which) {
        case _PREC_MATH_TRIG_SIN: {
            _Prec_Math_WorkAdd(&sum, &em1, &one);
            _Prec_Math_WorkDivide(&part, &em1, &sum);
            _Prec_Math_WorkAdd(&value, &em1, &part);
            value.ev_unit--;
        } break;
        case _PREC_MATH_TRIG_COS: {
            _Prec_Math_WorkAdd(&sum, &em1, &one);
            _Prec_Math_WorkMultiply(&part, &em1, &em1);
            _Prec_Math_WorkDivide(&value, &part, &sum);
            value.ev_unit--;
            _Prec_Math_WorkAdd(&value, &one, &value);
        } break;
        default: {
            _Prec_Math_WorkAdd(&sum, &one, &one);
            _Prec_Math_WorkAdd(&sum, &em1, &sum);
            _Prec_Math_WorkDivide(&value, &em1, &sum);
        } break;
    }
    value.ev_negative = which != _PREC_MATH_TRIG_COS && x < 0;
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take the inverse hyperbolic sine, cosine or tangent of x, within 2 ulps.
long double _Prec_Math_TakeArcHyperbolic(long double x, _Prec_Math_Trig which, _Exact_Type type)
{
    _Exact_Value a;
    _Exact_Value u;
    _Exact_Value one;
    _Exact_Value sum;
    _Exact_Value part;
    _Exact_Value root;
    _Exact_Value value;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if ((which == _PREC_MATH_TRIG_COS && x < 1) || (which == _PREC_MATH_TRIG_TAN && _Exact_ClearSign(x) > 1)) {
        return _Prec_Math_RaiseDomain(x, 0);
    }
    if (which == _PREC_MATH_TRIG_TAN && _Exact_ClearSign(x) == 1) {
        return x > 0 ? -_Prec_Math_RaisePole() : _Prec_Math_RaisePole();
    }
    if (_Exact_IsInf(x) || x == 0) {
        return x;
    }
    if (which == _PREC_MATH_TRIG_COS && x == 1) {
        return 0.0L;
    }
    if (_Exact_ClearSign(x) < _PREC_MATH_TINY) {
        return _Prec_Math_TakeTiny(x, which == _PREC_MATH_TRIG_SIN ? _EXACT_STEP_DOWN : _EXACT_STEP_UP, type);
    }
    _Exact_MakeValue(&a, _Exact_ClearSign(x));
    _Exact_MakeValue(&one, 1.0L);
    switch (which) {
        case _PREC_MATH_TRIG_SIN: {
            _Exact_MultiplyValues(&part, &a, &a);
            _Prec_Math_WorkAdd(&sum, &part, &one);
            _Prec_Math_WorkRoot(&root, &sum);
            _Prec_Math_WorkAdd(&root, &root, &one);
            _Prec_Math_WorkDivide(&part, &part, &root);
            _Prec_Math_WorkAdd(&u, &a, &part);
        } break;
        case _PREC_MATH_TRIG_COS: {
            _Prec_Math_WorkAdd(&sum, &a, &one);
            one.ev_negative = 1;
            _Prec_Math_WorkAdd(&u, &a, &one);
            _Prec_Math_WorkMultiply(&part, &u, &sum);
            _Prec_Math_WorkRoot(&root, &part);
            _Prec_Math_WorkAdd(&u, &u, &root);
        } break;
        default: {
            a.ev_negative = 1;
            _Exact_AddValues(&sum, &one, &a);
            a.ev_negative = 0;
            _Prec_Math_WorkDivide(&u, &a, &sum);
            u.ev_unit++;
        } break;
    }
    _Prec_Math_Log1pExact(&value, &u);
    if (which == _PREC_MATH_TRIG_TAN) {
        value.ev_unit--;
    }
    value.ev_negative = which != _PREC_MATH_TRIG_COS && x < 0;
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take erf of an a in (0, split) by its series of positive terms.
void _Prec_Math_ErfExact(_Exact_Value *value, long double a)
{
    _Exact_Value z;
    _Exact_Value one;
    _Exact_Value side;
    _Exact_Value decay;
    _Exact_Value series;
    _Exact_Value constant;
    unsigned int n = _PREC_MATH_ERF_TERMS_BASE + _PREC_MATH_ERF_TERMS_STEP * (1 + (unsigned int) a);

    _Exact_MakeValue(&side, a);
    _Exact_MultiplyValues(&z, &side, &side);
    z.ev_unit++;
    _Exact_MakeValue(&one, 1.0L);
    series = one;
    for (; n >= 1; n--) {
        _Prec_Math_WorkMultiply(&series, &z, &series);
        _Prec_Math_WorkDivideSmall(&series, 2 * n + 1);
        _Prec_Math_WorkAdd(&series, &one, &series);
    }
    z.ev_unit--;
    z.ev_negative = 1;
    _Prec_Math_ExpExact(&decay, &z);
    _Prec_Math_TrimExact(&decay);
    _Prec_Math_WorkMultiply(value, &series, &decay);
    _Prec_Math_WorkMultiply(value, &side, value);
    _Prec_Math_MakeConstant(&constant, _PREC_MATH_CONST_INV_ROOT_PI);
    _Prec_Math_WorkMultiply(value, &constant, value);
    value->ev_unit++;
}

// Take erfc of an a from the split on by its continued fraction.
void _Prec_Math_ErfcExact(_Exact_Value *value, long double a)
{
    _Exact_Value z;
    _Exact_Value num;
    _Exact_Value den;
    _Exact_Value half;
    _Exact_Value next;
    _Exact_Value part;
    _Exact_Value side;
    _Exact_Value count;
    _Exact_Value decay;
    _Exact_Value prev_num;
    _Exact_Value prev_den;
    _Exact_Value constant;

    _Exact_MakeValue(&side, a);
    _Exact_MakeValue(&prev_num, 1.0L);
    num = side;
    prev_den = side;
    _Exact_MultiplyValues(&z, &side, &side);
    _Exact_MakeValue(&half, _PREC_MATH_HALF);
    _Prec_Math_WorkAdd(&den, &z, &half);
    for (int k = 3; k <= _PREC_MATH_ERFC_TERMS; k++) {
        _Exact_MakeValue(&count, (long double) (k - 1));
        _Prec_Math_WorkMultiply(&part, &side, &num);
        _Prec_Math_WorkMultiply(&next, &count, &prev_num);
        next.ev_unit--;
        prev_num = num;
        _Prec_Math_WorkAdd(&num, &part, &next);
        _Prec_Math_WorkMultiply(&part, &side, &den);
        _Prec_Math_WorkMultiply(&next, &count, &prev_den);
        next.ev_unit--;
        prev_den = den;
        _Prec_Math_WorkAdd(&den, &part, &next);
    }
    z.ev_negative = 1;
    _Prec_Math_ExpExact(&decay, &z);
    _Prec_Math_TrimExact(&decay);
    _Prec_Math_WorkDivide(value, &num, &den);
    _Prec_Math_WorkMultiply(value, &decay, value);
    _Prec_Math_MakeConstant(&constant, _PREC_MATH_CONST_INV_ROOT_PI);
    _Prec_Math_WorkMultiply(value, &constant, value);
}

// Make Stirling's kth coefficient, from 0, as a value, untrimmed.
void _Prec_Math_MakeStirling(_Exact_Value *e, int k)
{
    _Exact_SetWide(&e->ev_sig, 0);
    _Str_CopyBytes(e->ev_sig.ew_limb, _Prec_Math_Stirling[k], sizeof(_Prec_Math_Stirling[k]));
    e->ev_unit = _Prec_Math_StirlingUnits[k];
    e->ev_negative = (k & 1) != 0;
    e->ev_sticky = 0;
}

// Take the logarithm of gamma of a y from the threshold on by Stirling.
void _Prec_Math_StirlingExact(_Exact_Value *log, const _Exact_Value *y)
{
    int e;
    _Exact_Value w;
    _Exact_Value z;
    _Exact_Value one;
    _Exact_Value num;
    _Exact_Value den;
    _Exact_Value part;
    _Exact_Value series;
    _Exact_Value constant;

    _Exact_MakeValue(&one, 1.0L);
    _Prec_Math_WorkDivide(&w, &one, y);
    _Prec_Math_WorkMultiply(&z, &w, &w);
    _Prec_Math_MakeStirling(&series, _PREC_MATH_STIRLING_TERMS - 1);
    for (int k = _PREC_MATH_STIRLING_TERMS - 2; k >= 0; k--) {
        _Prec_Math_WorkMultiply(&series, &z, &series);
        _Prec_Math_MakeStirling(&constant, k);
        _Prec_Math_WorkAdd(&series, &constant, &series);
    }
    _Prec_Math_WorkMultiply(&series, &w, &series);
    e = _Prec_Math_SplitLog(&num, &den, y);
    _Prec_Math_LogExact(log, &num, &den, e);
    _Exact_MakeValue(&part, -_PREC_MATH_HALF);
    _Prec_Math_WorkAdd(&part, y, &part);
    _Prec_Math_WorkMultiply(log, &part, log);
    part = *y;
    part.ev_negative = 1;
    _Prec_Math_WorkAdd(log, log, &part);
    _Prec_Math_MakeConstant(&constant, _PREC_MATH_CONST_LN_ROOT_2PI);
    _Prec_Math_WorkAdd(log, log, &constant);
    _Prec_Math_WorkAdd(log, log, &series);
}

// Take the logarithm of |gamma| of an x off the poles, returning its sign.
_Bool _Prec_Math_LogGammaExact(_Exact_Value *log, long double x)
{
    int e;
    long whole;
    long double rest;
    long double turn;
    _Bool negative;
    _Exact_Value y;
    _Exact_Value r;
    _Exact_Value one;
    _Exact_Value num;
    _Exact_Value den;
    _Exact_Value part;
    _Exact_Value sine;
    _Exact_Value product;

    _Exact_MakeValue(&one, 1.0L);
    _Exact_MakeValue(&y, x);
    if (x >= _PREC_MATH_STIRLING_FROM) {
        _Prec_Math_StirlingExact(log, &y);
        return 0;
    }
    whole = (long) x;
    if (x < whole) {
        whole--;
    }
    if (x > -_PREC_MATH_STIRLING_FROM) {
        product = y;
        for (long i = whole + 1; i < _PREC_MATH_STIRLING_FROM; i++) {
            _Prec_Math_WorkAdd(&y, &y, &one);
            _Prec_Math_WorkMultiply(&product, &product, &y);
        }
        _Prec_Math_WorkAdd(&y, &y, &one);
        _Prec_Math_StirlingExact(log, &y);
        negative = product.ev_negative;
        product.ev_negative = 0;
        e = _Prec_Math_SplitLog(&num, &den, &product);
        _Prec_Math_LogExact(&part, &num, &den, e);
        part.ev_negative = ! part.ev_negative;
        _Prec_Math_WorkAdd(log, log, &part);
        return negative;
    }
    rest = x - (long double) whole;
    if (rest > _PREC_MATH_HALF) {
        rest -= 1;
        whole++;
    }
    negative = (rest < 0) != ((whole & 1) != 0);
    turn = _Exact_ClearSign(rest) > _PREC_MATH_SINE_TURN ? _PREC_MATH_HALF - _Exact_ClearSign(rest) : _Exact_ClearSign(rest);
    sine = one;
    if (turn != 0) {
        _Exact_MakeValue(&part, turn);
        _Prec_Math_MakeConstant(&r, _PREC_MATH_CONST_PI);
        _Prec_Math_WorkMultiply(&r, &r, &part);
        _Prec_Math_TrigExact(&sine, &r, _Exact_ClearSign(rest) > _PREC_MATH_SINE_TURN ? _PREC_MATH_TRIG_COS : _PREC_MATH_TRIG_SIN);
    }
    _Prec_Math_MakeConstant(&r, _PREC_MATH_CONST_PI);
    _Prec_Math_WorkDivide(&r, &r, &sine);
    e = _Prec_Math_SplitLog(&num, &den, &r);
    _Prec_Math_LogExact(log, &num, &den, e);
    y.ev_negative = 0;
    _Prec_Math_WorkAdd(&y, &y, &one);
    _Prec_Math_StirlingExact(&part, &y);
    part.ev_negative = 1;
    _Prec_Math_WorkAdd(log, log, &part);
    return negative;
}

// Take erf of x, within 2 ulps of type.
long double _Prec_Math_TakeErf(long double x, _Exact_Type type)
{
    _Exact_Value one;
    _Exact_Value value;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (_Exact_IsInf(x)) {
        return _Exact_CopySign(1.0L, x);
    }
    if (x == 0) {
        return x;
    }
    if (_Exact_ClearSign(x) < _PREC_MATH_ERF_SPLIT) {
        _Prec_Math_ErfExact(&value, _Exact_ClearSign(x));
    } else {
        _Prec_Math_ErfcExact(&value, _Exact_ClearSign(x));
        _Exact_MakeValue(&one, 1.0L);
        value.ev_negative = 1;
        _Prec_Math_WorkAdd(&value, &one, &value);
    }
    value.ev_negative = x < 0;
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take erfc of x, within 2 ulps of type.
long double _Prec_Math_TakeErfc(long double x, _Exact_Type type)
{
    _Exact_Value base;
    _Exact_Value value;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (_Exact_IsInf(x)) {
        return x > 0 ? 0.0L : 2.0L;
    }
    if (x == 0) {
        return 1.0L;
    }
    if (_Exact_ClearSign(x) < _PREC_MATH_ERF_SPLIT) {
        _Prec_Math_ErfExact(&value, _Exact_ClearSign(x));
        _Exact_MakeValue(&base, 1.0L);
        value.ev_negative = x > 0;
        _Prec_Math_WorkAdd(&value, &base, &value);
    } else {
        _Prec_Math_ErfcExact(&value, _Exact_ClearSign(x));
    }
    if (x <= -_PREC_MATH_ERF_SPLIT) {
        _Exact_MakeValue(&base, 2.0L);
        value.ev_negative = 1;
        _Prec_Math_WorkAdd(&value, &base, &value);
    }
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take the logarithm of |gamma| of x, within 2 ulps of type.
long double _Prec_Math_TakeLgamma(long double x, _Exact_Type type)
{
    _Exact_Value value;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (_Exact_IsInf(x)) {
        return _EXACT_INFINITY;
    }
    if (x == 1 || x == 2) {
        return 0.0L;
    }
    if (x == 0 || (x < 0 && _Prec_Math_ReadParity(x) != _PREC_MATH_PARITY_NONE)) {
        return -_Prec_Math_RaisePole();
    }
    _Prec_Math_LogGammaExact(&value, x);
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take gamma of x, within 2 ulps of type, of an integer exact.
long double _Prec_Math_TakeTgamma(long double x, _Exact_Type type)
{
    _Exact_Wide factor;
    _Exact_Value log;
    _Exact_Value value;
    _Bool negative;

    if (_Exact_IsNan(x)) {
        return x + x;
    }
    if (x == 0) {
        return _Exact_IsNegative(x) ? _Prec_Math_RaisePole() : -_Prec_Math_RaisePole();
    }
    if (_Exact_IsInf(x) && x > 0) {
        return x;
    }
    if (_Exact_IsInf(x) || (x < 0 && _Prec_Math_ReadParity(x) != _PREC_MATH_PARITY_NONE)) {
        return _Prec_Math_RaiseDomain(x, 0);
    }
    if (x > 0 && x <= _PREC_MATH_FACTORIAL_MAX && _Prec_Math_ReadParity(x) != _PREC_MATH_PARITY_NONE) {
        _Exact_SetWide(&value.ev_sig, _EXACT_LOW_BIT);
        value.ev_unit = 0;
        value.ev_negative = 0;
        value.ev_sticky = 0;
        for (int k = 2; k < (int) x; k++) {
            _Exact_SetWide(&factor, (unsigned long long) k);
            _Exact_MultiplyWides(&value.ev_sig, &value.ev_sig, &factor);
        }
        return _Prec_Math_RoundRange(&value, type);
    }
    negative = _Prec_Math_LogGammaExact(&log, x);
    _Prec_Math_ExpExact(&value, &log);
    value.ev_negative = negative;
    value.ev_sticky = 1;
    return _Prec_Math_RoundRange(&value, type);
}

// Take the arc cosine of x.
double _prec_acos(double x)
{
    return (double) _Prec_Math_TakeArc(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_DOUBLE);
}

// Take the arc cosine of x.
float _prec_acosf(float x)
{
    return (float) _Prec_Math_TakeArc(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_FLOAT);
}

// Take the arc cosine of x.
long double _prec_acosl(long double x)
{
    return _Prec_Math_TakeArc(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_LDOUBLE);
}

// Take the arc sine of x.
double _prec_asin(double x)
{
    return (double) _Prec_Math_TakeArc(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_DOUBLE);
}

// Take the arc sine of x.
float _prec_asinf(float x)
{
    return (float) _Prec_Math_TakeArc(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_FLOAT);
}

// Take the arc sine of x.
long double _prec_asinl(long double x)
{
    return _Prec_Math_TakeArc(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_LDOUBLE);
}

// Take the arc tangent of x.
double _prec_atan(double x)
{
    return (double) _Prec_Math_TakeAtan2(x, 1.0L, _EXACT_TYPE_DOUBLE);
}

// Take the arc tangent of x.
float _prec_atanf(float x)
{
    return (float) _Prec_Math_TakeAtan2(x, 1.0L, _EXACT_TYPE_FLOAT);
}

// Take the arc tangent of x.
long double _prec_atanl(long double x)
{
    return _Prec_Math_TakeAtan2(x, 1.0L, _EXACT_TYPE_LDOUBLE);
}

// Take the arc tangent of y over x by both signs.
double _prec_atan2(double y, double x)
{
    return (double) _Prec_Math_TakeAtan2(y, x, _EXACT_TYPE_DOUBLE);
}

// Take the arc tangent of y over x by both signs.
float _prec_atan2f(float y, float x)
{
    return (float) _Prec_Math_TakeAtan2(y, x, _EXACT_TYPE_FLOAT);
}

// Take the arc tangent of y over x by both signs.
long double _prec_atan2l(long double y, long double x)
{
    return _Prec_Math_TakeAtan2(y, x, _EXACT_TYPE_LDOUBLE);
}

// Take the cosine of x.
double _prec_cos(double x)
{
    return (double) _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_DOUBLE);
}

// Take the cosine of x.
float _prec_cosf(float x)
{
    return (float) _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_FLOAT);
}

// Take the cosine of x.
long double _prec_cosl(long double x)
{
    return _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_LDOUBLE);
}

// Take the sine of x.
double _prec_sin(double x)
{
    return (double) _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_DOUBLE);
}

// Take the sine of x.
float _prec_sinf(float x)
{
    return (float) _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_FLOAT);
}

// Take the sine of x.
long double _prec_sinl(long double x)
{
    return _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_LDOUBLE);
}

// Take the tangent of x.
double _prec_tan(double x)
{
    return (double) _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_DOUBLE);
}

// Take the tangent of x.
float _prec_tanf(float x)
{
    return (float) _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_FLOAT);
}

// Take the tangent of x.
long double _prec_tanl(long double x)
{
    return _Prec_Math_TakeTrig(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_LDOUBLE);
}

// Take the inverse hyperbolic cosine of x.
double _prec_acosh(double x)
{
    return (double) _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_DOUBLE);
}

// Take the inverse hyperbolic cosine of x.
float _prec_acoshf(float x)
{
    return (float) _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_FLOAT);
}

// Take the inverse hyperbolic cosine of x.
long double _prec_acoshl(long double x)
{
    return _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_LDOUBLE);
}

// Take the inverse hyperbolic sine of x.
double _prec_asinh(double x)
{
    return (double) _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_DOUBLE);
}

// Take the inverse hyperbolic sine of x.
float _prec_asinhf(float x)
{
    return (float) _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_FLOAT);
}

// Take the inverse hyperbolic sine of x.
long double _prec_asinhl(long double x)
{
    return _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_LDOUBLE);
}

// Take the inverse hyperbolic tangent of x.
double _prec_atanh(double x)
{
    return (double) _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_DOUBLE);
}

// Take the inverse hyperbolic tangent of x.
float _prec_atanhf(float x)
{
    return (float) _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_FLOAT);
}

// Take the inverse hyperbolic tangent of x.
long double _prec_atanhl(long double x)
{
    return _Prec_Math_TakeArcHyperbolic(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_LDOUBLE);
}

// Take the hyperbolic cosine of x.
double _prec_cosh(double x)
{
    return (double) _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_DOUBLE);
}

// Take the hyperbolic cosine of x.
float _prec_coshf(float x)
{
    return (float) _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_FLOAT);
}

// Take the hyperbolic cosine of x.
long double _prec_coshl(long double x)
{
    return _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_COS, _EXACT_TYPE_LDOUBLE);
}

// Take the hyperbolic sine of x.
double _prec_sinh(double x)
{
    return (double) _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_DOUBLE);
}

// Take the hyperbolic sine of x.
float _prec_sinhf(float x)
{
    return (float) _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_FLOAT);
}

// Take the hyperbolic sine of x.
long double _prec_sinhl(long double x)
{
    return _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_SIN, _EXACT_TYPE_LDOUBLE);
}

// Take the hyperbolic tangent of x.
double _prec_tanh(double x)
{
    return (double) _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_DOUBLE);
}

// Take the hyperbolic tangent of x.
float _prec_tanhf(float x)
{
    return (float) _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_FLOAT);
}

// Take the hyperbolic tangent of x.
long double _prec_tanhl(long double x)
{
    return _Prec_Math_TakeHyperbolic(x, _PREC_MATH_TRIG_TAN, _EXACT_TYPE_LDOUBLE);
}

// Take e to x.
double _prec_exp(double x)
{
    return (double) _Prec_Math_TakeExp(x, _PREC_MATH_BASE_E, _EXACT_TYPE_DOUBLE);
}

// Take e to x.
float _prec_expf(float x)
{
    return (float) _Prec_Math_TakeExp(x, _PREC_MATH_BASE_E, _EXACT_TYPE_FLOAT);
}

// Take e to x.
long double _prec_expl(long double x)
{
    return _Prec_Math_TakeExp(x, _PREC_MATH_BASE_E, _EXACT_TYPE_LDOUBLE);
}

// Take 2 to x.
double _prec_exp2(double x)
{
    return (double) _Prec_Math_TakeExp(x, _PREC_MATH_BASE_2, _EXACT_TYPE_DOUBLE);
}

// Take 2 to x.
float _prec_exp2f(float x)
{
    return (float) _Prec_Math_TakeExp(x, _PREC_MATH_BASE_2, _EXACT_TYPE_FLOAT);
}

// Take 2 to x.
long double _prec_exp2l(long double x)
{
    return _Prec_Math_TakeExp(x, _PREC_MATH_BASE_2, _EXACT_TYPE_LDOUBLE);
}

// Take e to x, less 1.
double _prec_expm1(double x)
{
    return (double) _Prec_Math_TakeExpm1(x, _EXACT_TYPE_DOUBLE);
}

// Take e to x, less 1.
float _prec_expm1f(float x)
{
    return (float) _Prec_Math_TakeExpm1(x, _EXACT_TYPE_FLOAT);
}

// Take e to x, less 1.
long double _prec_expm1l(long double x)
{
    return _Prec_Math_TakeExpm1(x, _EXACT_TYPE_LDOUBLE);
}

// Take the natural logarithm of x.
double _prec_log(double x)
{
    return (double) _Prec_Math_TakeLog(x, _PREC_MATH_BASE_E, _EXACT_TYPE_DOUBLE);
}

// Take the natural logarithm of x.
float _prec_logf(float x)
{
    return (float) _Prec_Math_TakeLog(x, _PREC_MATH_BASE_E, _EXACT_TYPE_FLOAT);
}

// Take the natural logarithm of x.
long double _prec_logl(long double x)
{
    return _Prec_Math_TakeLog(x, _PREC_MATH_BASE_E, _EXACT_TYPE_LDOUBLE);
}

// Take the base-10 logarithm of x.
double _prec_log10(double x)
{
    return (double) _Prec_Math_TakeLog(x, _PREC_MATH_BASE_10, _EXACT_TYPE_DOUBLE);
}

// Take the base-10 logarithm of x.
float _prec_log10f(float x)
{
    return (float) _Prec_Math_TakeLog(x, _PREC_MATH_BASE_10, _EXACT_TYPE_FLOAT);
}

// Take the base-10 logarithm of x.
long double _prec_log10l(long double x)
{
    return _Prec_Math_TakeLog(x, _PREC_MATH_BASE_10, _EXACT_TYPE_LDOUBLE);
}

// Take the natural logarithm of 1 plus x.
double _prec_log1p(double x)
{
    return (double) _Prec_Math_TakeLog1p(x, _EXACT_TYPE_DOUBLE);
}

// Take the natural logarithm of 1 plus x.
float _prec_log1pf(float x)
{
    return (float) _Prec_Math_TakeLog1p(x, _EXACT_TYPE_FLOAT);
}

// Take the natural logarithm of 1 plus x.
long double _prec_log1pl(long double x)
{
    return _Prec_Math_TakeLog1p(x, _EXACT_TYPE_LDOUBLE);
}

// Take the base-2 logarithm of x.
double _prec_log2(double x)
{
    return (double) _Prec_Math_TakeLog(x, _PREC_MATH_BASE_2, _EXACT_TYPE_DOUBLE);
}

// Take the base-2 logarithm of x.
float _prec_log2f(float x)
{
    return (float) _Prec_Math_TakeLog(x, _PREC_MATH_BASE_2, _EXACT_TYPE_FLOAT);
}

// Take the base-2 logarithm of x.
long double _prec_log2l(long double x)
{
    return _Prec_Math_TakeLog(x, _PREC_MATH_BASE_2, _EXACT_TYPE_LDOUBLE);
}

// Raise x to the power y.
double _prec_pow(double x, double y)
{
    return (double) _Prec_Math_TakePow(x, y, _EXACT_TYPE_DOUBLE);
}

// Raise x to the power y.
float _prec_powf(float x, float y)
{
    return (float) _Prec_Math_TakePow(x, y, _EXACT_TYPE_FLOAT);
}

// Raise x to the power y.
long double _prec_powl(long double x, long double y)
{
    return _Prec_Math_TakePow(x, y, _EXACT_TYPE_LDOUBLE);
}

// Take the error function of x.
double _prec_erf(double x)
{
    return (double) _Prec_Math_TakeErf(x, _EXACT_TYPE_DOUBLE);
}

// Take the error function of x.
float _prec_erff(float x)
{
    return (float) _Prec_Math_TakeErf(x, _EXACT_TYPE_FLOAT);
}

// Take the error function of x.
long double _prec_erfl(long double x)
{
    return _Prec_Math_TakeErf(x, _EXACT_TYPE_LDOUBLE);
}

// Take the complementary error function of x.
double _prec_erfc(double x)
{
    return (double) _Prec_Math_TakeErfc(x, _EXACT_TYPE_DOUBLE);
}

// Take the complementary error function of x.
float _prec_erfcf(float x)
{
    return (float) _Prec_Math_TakeErfc(x, _EXACT_TYPE_FLOAT);
}

// Take the complementary error function of x.
long double _prec_erfcl(long double x)
{
    return _Prec_Math_TakeErfc(x, _EXACT_TYPE_LDOUBLE);
}

// Take the natural logarithm of the magnitude of gamma of x.
double _prec_lgamma(double x)
{
    return (double) _Prec_Math_TakeLgamma(x, _EXACT_TYPE_DOUBLE);
}

// Take the natural logarithm of the magnitude of gamma of x.
float _prec_lgammaf(float x)
{
    return (float) _Prec_Math_TakeLgamma(x, _EXACT_TYPE_FLOAT);
}

// Take the natural logarithm of the magnitude of gamma of x.
long double _prec_lgammal(long double x)
{
    return _Prec_Math_TakeLgamma(x, _EXACT_TYPE_LDOUBLE);
}

// Take gamma of x.
double _prec_tgamma(double x)
{
    return (double) _Prec_Math_TakeTgamma(x, _EXACT_TYPE_DOUBLE);
}

// Take gamma of x.
float _prec_tgammaf(float x)
{
    return (float) _Prec_Math_TakeTgamma(x, _EXACT_TYPE_FLOAT);
}

// Take gamma of x.
long double _prec_tgammal(long double x)
{
    return _Prec_Math_TakeTgamma(x, _EXACT_TYPE_LDOUBLE);
}
