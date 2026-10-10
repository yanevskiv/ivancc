// (Test) Status: 10
// A handler rt_sigaction installs runs when the program kills itself, and returns to the kill through its restorer, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13
#define SYS_GETPID       39
#define SYS_KILL         62

#define SA_RESTORER 0x04000000

#define SIGUSR1 10

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

static volatile int got;

static void on_signal(int sig)
{
    got = sig;
}

int main(void)
{
    struct sigaction_linux act = {0};

    act.handler = on_signal;
    act.flags = SA_RESTORER;
    act.restorer = restorer;
    if (sys(SYS_RT_SIGACTION, SIGUSR1, (long) &act, 0, 8) != 0) {
        return 1;
    }
    if (sys(SYS_KILL, sys(SYS_GETPID, 0, 0, 0, 0), SIGUSR1, 0, 0) != 0) {
        return 2;
    }
    return got;
}
