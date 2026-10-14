// (Test) Status: 136
// (Test) Emulator error: [ERR_CPU_X87_FLOAT_UNMASKED]
// Can't run an x87 instruction past an unmasked exception pending.
// Note: the hardware raises #MF at the FWAIT, which Linux sends as SIGFPE.

unsigned short divide_unmasked = 0x037B;
unsigned long one[2] = {0x8000000000000000, 0x3FFF};
unsigned long zero[2] = {0x0000000000000000, 0x0000};

int main(void)
{
#ifdef __x86_64__
    __asm__ ("leaq divide_unmasked(%rip), %rax\n"
             "fldcw (%rax)\n"
             "leaq one(%rip), %rax\n"
             "fldt (%rax)\n"
             "leaq zero(%rip), %rax\n"
             "fldt (%rax)\n"
             "fdivrp %st, %st(1)\n"
             "fwait");
#endif
    return 0;
}
