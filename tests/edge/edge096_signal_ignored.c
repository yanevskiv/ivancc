// (Test) Status: 0
// A kill of a signal the program ignores returns 0 and changes nothing, though the signal would end it by default, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13
#define SYS_GETPID       39
#define SYS_KILL         62

#define SIG_IGN 1

#define SA_RESTORER 0x04000000

#define SIGTERM 15

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

// Send the program a signal.
static long kill_self(int sig)
{
    return sys(SYS_KILL, sys(SYS_GETPID, 0, 0, 0, 0), sig, 0, 0);
}

int main(void)
{
    struct sigaction_linux act = {0};

    act.handler = (void (*)(int)) SIG_IGN;
    if (sys(SYS_RT_SIGACTION, SIGTERM, (long) &act, 0, 8) != 0) {
        return 1;
    }
    if (kill_self(SIGTERM) != 0) {
        return 2;
    }
    return 0;
}
