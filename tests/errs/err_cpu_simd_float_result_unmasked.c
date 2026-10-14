// (Test) Status: 136
// (Test) Emulator error: [ERR_CPU_SIMD_FLOAT_RESULT_UNMASKED]
// Can't write an SSE result that raises an unmasked exception.
// Note: the hardware raises #XF, which Linux sends as SIGFPE.

unsigned precision_unmasked = 0x0F80;
unsigned long one = 0x3FF0000000000000;
unsigned long three = 0x4008000000000000;

int main(void)
{
#ifdef __x86_64__
    __asm__ ("leaq precision_unmasked(%rip), %rax\n"
             "ldmxcsr (%rax)\n"
             "leaq one(%rip), %rax\n"
             "movq (%rax), %rdx\n"
             "movq %rdx, %xmm0\n"
             "leaq three(%rip), %rax\n"
             "movq (%rax), %rdx\n"
             "movq %rdx, %xmm1\n"
             "divsd %xmm1, %xmm0");
#endif
    return 0;
}
