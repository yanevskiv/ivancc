// (Test) Status: 42
// Width suffixes on the forms the encoder has, each agreeing with its
// register: stores of 8, 16 and 32 bits, a 16-bit load, sign- and
// zero-extending loads, a 32-bit move that clears the upper half, and a
// shift by %cl.

void store8(unsigned long *p, long v);
void store16(unsigned long *p, long v);
void store32(unsigned long *p, long v);
long load16(const unsigned long *p);
long load_s8(const unsigned long *p);
long load_z16(const unsigned long *p);
long load_s32(const unsigned long *p);
unsigned long clear_high(unsigned long v);
long shift(long v, long n);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl store8\n"
         "store8: movb %sil, (%rdi)\n"
         "ret\n"
         ".globl store16\n"
         "store16: movw %si, (%rdi)\n"
         "ret\n"
         ".globl store32\n"
         "store32: movl %esi, (%rdi)\n"
         "ret\n"
         ".globl load16\n"
         "load16: movq $-1, %rax\n"
         "movw (%rdi), %ax\n"
         "ret\n"
         ".globl load_s8\n"
         "load_s8: movsbq (%rdi), %rax\n"
         "ret\n"
         ".globl load_z16\n"
         "load_z16: movzwq (%rdi), %rax\n"
         "ret\n"
         ".globl load_s32\n"
         "load_s32: movslq (%rdi), %rax\n"
         "ret\n"
         ".globl clear_high\n"
         "clear_high: movq %rdi, %rax\n"
         "movl %eax, %eax\n"
         "ret\n"
         ".globl shift\n"
         "shift: movq %rdi, %rax\n"
         "movq %rsi, %rcx\n"
         "shlq %cl, %rax\n"
         "ret");
#endif

int main(void)
{
    unsigned long word = 0;

    store8(&word, 0x1ff);
    if (word != 0xff) {
        return 1;
    }
    store16(&word, 0x12345678);
    if (word != 0x5678) {
        return 2;
    }
    word = ~0UL;
    store32(&word, 0x12345678);
    if (word != 0xffffffff12345678UL) {
        return 3;
    }
    word = 0x80;
    if (load_s8(&word) != -128) {
        return 4;
    }
    word = 0xfffe;
    if (load_z16(&word) != 65534) {
        return 5;
    }
    word = 0xfffffffe;
    if (load_s32(&word) != -2) {
        return 6;
    }
    if (clear_high(~0UL) != 0xffffffffUL) {
        return 7;
    }
    if (shift(3, 4) != 48) {
        return 8;
    }
    word = 0x1234;
    if ((unsigned long) load16(&word) != 0xffffffffffff1234UL) {
        return 9;
    }
    return 42;
}
