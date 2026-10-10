// (Test) Status: 136
// A div or an idiv of a %rdx:%rax wider than its operand, no C division's, keeps the carry and the borrow between the halves, and a high half not below the divisor kills the program with SIGFPE.

unsigned long divq(unsigned long hi, unsigned long lo, unsigned long d, unsigned long *rem);
long idivq(long hi, long lo, long d, long *rem);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl divq\n"
         "divq: movq %rdx, %r8; movq %rdi, %rdx; movq %rsi, %rax; divq %r8; movq %rdx, (%rcx)\n"
         "ret\n"
         ".globl idivq\n"
         "idivq: movq %rdx, %r8; movq %rdi, %rdx; movq %rsi, %rax; idivq %r8; movq %rdx, (%rcx)\n"
         "ret");
#endif

int main(void)
{
    unsigned long urem = 1;
    long srem = 1;

    if (divq(1, 0, 2, &urem) != 0x8000000000000000UL || urem != 0) {
        return 1;
    }
    if (divq(0xFFFFFFFFFFFFFFFEUL, 1, 0xFFFFFFFFFFFFFFFFUL, &urem) != 0xFFFFFFFFFFFFFFFFUL || urem != 0) {
        return 2;
    }
    if (idivq(-1, 0, 4, &srem) != -4611686018427387904L || srem != 0) {
        return 3;
    }
    if (idivq(-2, -6, 4, &srem) != -4611686018427387905L || srem != -2) {
        return 4;
    }
    divq(2, 0, 2, &urem);
    return 5;
}
