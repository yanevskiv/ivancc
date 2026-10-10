// (Test) Status: 139
// A signal whose frame does not fit under the stack ends the program by SIGSEGV instead of running its handler, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13

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

static void on_signal(int sig)
{
    (void) sig;
}

// Install a handler for sig with flags, blocking mask while it runs.
static long install(int sig, void (*handler)(int), unsigned long flags, unsigned long mask)
{
    struct sigaction_linux act = {0};

    act.handler = handler;
    act.flags = flags | SA_RESTORER;
    act.restorer = restorer;
    act.mask = mask;
    return sys(SYS_RT_SIGACTION, sig, (long) &act, 0, 8);
}

int main(void)
{
    if (install(SIGUSR1, on_signal, 0, 0) != 0) {
        return 1;
    }
#ifdef __x86_64__
    __asm__ (
        "movq $39, %rax # getpid\n"
        "syscall\n"
        "movq %rax, %rdi\n"
        "movq $10, %rsi # SIGUSR1\n"
        "movq $64, %rsp\n"
        "movq $62, %rax # kill\n"
        "syscall\n"
    );
#endif
    return 2;
}
