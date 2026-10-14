// (Test) Status: 42
// Scalar SSE arithmetic in file-scope assembly rounds in MXCSR's mode and
// sets its flags: ties, a sum below the last bit, an overflow and an exact
// zero difference in each of the four modes, on the machine and in the
// emulator.

unsigned long op_addsd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_subsd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_mulsd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_divss(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);

// Run INSN on a and b under mxcsr, give the result and MXCSR after.
#define SSE_OP(NAME, INSN)            \
    ".globl " NAME "\n"               \
    NAME ": subq $8, %rsp\n"          \
    "stmxcsr (%rsp)\n"                \
    "movl %edi, 4(%rsp)\n"            \
    "ldmxcsr 4(%rsp)\n"               \
    "movq %rsi, %xmm0\n"              \
    "movq %rdx, %xmm1\n"              \
    INSN " %xmm1, %xmm0\n"            \
    "stmxcsr (%rcx)\n"                \
    "ldmxcsr (%rsp)\n"                \
    "movq %xmm0, %rax\n"              \
    "addq $8, %rsp\n"                 \
    "ret\n"

#ifdef __x86_64__
__asm__ (".text\n"
         SSE_OP("op_addsd", "addsd")
         SSE_OP("op_subsd", "subsd")
         SSE_OP("op_mulsd", "mulsd")
         SSE_OP("op_divss", "divss"));
#endif

// Every exception masked, and each of the four rounding modes.
#define MASKED 0x1F80
#define NEAREST 0x0000
#define DOWN    0x2000
#define UP      0x4000
#define ZERO    0x6000

// One operation, its operands and what it should give.
struct check {
    unsigned long (*op)(unsigned, unsigned long, unsigned long, unsigned *);
    unsigned mxcsr;
    unsigned long a;
    unsigned long b;
    unsigned long result;
    unsigned after;
};

// Run each check, failing with first plus its index.
static int run(const struct check *checks, int count, int first)
{
    for (int i = 0; i < count; i++) {
        unsigned after = 0;
        unsigned long result = checks[i].op(checks[i].mxcsr, checks[i].a, checks[i].b, &after);

        if (result != checks[i].result || after != checks[i].after) {
            return first + i;
        }
    }
    return 0;
}

// 1 + 2^-53 + 2^-105 should round by the mode, and set PE.
// [Amd64-Vol1:4.2.2]: RC, to nearest, down, up or toward zero
static int check_modes(void)
{
    static const struct check checks[] = {
        {op_addsd, MASKED | NEAREST, 0x3FF0000000000000, 0x3CA0000000000001, 0x3FF0000000000001, 0x1FA0},
        {op_addsd, MASKED | DOWN,    0x3FF0000000000000, 0x3CA0000000000001, 0x3FF0000000000000, 0x3FA0},
        {op_addsd, MASKED | UP,      0x3FF0000000000000, 0x3CA0000000000001, 0x3FF0000000000001, 0x5FA0},
        {op_addsd, MASKED | ZERO,    0x3FF0000000000000, 0x3CA0000000000001, 0x3FF0000000000000, 0x7FA0},
        {op_addsd, MASKED | NEAREST, 0xBFF0000000000000, 0xBCA0000000000001, 0xBFF0000000000001, 0x1FA0},
        {op_addsd, MASKED | DOWN,    0xBFF0000000000000, 0xBCA0000000000001, 0xBFF0000000000001, 0x3FA0},
        {op_addsd, MASKED | UP,      0xBFF0000000000000, 0xBCA0000000000001, 0xBFF0000000000000, 0x5FA0},
        {op_addsd, MASKED | ZERO,    0xBFF0000000000000, 0xBCA0000000000001, 0xBFF0000000000000, 0x7FA0},
        {op_divss, MASKED | NEAREST, 0x3F800000,         0x40400000,         0x3EAAAAAB,         0x1FA0},
        {op_divss, MASKED | ZERO,    0x3F800000,         0x40400000,         0x3EAAAAAA,         0x7FA0}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 1);
}

// A tie should go to the even neighbour, an exact sum set no flag.
// [Amd64-Vol1:4.2.2]: round to nearest, a tie to the even value
static int check_ties(void)
{
    static const struct check checks[] = {
        {op_addsd, MASKED | NEAREST, 0x3FF0000000000000, 0x3CA0000000000000, 0x3FF0000000000000, 0x1FA0},
        {op_addsd, MASKED | NEAREST, 0x3FF0000000000001, 0x3CA0000000000000, 0x3FF0000000000002, 0x1FA0},
        {op_addsd, MASKED | NEAREST, 0x3FF0000000000000, 0x3FF0000000000000, 0x4000000000000000, 0x1F80}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 20);
}

// An overflow should give infinity or the greatest finite number by the mode
// and the sign, and set OE and PE.
// [Amd64-Vol1:4.10.2.6]: OE, a rounded result past the greatest normal number
static int check_overflow(void)
{
    static const struct check checks[] = {
        {op_mulsd, MASKED | NEAREST, 0x7FE0000000000000, 0x4000000000000000, 0x7FF0000000000000, 0x1FA8},
        {op_mulsd, MASKED | DOWN,    0x7FE0000000000000, 0x4000000000000000, 0x7FEFFFFFFFFFFFFF, 0x3FA8},
        {op_mulsd, MASKED | UP,      0x7FE0000000000000, 0x4000000000000000, 0x7FF0000000000000, 0x5FA8},
        {op_mulsd, MASKED | ZERO,    0x7FE0000000000000, 0x4000000000000000, 0x7FEFFFFFFFFFFFFF, 0x7FA8},
        {op_mulsd, MASKED | NEAREST, 0xFFE0000000000000, 0x4000000000000000, 0xFFF0000000000000, 0x1FA8},
        {op_mulsd, MASKED | DOWN,    0xFFE0000000000000, 0x4000000000000000, 0xFFF0000000000000, 0x3FA8},
        {op_mulsd, MASKED | UP,      0xFFE0000000000000, 0x4000000000000000, 0xFFEFFFFFFFFFFFFF, 0x5FA8},
        {op_mulsd, MASKED | ZERO,    0xFFE0000000000000, 0x4000000000000000, 0xFFEFFFFFFFFFFFFF, 0x7FA8}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 30);
}

// x - x should be +0, but -0 when rounding down.
// [Amd64-Vol1:4.2.2]: RC, the direction a result is rounded in
static int check_zero_sign(void)
{
    static const struct check checks[] = {
        {op_subsd, MASKED | NEAREST, 0x3FF0000000000000, 0x3FF0000000000000, 0x0000000000000000, 0x1F80},
        {op_subsd, MASKED | DOWN,    0x3FF0000000000000, 0x3FF0000000000000, 0x8000000000000000, 0x3F80},
        {op_subsd, MASKED | UP,      0x3FF0000000000000, 0x3FF0000000000000, 0x0000000000000000, 0x5F80},
        {op_subsd, MASKED | ZERO,    0x3FF0000000000000, 0x3FF0000000000000, 0x0000000000000000, 0x7F80}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 40);
}

int main(void)
{
    int failed = check_modes();

    failed = failed ? failed : check_ties();
    failed = failed ? failed : check_overflow();
    failed = failed ? failed : check_zero_sign();
    return failed ? failed : 42;
}
