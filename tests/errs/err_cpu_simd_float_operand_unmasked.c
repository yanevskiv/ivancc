// (Test) Status: 136
// (Test) Emulator error: [ERR_CPU_SIMD_FLOAT_OPERAND_UNMASKED]
// Can't run an SSE operation whose operands raise an unmasked exception.
// Note: the hardware raises #XF, which Linux sends as SIGFPE.

unsigned zero_divide_unmasked = 0x1D80;
unsigned long one = 0x3FF0000000000000;

int main(void)
{
#ifdef __x86_64__
    __asm__ ("leaq zero_divide_unmasked(%rip), %rax\n"
             "ldmxcsr (%rax)\n"
             "leaq one(%rip), %rax\n"
             "movq (%rax), %rdx\n"
             "movq %rdx, %xmm0\n"
             "movq $0, %rdx\n"
             "movq %rdx, %xmm1\n"
             "divsd %xmm1, %xmm0");
#endif
    return 0;
}
