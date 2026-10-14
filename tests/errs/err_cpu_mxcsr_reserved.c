// (Test) Status: 139
// (Test) Emulator error: [ERR_CPU_MXCSR_RESERVED]
// Can't load MXCSR with a reserved bit set.
// Note: the hardware raises #GP, which Linux sends as SIGSEGV.

unsigned mxcsr_reserved = 0x10000;

int main(void)
{
#ifdef __x86_64__
    __asm__ ("leaq mxcsr_reserved(%rip), %rax\n"
             "ldmxcsr (%rax)");
#endif
    return 0;
}
