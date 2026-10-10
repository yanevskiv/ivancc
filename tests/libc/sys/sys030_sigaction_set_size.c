// (Test) Status: 234
// An rt_sigaction with a sigset size other than 8 returns -EINVAL, which the program exits with, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13
#define SYS_EXIT         60

#define SA_RESTORER 0x04000000

// Linux's struct sigaction, as rt_sigaction reads it.
struct sigaction_linux {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

long sys(long nr, long a, long b, long c, long d);
void restorer(void);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret\n"
         ".globl restorer\n"
         "restorer: movq $15, %rax # rt_sigreturn\n"
         "syscall");
#endif

static void on_signal(int sig)
{
    (void) sig;
}

int main(void)
{
    struct sigaction_linux act = {0};

    act.handler = on_signal;
    act.flags = SA_RESTORER;
    act.restorer = restorer;
    sys(SYS_EXIT, sys(SYS_RT_SIGACTION, 10, (long) &act, 0, 4), 0, 0, 0);
    return 1;
}
