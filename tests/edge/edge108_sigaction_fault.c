// (Test) Status: 242
// An rt_sigaction of signal 99 from an unmapped act returns -EFAULT before -EINVAL, which the program exits with, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13
#define SYS_EXIT         60

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
    sys(SYS_EXIT, sys(SYS_RT_SIGACTION, 99, 8, 0, 8), 0, 0, 0);
    return 1;
}
