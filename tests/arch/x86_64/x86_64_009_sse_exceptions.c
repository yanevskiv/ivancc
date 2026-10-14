// (Test) Status: 42
// Scalar SSE arithmetic in file-scope assembly raises MXCSR's flags as the
// hardware does with every exception masked: tininess after rounding, FZ
// and DAZ, a denormal operand, a division by zero, an invalid operation and
// the NaN each gives, and ucomisd's flags, on the machine and in the
// emulator.

unsigned long op_addsd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_mulsd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_divsd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_subsd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_addss(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);
unsigned long op_ucomisd(unsigned mxcsr, unsigned long a, unsigned long b, unsigned *after);

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
         SSE_OP("op_mulsd", "mulsd")
         SSE_OP("op_divsd", "divsd")
         SSE_OP("op_subsd", "subsd")
         SSE_OP("op_addss", "addss")
         ".globl op_ucomisd\n"
         "op_ucomisd: subq $8, %rsp\n"
         "stmxcsr (%rsp)\n"
         "movl %edi, 4(%rsp)\n"
         "ldmxcsr 4(%rsp)\n"
         "movq %rsi, %xmm0\n"
         "movq %rdx, %xmm1\n"
         "movq $0, %rax\n"
         "movq $0, %rsi\n"
         "movq $0, %rdx\n"
         "ucomisd %xmm1, %xmm0\n"
         "sete %al\n"
         "setp %sil\n"
         "setb %dl\n"
         "stmxcsr (%rcx)\n"
         "ldmxcsr (%rsp)\n"
         "addq %rax, %rax\n"
         "addq %rsi, %rax\n"
         "addq %rax, %rax\n"
         "addq %rdx, %rax\n"
         "addq $8, %rsp\n"
         "ret\n");
#endif

// Every exception masked, and the bits that bend IEEE 754.
#define MASKED 0x1F80
#define DAZ    0x0040
#define FZ     0x8000

// What ucomisd leaves: ZF, PF and CF as bits 2, 1 and 0.
#define EQUAL     4
#define LESS      1
#define UNORDERED 7

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

// A result tiny before rounding but not after should set PE alone; one tiny
// and exact nothing; one tiny and inexact UE and PE.
// [Amd64-Vol1:4.10.2.7]: UE, a rounded result too small to be normal, and inexact when masked
static int check_tiny(void)
{
    static const struct check checks[] = {
        {op_mulsd, MASKED, 0x3FEFFFFFFFFFFFFE, 0x0010000000000001, 0x0010000000000000, 0x1FA0},
        {op_mulsd, MASKED, 0x3FE0000000000000, 0x0010000000000000, 0x0008000000000000, 0x1F80},
        {op_mulsd, MASKED, 0x3FE0000000000001, 0x0010000000000001, 0x0008000000000001, 0x1FB0}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 1);
}

// FZ should give a tiny result as zero, setting UE and PE even when exact.
// [Amd64-Vol1:4.2.2]: FZ, flush to zero for masked underflow
static int check_flush(void)
{
    static const struct check checks[] = {
        {op_mulsd, MASKED | FZ, 0x3FE0000000000000, 0x0010000000000000, 0x0000000000000000, 0x9FB0},
        {op_mulsd, MASKED | FZ, 0x3FEFFFFFFFFFFFFE, 0x0010000000000001, 0x0010000000000000, 0x9FA0}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 10);
}

// A denormal operand should set DE, unless DAZ reads it as a zero of its sign.
// [Amd64-Vol1:4.2.2]: DE for a denormal operand, unless DAZ is set
static int check_denormal(void)
{
    static const struct check checks[] = {
        {op_addsd, MASKED,       0x0000000000000001, 0x3FF0000000000000, 0x3FF0000000000000, 0x1FA2},
        {op_addsd, MASKED,       0x0000000000000001, 0x0000000000000000, 0x0000000000000001, 0x1F82},
        {op_addsd, MASKED | DAZ, 0x8000000000000001, 0x0000000000000000, 0x0000000000000000, 0x1FC0},
        {op_addsd, MASKED | DAZ, 0x8000000000000001, 0x8000000000000001, 0x8000000000000000, 0x1FC0}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 20);
}

// A division by zero should set ZE alone, before a denormal's DE.
// [Amd64-Vol1:4.10.3]: Table 4-13, ZE before DE
static int check_divide(void)
{
    static const struct check checks[] = {
        {op_divsd, MASKED, 0x3FF0000000000000, 0x0000000000000000, 0x7FF0000000000000, 0x1F84},
        {op_divsd, MASKED, 0x0000000000000001, 0x0000000000000000, 0x7FF0000000000000, 0x1F84}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 30);
}

// An invalid operation should give the negative default NaN, and a NaN
// operand its own, quieted, the first before the second, raising IE for an
// SNaN alone.
// [Amd64-Vol1:4.3.3.3]: Table 4-5, the NaN results from NaN operands
static int check_nan(void)
{
    static const struct check checks[] = {
        {op_divsd, MASKED, 0x0000000000000000, 0x0000000000000000, 0xFFF8000000000000, 0x1F81},
        {op_subsd, MASKED, 0x7FF0000000000000, 0x7FF0000000000000, 0xFFF8000000000000, 0x1F81},
        {op_mulsd, MASKED, 0x0000000000000000, 0x7FF0000000000000, 0xFFF8000000000000, 0x1F81},
        {op_addsd, MASKED, 0x7FF8000000000001, 0xFFF8000000000002, 0x7FF8000000000001, 0x1F80},
        {op_addsd, MASKED, 0x3FF0000000000000, 0xFFF8000000000002, 0xFFF8000000000002, 0x1F80},
        {op_addsd, MASKED, 0x7FF0000000000001, 0xFFF8000000000002, 0x7FF8000000000001, 0x1F81},
        {op_addsd, MASKED, 0x7FF8000000000001, 0xFFF0000000000002, 0x7FF8000000000001, 0x1F81},
        {op_addsd, MASKED, 0x3FF0000000000000, 0xFFF0000000000002, 0xFFF8000000000002, 0x1F81},
        {op_addsd, MASKED, 0x7FF0000000000001, 0x0000000000000001, 0x7FF8000000000001, 0x1F81},
        {op_addss, MASKED, 0x7FC00001,         0xFFC00002,         0x7FC00001,         0x1F80}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 40);
}

// ucomisd should set ZF, PF and CF by the order, IE for an SNaN alone.
// [Amd64-Vol4:UCOMISD]: the flags of each order, and IE for an SNaN
static int check_compare(void)
{
    static const struct check checks[] = {
        {op_ucomisd, MASKED,       0x3FF0000000000000, 0x4000000000000000, LESS,      0x1F80},
        {op_ucomisd, MASKED,       0x3FF0000000000000, 0x3FF0000000000000, EQUAL,     0x1F80},
        {op_ucomisd, MASKED,       0x4000000000000000, 0x3FF0000000000000, 0,         0x1F80},
        {op_ucomisd, MASKED,       0x7FF8000000000000, 0x4000000000000000, UNORDERED, 0x1F80},
        {op_ucomisd, MASKED,       0x7FF0000000000001, 0x4000000000000000, UNORDERED, 0x1F81},
        {op_ucomisd, MASKED,       0x0000000000000001, 0x4000000000000000, LESS,      0x1F82},
        {op_ucomisd, MASKED | DAZ, 0x0000000000000001, 0x8000000000000000, EQUAL,     0x1FC0}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 60);
}

int main(void)
{
    int failed = check_tiny();

    failed = failed ? failed : check_flush();
    failed = failed ? failed : check_denormal();
    failed = failed ? failed : check_divide();
    failed = failed ? failed : check_nan();
    failed = failed ? failed : check_compare();
    return failed ? failed : 42;
}
