// (Test) Status: 247
// An ioctl other than TCGETS of a file returns -ENOTTY, and of a descriptor that is not open -EBADF, which the program exits with, on the machine and in the emulator.

#define SYS_IOCTL 16
#define SYS_EXIT  60

#define TIOCGWINSZ 0x5413

#define ENOTTY 25

#define CLOSED 99

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

    if (sys(SYS_IOCTL, 0, TIOCGWINSZ, (long) buf) != -ENOTTY) {
        return 1;
    }
    sys(SYS_EXIT, sys(SYS_IOCTL, CLOSED, TIOCGWINSZ, (long) buf), 0, 0);
    return 2;
}
