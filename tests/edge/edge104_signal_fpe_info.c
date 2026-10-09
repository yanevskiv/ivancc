// (Test) Status: 0
// A division by zero delivers SIGFPE with the idiv's address in si_addr and FPE_INTDIV, and its handler may resume elsewhere, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13

#define SA_SIGINFO  0x4
#define SA_RESTORER 0x04000000

#define SIGFPE 8

// Linux's struct sigaction, as rt_sigaction reads it.
struct sigaction_linux {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

long sys(long nr, long a, long b, long c, long d);
void restorer(void);
void divider(void);
void divide(void);
void resume(void);

static volatile int bad = 99;

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret\n"
         ".globl restorer\n"
         "restorer: movq $15, %rax # rt_sigreturn\n"
         "syscall\n"
         ".globl divider\n"
         "divider: movq $0, %rcx\n"
         "movq $1, %rax\n"
         "cqo\n"
         ".globl divide\n"
         "divide: idivq %rcx\n"
         ".globl resume\n"
         "resume: ret");
#endif

// Check the fault's siginfo and sigcontext, and resume past the division.
static void on_fpe(int sig, int *info, unsigned long *uc)
{
    unsigned long *sc = uc + 5;

    if (sig != SIGFPE || info[0] != SIGFPE || info[2] != 1 || *(void **) (info + 4) != (void *) divide) {
        bad = 1;
    } else if (sc[16] != (unsigned long) divide || sc[20] != 0) {
        bad = 2;
    } else {
        bad = 0;
    }
    sc[16] = (unsigned long) resume;
}

int main(void)
{
    struct sigaction_linux act = {0};

    act.handler = (void (*)(int)) on_fpe;
    act.flags = SA_SIGINFO | SA_RESTORER;
    act.restorer = restorer;
    if (sys(SYS_RT_SIGACTION, SIGFPE, (long) &act, 0, 8) != 0) {
        return 100;
    }
    divider();
    return bad;
}
