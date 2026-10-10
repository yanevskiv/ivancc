// (Test) Status: 242
// An ioctl TCGETS of a terminal into an unmapped struct returns -EFAULT, which the program exits with, on the machine and in the emulator.

#define SYS_OPEN  2
#define SYS_IOCTL 16
#define SYS_EXIT  60

#define O_RDWR   02
#define O_NOCTTY 0400

#define TCGETS 0x5401

#define UNMAPPED 8

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
    long fd = sys(SYS_OPEN, (long) "/dev/ptmx", O_RDWR | O_NOCTTY, 0);

    if (fd < 0) {
        return 1;
    }
    sys(SYS_EXIT, sys(SYS_IOCTL, fd, TCGETS, UNMAPPED), 0, 0);
    return 2;
}
