// (Test) Status: 200
// Constant expressions under the usual arithmetic conversions. Each value is
// folded while parsing, as an enumerator or an array length, and again after
// analysis, as a static initializer, then checked against gcc's result.

enum Promoted {
    P_NOT_UCHAR  = ~(unsigned char) 0,
    P_NEG_USHORT = -(unsigned short) 1,
    P_CHAR_SUM   = (signed char) -128 + (unsigned char) 255,
    P_BOOL_SUM   = (_Bool) 5 + (_Bool) 0 + 1,
    P_SHIFT_CHAR = (unsigned char) 128 << 1
};

enum Mixed {
    M_LT_UINT    = -1 < 1u,
    M_LE_UINT    = 0u <= -1,
    M_GT_ULONG   = -1 > 0ul,
    M_GE_LONG    = -1L >= 0u,
    M_EQ_UINT    = -1 == 4294967295u,
    M_NE_LONG    = -1L != 4294967295u,
    M_DIV_UINT   = -6 / 2u == 2147483645u,
    M_MOD_UINT   = -7 % 4u,
    M_SHR_UINT   = (int) (0x80000000u >> 31),
    M_SHR_INT    = -8 >> 1,
    M_COND_UINT  = (1 ? -1 : 0u) > 0,
    M_COND_LONG  = (0 ? 0u : -1L) < 0,
    M_AND_WIDE   = 0x100000000L && 1,
    M_CAST_CHAR  = (signed char) 0x1ff,
    M_CAST_SHORT = (unsigned short) -1
};

char len_uint[-1 < 1u ? 1 : 7];
char len_wrap[(0u - 1) / 0x10000000u];
char len_cond[(1 ? -1 : 0u) > 0 ? 9 : 1];

unsigned long s_ulong_wrap = 0ul - 1;
unsigned int s_uint_wrap = 0u - 2;
unsigned int s_uint_mul = 0x10000u * 0x10000u + 3;
long s_long_mix = -1L + 4294967295u;
long s_long_neg = -(long) 0x80000000u;
long s_int_shift = (int) 0x80000000u >> 31;
unsigned long s_ulong_shift = 0xffffffffffffffffUL >> 63;
unsigned long s_cond_widen = 0 ? 1ul : -1;
int s_not_wide = ! 0x100000000L;
int s_div_neg = -7 / 2;
int s_mod_neg = -7 % 2;

int check_enums(void)
{
    if (P_NOT_UCHAR != -1 || P_NEG_USHORT != -1 || P_CHAR_SUM != 127) return 1;
    if (P_BOOL_SUM != 2 || P_SHIFT_CHAR != 256) return 2;
    if (M_LT_UINT != 0 || M_LE_UINT != 1 || M_GT_ULONG != 1 || M_GE_LONG != 0) return 3;
    if (M_EQ_UINT != 1 || M_NE_LONG != 1 || M_DIV_UINT != 1 || M_MOD_UINT != 1) return 4;
    if (M_SHR_UINT != 1 || M_SHR_INT != -4 || M_COND_UINT != 1 || M_COND_LONG != 1) return 5;
    if (M_AND_WIDE != 1 || M_CAST_CHAR != -1 || M_CAST_SHORT != 65535) return 6;
    return 0;
}

int check_lengths(void)
{
    if (sizeof(len_uint) != 7 || sizeof(len_wrap) != 15 || sizeof(len_cond) != 9) return 11;
    return 0;
}

int check_statics(void)
{
    if (s_ulong_wrap != 0xffffffffffffffffUL || s_uint_wrap != 4294967294u) return 21;
    if (s_uint_mul != 3 || s_long_mix != 4294967294L || s_long_neg != -2147483648L) return 22;
    if (s_int_shift != -1 || s_ulong_shift != 1 || s_cond_widen != 0xffffffffffffffffUL) return 23;
    if (s_not_wide != 0 || s_div_neg != -3 || s_mod_neg != -1) return 24;
    return 0;
}

int main()
{
    int r;

    if ((r = check_enums()) != 0) return r;
    if ((r = check_lengths()) != 0) return r;
    if ((r = check_statics()) != 0) return r;
    return 200;
}
