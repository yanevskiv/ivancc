// (Test) Status: 136
// A 32-bit idivl of INT_MIN by -1 overflows the 32-bit quotient and kills
// the program with SIGFPE, though the same values fit a 64-bit one.

int main(void)
{
#ifdef __x86_64__
    __asm__ ("movl $-2147483648, %eax\n"
             "movl $-1, %ecx\n"
             ".byte 0x99 # cltd\n"
             ".byte 0xf7, 0xf9 # idivl %ecx");
#endif
    return 0;
}
