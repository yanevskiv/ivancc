// (Test) Status: 0
// A handler enters with the signal in %rdi, the siginfo in %rsi and the ucontext in %rdx, on a frame Linux lays out under the stack, with the SSE registers cleared, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13
#define SYS_GETPID       39
#define SYS_KILL         62

#define SA_SIGINFO  0x4
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
void on_signal(int sig);

static volatile int bad = 99;
static long pid;
long xmm_at_entry[2];

// Check what the handler entered with, the frame's address after the rest.
void check(long sig, int *info, unsigned long *uc, unsigned long *frame)
{
    unsigned long *sc = uc + 5;

    if (sig != SIGUSR1 || (unsigned long) frame % 16 != 8) {
        bad = 1;
    } else if (frame[0] != (unsigned long) restorer || (unsigned long *) info != frame + 39 || uc != frame + 1) {
        bad = 2;
    } else if (info[0] != SIGUSR1 || info[1] != 0 || info[2] != 0 || info[4] != pid) {
        bad = 3;
    } else if (uc[1] != 0 || uc[2] != 0 || uc[4] != 0 || uc[37] != 0) {
        bad = 4;
    } else if (sc[8] != (unsigned long) pid || sc[9] != SIGUSR1 || sc[13] != 0 || (sc[18] & 0xFFFF) != 0x33 || sc[18] >> 48 != 0x2b) {
        bad = 5;
    } else if (xmm_at_entry[0] != 0 || xmm_at_entry[1] != 0) {
        bad = 7;
    } else if (sc[15] <= (unsigned long) frame || sc[23] <= (unsigned long) frame || sc[23] >= sc[15] || sc[23] % 64 != 0) {
        bad = 6;
    } else {
        bad = 0;
    }
}

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret\n"
         ".globl restorer\n"
         "restorer: movq $15, %rax # rt_sigreturn\n"
         "syscall\n"
         ".globl on_signal\n"
         "on_signal: leaq xmm_at_entry(%rip), %rax\n"
         "movq %xmm0, %rcx\n"
         "movq %rcx, (%rax)\n"
         "movq %xmm15, %rcx\n"
         "movq %rcx, 8(%rax)\n"
         "movq %rsp, %rcx\n"
         "subq $8, %rsp\n"
         "call check\n"
         "addq $8, %rsp\n"
         "ret");
#endif

int main(void)
{
    struct sigaction_linux act = {0};

    act.handler = on_signal;
    act.flags = SA_SIGINFO | SA_RESTORER;
    act.restorer = restorer;
    if (sys(SYS_RT_SIGACTION, SIGUSR1, (long) &act, 0, 8) != 0) {
        return 100;
    }
    pid = sys(SYS_GETPID, 0, 0, 0, 0);
#ifdef __x86_64__
    __asm__ (
        "movq $-1, %rax\n"
        "movq %rax, %xmm0\n"
        "movq %rax, %xmm15\n"
    );
#endif
    if (sys(SYS_KILL, pid, SIGUSR1, 0, 0) != 0) {
        return 101;
    }
    return bad;
}
