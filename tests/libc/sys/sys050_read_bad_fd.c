// (Test) Status: 247
// A read of a descriptor that is not open returns -EBADF, which the program exits with, on the machine and in the emulator.

#define SYS_READ 0
#define SYS_EXIT 60

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
    char buf[4];

    sys(SYS_EXIT, sys(SYS_READ, CLOSED, (long) buf, sizeof(buf)), 0, 0);
    return 1;
}
