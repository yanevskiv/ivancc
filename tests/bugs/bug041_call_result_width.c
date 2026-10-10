// (Test) Status: 0
// bug041. A caller took all of %rax as the value of a call returning an integer
// narrower than 64 bits, where the SysV ABI leaves the bits past its width to
// the callee. A callee from gcc that leaves them set, as each one below does by
// returning its argument whole, read as another value: `as_int(0x1fffffffe)`
// compared as 8589934590, not -2.

_Bool as_bool(long x);
signed char as_schar(long x);
unsigned char as_uchar(long x);
short as_short(long x);
unsigned short as_ushort(long x);
int as_int(long x);
unsigned int as_uint(long x);
long as_long(long x);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl as_bool\n"
         ".globl as_schar\n"
         ".globl as_uchar\n"
         ".globl as_short\n"
         ".globl as_ushort\n"
         ".globl as_int\n"
         ".globl as_uint\n"
         ".globl as_long\n"
         "as_bool:\n"
         "as_schar:\n"
         "as_uchar:\n"
         "as_short:\n"
         "as_ushort:\n"
         "as_int:\n"
         "as_uint:\n"
         "as_long:\n"
         "mov %rdi, %rax\n"
         "ret");
#endif

long global;

// Take the value of an int back as a long.
long widen(long x)
{
    return x;
}

int main(void)
{
    int (*to_int)(long) = as_int;
    unsigned char (*to_uchar)(long) = as_uchar;
    long local;

    if (! (as_int(0x1fffffffeL) == -2) || ! (as_int(0x1fffffffeL) < 0)) return 1;
    if (! (to_int(0x1fffffffeL) < 0) || to_int(0x300000007L) != 7) return 2;
    if (as_uint(0x100000005L) != 5 || (long) as_uint(-0xfffffff9L) != 7) return 3;
    if (as_short(0x1fffeL) != -2 || as_ushort(0x1fffeL) != 0xfffe) return 4;
    if (as_schar(0x1ffL) != -1 || as_uchar(0x1ffL) != 0xff || to_uchar(0x2feL) != 0xfe) return 5;
    if (as_bool(0x100L) != 0 || as_bool(0x7fffff01L) != 1) return 6;
    local = as_int(0xfffffffeL);
    global = as_short(0xffffL);
    if (local != -2 || global != -1) return 7;
    if (as_int(0x1fffffffeL) + 1L != -1 || as_uint(0x1fffffffeL) + 2L != 0x100000000L) return 8;
    if (widen(as_int(0x1fffffffeL)) != -2 || widen(as_uchar(-1L)) != 255) return 9;
    if (as_long(0x1fffffffeL) != 0x1fffffffeL) return 10;
    return 0;
}
