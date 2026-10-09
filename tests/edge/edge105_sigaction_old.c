// (Test) Status: 0
// rt_sigaction returns the disposition it replaces, less the flags Linux does not know and SIGKILL and SIGSTOP in the mask, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13

#define SA_RESTORER 0x04000000
#define SA_UNKNOWN  0x100

#define SIGKILL 9
#define SIGUSR1 10
#define SIGSTOP 19

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
    struct sigaction_linux old = {0};

    act.handler = on_signal;
    act.flags = SA_RESTORER | SA_UNKNOWN;
    act.restorer = restorer;
    act.mask = ~0UL;
    if (sys(SYS_RT_SIGACTION, SIGUSR1, (long) &act, (long) &old, 8) != 0) {
        return 1;
    }
    if (old.handler != 0 || old.flags != 0 || old.restorer != 0 || old.mask != 0) {
        return 2;
    }
    if (sys(SYS_RT_SIGACTION, SIGUSR1, 0, (long) &old, 8) != 0) {
        return 3;
    }
    if (old.handler != on_signal || old.flags != SA_RESTORER || old.restorer != restorer) {
        return 4;
    }
    if (old.mask != ~((1UL << (SIGKILL - 1)) | (1UL << (SIGSTOP - 1)))) {
        return 5;
    }
    return 0;
}
