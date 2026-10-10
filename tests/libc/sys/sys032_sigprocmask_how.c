// (Test) Status: 234
// An rt_sigprocmask with a way of changing the mask Linux does not have returns -EINVAL, which the program exits with, on the machine and in the emulator.

#define SYS_RT_SIGPROCMASK 14
#define SYS_EXIT           60

#define SIG_HOW_UNKNOWN 3

long sys(long nr, long a, long b, long c, long d);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret");
#endif

int main(void)
{
    unsigned long set = 0;

    sys(SYS_EXIT, sys(SYS_RT_SIGPROCMASK, SIG_HOW_UNKNOWN, (long) &set, 0, 8), 0, 0, 0);
    return 1;
}
