// (Test) Status: 231
// An ioctl TCGETS of standard input, a file, returns -ENOTTY, which the program exits with, on the machine and in the emulator.

#define SYS_IOCTL 16
#define SYS_EXIT  60

#define TCGETS 0x5401

long sys(long nr, long a, long b, long c);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx\n"
         "syscall\n"
         "ret");
#endif

int main(void)
{
    unsigned char buf[64];

    sys(SYS_EXIT, sys(SYS_IOCTL, 0, TCGETS, (long) buf), 0, 0);
    return 1;
}
