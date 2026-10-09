// (Test) Status: 0
// A read of unmapped memory delivers SIGSEGV with its address in si_addr and cr2, SEGV_MAPERR and the #PF's vector and error code, and its handler may resume elsewhere, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13

#define SA_SIGINFO  0x4
#define SA_RESTORER 0x04000000

#define SIGSEGV 11

// Linux's struct sigaction, as rt_sigaction reads it.
struct sigaction_linux {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

long sys(long nr, long a, long b, long c, long d);
void restorer(void);
void reader(void);
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
         ".globl reader\n"
         "reader: movq $16, %rax\n"
         "movq (%rax), %rax\n"
         ".globl resume\n"
         "resume: ret");
#endif

// Check the fault's siginfo and sigcontext, and resume past the read.
static void on_segv(int sig, int *info, unsigned long *uc)
{
    unsigned long *sc = uc + 5;

    if (sig != SIGSEGV || info[0] != SIGSEGV || info[2] != 1 || *(unsigned long *) (info + 4) != 16) {
        bad = 1;
    } else if (sc[19] != 4 || sc[20] != 14 || sc[22] != 16) {
        bad = 2;
    } else if (sc[16] != (unsigned long) resume - 3) {
        bad = 3;
    } else {
        bad = 0;
    }
    sc[16] = (unsigned long) resume;
}

int main(void)
{
    struct sigaction_linux act = {0};

    act.handler = (void (*)(int)) on_segv;
    act.flags = SA_SIGINFO | SA_RESTORER;
    act.restorer = restorer;
    if (sys(SYS_RT_SIGACTION, SIGSEGV, (long) &act, 0, 8) != 0) {
        return 100;
    }
    reader();
    return bad;
}
