// (Test) Status: 0
// An unmasked x87 exception reaches a SIGFPE handler as Linux sends it: at
// the next waiting instruction, not the one that raised it, past the no-wait
// ones between, with the si_code of its flag, trap 16 and the status word
// as the fault left it, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13

#define SA_SIGINFO  0x4
#define SA_RESTORER 0x04000000

#define SIGFPE 8

// Where a handler finds what it reads: siginfo's si_code, the ucontext's
// trap number, %rip and FXSAVE area, and the status word in that area.
#define SI_CODE    8
#define UC_RIP     168
#define UC_TRAPNO  200
#define UC_FPSTATE 224
#define FX_FSW     2

// The status word's TOP, which the checks leave out, and the flags a handler
// clears so the waiting instruction runs when it returns: the exceptions, SF,
// ES and B.
#define FSW_TOP   0x3800
#define FSW_CLEAR 0x80FF

// Linux's struct sigaction, as rt_sigaction reads it.
struct sigaction_linux {
    void (*handler)(int, void *, void *);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

// An x87 register's bits: the significand, then the sign and exponent.
struct x80 {
    unsigned long lo;
    unsigned long hi;
};

long sys(long nr, long a, long b, long c, long d);
void restorer(void);
void divide_fwait(unsigned fcw, const struct x80 *a, const struct x80 *b);
void divide_fchs(unsigned fcw, const struct x80 *a, const struct x80 *b);
extern char at_fwait[];
extern char at_fchs[];

int trap_code;
unsigned long trap_no;
unsigned long trap_rip;
unsigned trap_fsw;

// Divide %st(1) = *a by %st = *b under fcw, then run the no-wait FNSTSW and
// FNSTCW and a waiting INSN at LABEL; empty the stack for the next call.
#define X87_DIVIDE(NAME, LABEL, INSN) \
    ".globl " NAME "\n"               \
    NAME ": subq $40, %rsp\n"         \
    "fnstcw 32(%rsp)\n"               \
    "movl %edi, 36(%rsp)\n"           \
    "fldcw 36(%rsp)\n"                \
    "fldt (%rsi)\n"                   \
    "fldt (%rdx)\n"                   \
    "fdivrp %st, %st(1)\n"            \
    "fnstsw 28(%rsp)\n"               \
    "fnstcw 28(%rsp)\n"               \
    ".globl " LABEL "\n"              \
    LABEL ": " INSN "\n"              \
    "fnclex\n"                        \
    "fnstenv (%rsp)\n"                \
    "movq $0, %rdi\n"                 \
    "movl %edi, 4(%rsp)\n"            \
    "movq $0xFFFF, %rdi\n"            \
    "movl %edi, 8(%rsp)\n"            \
    "fldenv (%rsp)\n"                 \
    "fldcw 32(%rsp)\n"                \
    "addq $40, %rsp\n"                \
    "ret\n"

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret\n"
         ".globl restorer\n"
         "restorer: movq $15, %rax # rt_sigreturn\n"
         "syscall\n"
         X87_DIVIDE("divide_fwait", "at_fwait", "fwait")
         X87_DIVIDE("divide_fchs", "at_fchs", "fchs"));
#endif

// Record what the fault left and clear the status word's flags, so the
// waiting instruction runs when the handler returns.
void on_fpe(int sig, void *info, void *context)
{
    unsigned char *uc = context;
    unsigned char *fpstate = *(unsigned char **) (uc + UC_FPSTATE);

    (void) sig;
    trap_code = *(int *) ((unsigned char *) info + SI_CODE);
    trap_no = *(unsigned long *) (uc + UC_TRAPNO);
    trap_rip = *(unsigned long *) (uc + UC_RIP);
    trap_fsw = *(unsigned short *) (fpstate + FX_FSW);
    *(unsigned short *) (fpstate + FX_FSW) &= (unsigned short) ~FSW_CLEAR;
}

// One trapping division, where it should trap and what its handler should see.
struct check {
    void (*divide)(unsigned, const struct x80 *, const struct x80 *);
    char *at;
    unsigned fcw;
    struct x80 a;
    struct x80 b;
    int code;
    unsigned fsw;
};

int main(void)
{
    static const struct check checks[] = {
        {divide_fwait, at_fwait, 0x037B, {0x8000000000000000, 0x3FFF}, {0x0000000000000000, 0x0000}, 3, 0x8084},
        {divide_fwait, at_fwait, 0x037E, {0x0000000000000000, 0x0000}, {0x0000000000000000, 0x0000}, 7, 0x8081},
        {divide_fwait, at_fwait, 0x035F, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 6, 0x82A0},
        {divide_fwait, at_fwait, 0x036F, {0x8000000000000000, 0x0001}, {0x8000000000000000, 0x4000}, 5, 0x8090},
        {divide_fwait, at_fwait, 0x0377, {0xFFFFFFFFFFFFFFFF, 0x7FFE}, {0x8000000000000000, 0x3FFE}, 4, 0x8088},
        {divide_fwait, at_fwait, 0x037D, {0x8000000000000000, 0x3FFF}, {0x0000000000000001, 0x0000}, 5, 0x8082},
        {divide_fchs,  at_fchs,  0x037B, {0x8000000000000000, 0x3FFF}, {0x0000000000000000, 0x0000}, 3, 0x8084}
    };
    struct sigaction_linux act = {0};

    act.handler = on_fpe;
    act.flags = SA_SIGINFO | SA_RESTORER;
    act.restorer = restorer;
    if (sys(SYS_RT_SIGACTION, SIGFPE, (long) &act, 0, 8) != 0) {
        return 100;
    }
    for (int i = 0; i < (int) (sizeof(checks) / sizeof(checks[0])); i++) {
        checks[i].divide(checks[i].fcw, &checks[i].a, &checks[i].b);
        if (trap_no != 16 || trap_rip != (unsigned long) checks[i].at) {
            return 1 + i;
        }
        if (trap_code != checks[i].code || (trap_fsw & ~FSW_TOP) != checks[i].fsw) {
            return 10 + i;
        }
        trap_code = 0;
        trap_no = 0;
        trap_rip = 0;
    }
    return 0;
}
