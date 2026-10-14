// (Test) Status: 0
// An unmasked SSE exception reaches a SIGFPE handler as Linux sends it: the
// si_code of its flag, trap 19, MXCSR as the fault left it, and the
// destination unwritten, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13

#define SA_SIGINFO  0x4
#define SA_RESTORER 0x04000000

#define SIGFPE 8

// Where a handler finds what it reads: siginfo's si_code, the ucontext's
// trap number, %rip and FXSAVE area, and MXCSR in that area.
#define SI_CODE    8
#define UC_RIP     168
#define UC_TRAPNO  200
#define UC_FPSTATE 224
#define FX_MXCSR   24

// The length of divsd %xmm1, %xmm0, which a handler steps over.
#define DIVSD_LENGTH 4

// Linux's struct sigaction, as rt_sigaction reads it.
struct sigaction_linux {
    void (*handler)(int, void *, void *);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

long sys(long nr, long a, long b, long c, long d);
void restorer(void);
unsigned long divide(unsigned mxcsr, unsigned long a, unsigned long b);

int trap_code;
unsigned long trap_no;
unsigned trap_mxcsr;

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret\n"
         ".globl restorer\n"
         "restorer: movq $15, %rax # rt_sigreturn\n"
         "syscall\n"
         ".globl divide\n"
         "divide: subq $8, %rsp\n"
         "stmxcsr (%rsp)\n"
         "movl %edi, 4(%rsp)\n"
         "ldmxcsr 4(%rsp)\n"
         "movq %rsi, %xmm0\n"
         "movq %rdx, %xmm1\n"
         "divsd %xmm1, %xmm0\n"
         "ldmxcsr (%rsp)\n"
         "movq %xmm0, %rax\n"
         "addq $8, %rsp\n"
         "ret");
#endif

// Record what the fault left, mask every exception and step over divsd.
void on_fpe(int sig, void *info, void *context)
{
    unsigned char *uc = context;
    unsigned char *fpstate = *(unsigned char **) (uc + UC_FPSTATE);

    (void) sig;
    trap_code = *(int *) ((unsigned char *) info + SI_CODE);
    trap_no = *(unsigned long *) (uc + UC_TRAPNO);
    trap_mxcsr = *(unsigned *) (fpstate + FX_MXCSR);
    *(unsigned *) (fpstate + FX_MXCSR) |= 0x1F80;
    *(unsigned long *) (uc + UC_RIP) += DIVSD_LENGTH;
}

// One trapping division and what its handler should see.
struct check {
    unsigned mxcsr;
    unsigned long a;
    unsigned long b;
    int code;
    unsigned after;
};

int main(void)
{
    static const struct check checks[] = {
        {0x1D80, 0x3FF0000000000000, 0x0000000000000000, 3, 0x1D84},
        {0x0F80, 0x3FF0000000000000, 0x4008000000000000, 6, 0x0FA0},
        {0x1780, 0x0010000000000000, 0x4000000000000000, 5, 0x1790},
        {0x1B80, 0x7FE0000000000000, 0x3FE0000000000000, 4, 0x1B88},
        {0x1F00, 0x0000000000000000, 0x0000000000000000, 7, 0x1F01},
        {0x1E80, 0x0000000000000001, 0x3FF0000000000000, 5, 0x1E82}
    };
    struct sigaction_linux act = {0};

    act.handler = on_fpe;
    act.flags = SA_SIGINFO | SA_RESTORER;
    act.restorer = restorer;
    if (sys(SYS_RT_SIGACTION, SIGFPE, (long) &act, 0, 8) != 0) {
        return 100;
    }
    for (int i = 0; i < (int) (sizeof(checks) / sizeof(checks[0])); i++) {
        unsigned long result = divide(checks[i].mxcsr, checks[i].a, checks[i].b);

        if (result != checks[i].a || trap_no != 19) {
            return 1 + i;
        }
        if (trap_code != checks[i].code || trap_mxcsr != checks[i].after) {
            return 10 + i;
        }
        trap_code = 0;
        trap_no = 0;
    }
    return 0;
}
