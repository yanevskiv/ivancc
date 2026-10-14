// (Test) Status: 42
// Scalar SSE conversions in file-scope assembly round in MXCSR's mode and
// set its flags as the hardware does: between float and double with their
// NaNs and denormals, from an integer, and to one truncated, on the machine
// and in the emulator.

unsigned long op_cvtsd2ss(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_cvtss2sd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_cvtsi2sdq(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_cvtsi2ssq(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_cvttsd2siq(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_cvttsd2sil(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_cvttss2siq(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);

// Run INSN from b into a's register under mxcsr, give it and MXCSR after.
#define SSE_OP(NAME, INSN)            \
    ".globl " NAME "\n"               \
    NAME ": subq $8, %rsp\n"          \
    "stmxcsr (%rsp)\n"                \
    "movl %edi, 4(%rsp)\n"            \
    "ldmxcsr 4(%rsp)\n"               \
    "movq %rsi, %xmm0\n"              \
    "movq %rdx, %xmm1\n"              \
    INSN "\n"                         \
    "stmxcsr (%rcx)\n"                \
    "ldmxcsr (%rsp)\n"                \
    "movq %xmm0, %rax\n"              \
    "addq $8, %rsp\n"                 \
    "ret\n"

// Run INSN from b into %rax under mxcsr, give it and MXCSR after.
#define SSE_TO_INT(NAME, INSN)        \
    ".globl " NAME "\n"               \
    NAME ": subq $8, %rsp\n"          \
    "stmxcsr (%rsp)\n"                \
    "movl %edi, 4(%rsp)\n"            \
    "ldmxcsr 4(%rsp)\n"               \
    "movq %rdx, %xmm1\n"              \
    "movq $0, %rax\n"                 \
    INSN "\n"                         \
    "stmxcsr (%rcx)\n"                \
    "ldmxcsr (%rsp)\n"                \
    "addq $8, %rsp\n"                 \
    "ret\n"

#ifdef __x86_64__
__asm__ (".text\n"
         SSE_OP("op_cvtsd2ss", "cvtsd2ss %xmm1, %xmm0")
         SSE_OP("op_cvtss2sd", "cvtss2sd %xmm1, %xmm0")
         SSE_OP("op_cvtsi2sdq", "cvtsi2sdq %rdx, %xmm0")
         SSE_OP("op_cvtsi2ssq", "cvtsi2ssq %rdx, %xmm0")
         SSE_TO_INT("op_cvttsd2siq", "cvttsd2si %xmm1, %rax")
         SSE_TO_INT("op_cvttsd2sil", ".byte 0xf2, 0x0f, 0x2c, 0xc1 # cvttsd2si %xmm1, %eax")
         SSE_TO_INT("op_cvttss2siq", "cvttss2si %xmm1, %rax"));
#endif

// Every exception masked, DAZ, and two rounding modes.
#define MASKED 0x1F80
#define DAZ    0x0040
#define DOWN   0x2000
#define UP     0x4000

// One conversion, its source and what it should give.
struct check {
    unsigned long (*op)(unsigned, unsigned long, unsigned long, unsigned *);
    unsigned mxcsr;
    unsigned long b;
    unsigned long result;
    unsigned after;
};

// Run each check, failing with first plus its index.
static int run(const struct check *checks, int count, int first)
{
    for (int i = 0; i < count; i++) {
        unsigned after = 0;
        unsigned long result = checks[i].op(checks[i].mxcsr, 0, checks[i].b, &after);

        if (result != checks[i].result || after != checks[i].after) {
            return first + i;
        }
    }
    return 0;
}

// A double should narrow to a float by the mode, overflowing, underflowing,
// and keeping a NaN's top payload bits, quieted.
// [Amd64-Vol4:CVTSD2SS]: a double rounded to a float by MXCSR.RC
static int check_narrow(void)
{
    static const struct check checks[] = {
        {op_cvtsd2ss, MASKED,      0x3FD5555555555555, 0x3EAAAAAB, 0x1FA0},
        {op_cvtsd2ss, MASKED | DOWN, 0x3FD5555555555555, 0x3EAAAAAA, 0x3FA0},
        {op_cvtsd2ss, MASKED,      0x47F0000000000000, 0x7F800000, 0x1FA8},
        {op_cvtsd2ss, MASKED,      0x3690000000000001, 0x00000001, 0x1FB0},
        {op_cvtsd2ss, MASKED,      0x0000000000000001, 0x00000000, 0x1FB2},
        {op_cvtsd2ss, MASKED,      0x7FF8123456789ABC, 0x7FC091A2, 0x1F80},
        {op_cvtsd2ss, MASKED,      0xFFF0123456789ABC, 0xFFC091A2, 0x1F81},
        {op_cvtsd2ss, MASKED,      0x7FF0000000000001, 0x7FC00000, 0x1F81}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 1);
}

// A float should widen exactly, a denormal setting DE, an SNaN quieted.
// [Amd64-Vol4:CVTSS2SD]: a float converted to a double
static int check_widen(void)
{
    static const struct check checks[] = {
        {op_cvtss2sd, MASKED,       0x7F800001, 0x7FF8000020000000, 0x1F81},
        {op_cvtss2sd, MASKED,       0xFFC12345, 0xFFF82468A0000000, 0x1F80},
        {op_cvtss2sd, MASKED,       0x00000001, 0x36A0000000000000, 0x1F82},
        {op_cvtss2sd, MASKED | DAZ, 0x80000001, 0x8000000000000000, 0x1FC0}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 20);
}

// An integer should round by the mode where the format cannot hold it.
// [Amd64-Vol4:CVTSI2SD]: an integer rounded by MXCSR.RC
static int check_from_integer(void)
{
    static const struct check checks[] = {
        {op_cvtsi2sdq, MASKED,        0x7FFFFFFFFFFFFFFF, 0x43E0000000000000, 0x1FA0},
        {op_cvtsi2sdq, MASKED | DOWN, 0x7FFFFFFFFFFFFFFF, 0x43DFFFFFFFFFFFFF, 0x3FA0},
        {op_cvtsi2ssq, MASKED,        0xFFFFFFFFFF000001, 0xCB7FFFFF,         0x1F80},
        {op_cvtsi2ssq, MASKED | UP,   0x0000000001000001, 0x4B800001,         0x5FA0},
        {op_cvtsi2ssq, MASKED,        0x0000000001000001, 0x4B800000,         0x1FA0}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 30);
}

// A value should truncate toward zero whatever the mode; a NaN or one out of
// range give the integer indefinite and IE, a denormal no DE.
// [Amd64-Vol4:CVTTSD2SI]: truncation, and the indefinite integer when invalid
static int check_to_integer(void)
{
    static const struct check checks[] = {
        {op_cvttsd2siq, MASKED,       0x3FF8000000000000, 0x0000000000000001, 0x1FA0},
        {op_cvttsd2siq, MASKED | UP,  0xBFF8000000000000, 0xFFFFFFFFFFFFFFFF, 0x5FA0},
        {op_cvttsd2siq, MASKED,       0x7FF8000000000000, 0x8000000000000000, 0x1F81},
        {op_cvttsd2siq, MASKED,       0x43E0000000000000, 0x8000000000000000, 0x1F81},
        {op_cvttsd2siq, MASKED,       0xC3E0000000000000, 0x8000000000000000, 0x1F80},
        {op_cvttsd2siq, MASKED,       0x0000000000000001, 0x0000000000000000, 0x1FA0},
        {op_cvttsd2siq, MASKED | DAZ, 0x0000000000000001, 0x0000000000000000, 0x1FC0},
        {op_cvttsd2sil, MASKED,       0x41E0000000000000, 0x0000000080000000, 0x1F81},
        {op_cvttsd2sil, MASKED,       0xC1E0000000000000, 0x0000000080000000, 0x1F80},
        {op_cvttsd2sil, MASKED,       0xC1E0000000100000, 0x0000000080000000, 0x1FA0},
        {op_cvttss2siq, MASKED,       0xDF000000,         0x8000000000000000, 0x1F80},
        {op_cvttss2siq, MASKED,       0x5F000000,         0x8000000000000000, 0x1F81}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 40);
}

int main(void)
{
    int failed = check_narrow();

    failed = failed ? failed : check_widen();
    failed = failed ? failed : check_from_integer();
    failed = failed ? failed : check_to_integer();
    return failed ? failed : 42;
}
