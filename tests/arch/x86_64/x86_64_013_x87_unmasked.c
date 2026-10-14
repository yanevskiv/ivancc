// (Test) Status: 42
// An unmasked x87 exception in file-scope assembly leaves what the hardware
// leaves before the #MF it waits to raise: OE and UE to a register write the
// result scaled into range, to memory nothing; IE, ZE and DE write nothing
// and keep the stack, but a load's DE, and PE writes its result, with ES and
// B set, on the machine and in the emulator.

// An x87 register's bits: the significand, then the sign and exponent.
struct x80 {
    unsigned long lo;
    unsigned long hi;
};

unsigned long op_faddp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fmulp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fdivrp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fucomip(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fstps(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);
unsigned long op_fisttpq(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);
unsigned long op_flds(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_push9(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_empty_faddp(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);

// Give the status word in %rax, %st and %st(1) at out under every mask, and
// empty the stack and clear the status word for the next call; FNSTSW and
// FNCLEX wait for no exception, so none is raised.
#define X87_FINISH                \
    "movq $0, %rax\n"             \
    "fnstsw %ax\n"                \
    "fnclex\n"                    \
    "movq $0x37F, %rdi\n"         \
    "movl %edi, 36(%rsp)\n"       \
    "fldcw 36(%rsp)\n"            \
    "fstpt (%rcx)\n"              \
    "fstpt 16(%rcx)\n"            \
    "fnstenv (%rsp)\n"            \
    "movq $0, %rdi\n"             \
    "movl %edi, 4(%rsp)\n"        \
    "movq $0xFFFF, %rdi\n"        \
    "movl %edi, 8(%rsp)\n"        \
    "fldenv (%rsp)\n"             \
    "fldcw 32(%rsp)\n"            \
    "addq $40, %rsp\n"            \
    "ret\n"

// Load the control word from %edi, saving the caller's.
#define X87_START(NAME)           \
    ".globl " NAME "\n"           \
    NAME ": subq $40, %rsp\n"     \
    "fnstcw 32(%rsp)\n"           \
    "movl %edi, 36(%rsp)\n"       \
    "fldcw 36(%rsp)\n"

// Run INSN on %st(1) = *a and %st = *b.
#define X87_BINOP(NAME, INSN)     \
    X87_START(NAME)               \
    "fldt (%rsi)\n"               \
    "fldt (%rdx)\n"               \
    INSN " %st, %st(1)\n"         \
    X87_FINISH

// Store %st = *a by INSN at *dst.
#define X87_STORE(NAME, INSN)     \
    X87_START(NAME)               \
    "fldt (%rsi)\n"               \
    INSN " (%rdx)\n"              \
    X87_FINISH

#ifdef __x86_64__
__asm__ (".text\n"
         X87_BINOP("op_faddp", "faddp")
         X87_BINOP("op_fmulp", "fmulp")
         X87_BINOP("op_fdivrp", "fdivrp")
         X87_STORE("op_fstps", "fstps")
         X87_STORE("op_fisttpq", "fisttpq")
         X87_START("op_flds")
         "flds (%rsi)\n"
         X87_FINISH
         X87_START("op_fucomip")
         "fldt (%rsi)\n"
         "fldt (%rdx)\n"
         "fucomip %st(1), %st\n"
         "movq $0, %rax\n"
         "movq $0, %rsi\n"
         "movq $0, %rdx\n"
         "sete %al\n"
         "setp %sil\n"
         "setb %dl\n"
         "addq %rax, %rax\n"
         "addq %rsi, %rax\n"
         "addq %rax, %rax\n"
         "addq %rdx, %rax\n"
         "movq %rax, 32(%rcx)\n"
         X87_FINISH
         X87_START("op_push9")
         "fldt (%rsi)\n"
         "fldt (%rsi)\n"
         "fldt (%rsi)\n"
         "fldt (%rsi)\n"
         "fldt (%rsi)\n"
         "fldt (%rsi)\n"
         "fldt (%rsi)\n"
         "fldt (%rsi)\n"
         "fldt (%rsi)\n"
         X87_FINISH
         X87_START("op_empty_faddp")
         "faddp %st, %st(1)\n"
         X87_FINISH);
#endif

// Control words with one exception unmasked, or two for OE at PC24.
#define UNMASK_IE   0x037E
#define UNMASK_DE   0x037D
#define UNMASK_ZE   0x037B
#define UNMASK_OE   0x0377
#define UNMASK_UE   0x036F
#define UNMASK_PE   0x035F
#define UNMASK_OE24 0x0077

// The status word's TOP, which the checks leave out.
#define FSW_TOP 0x3800

// What fucomip leaves: ZF, PF and CF as bits 2, 1 and 0.
#define LESS      1
#define UNORDERED 7

// What a store leaves in memory it does not write.
#define UNWRITTEN 0x1234567812345678

// Operands and results.
#define ONE      {0x8000000000000000, 0x3FFF}
#define THREE    {0xC000000000000000, 0x4000}
#define HALF     {0x8000000000000000, 0x3FFE}
#define ZERO     {0x0000000000000000, 0x0000}
#define INF      {0x8000000000000000, 0x7FFF}
#define MAX      {0xFFFFFFFFFFFFFFFF, 0x7FFE}
#define MIN      {0x8000000000000000, 0x0001}
#define QNAN1    {0xC000000000000001, 0x7FFF}
#define SNAN1    {0x8000000000000001, 0x7FFF}
#define DENORMAL {0x0000000000000001, 0x0000}
#define TWO63    {0x8000000000000000, 0x403E}
#define INDEF    {0xC000000000000000, 0xFFFF}

// One operation, its operands, and the two registers it should leave.
struct check {
    unsigned long (*op)(unsigned, const struct x80 *, const struct x80 *, struct x80 *);
    unsigned fcw;
    struct x80 a;
    struct x80 b;
    unsigned fsw;
    struct x80 st0;
    struct x80 st1;
};

// One operation on one or no register, and the two registers it should leave.
struct load {
    unsigned long (*op)(unsigned, const struct x80 *, unsigned long *, struct x80 *);
    unsigned fcw;
    struct x80 src;
    unsigned fsw;
    struct x80 st0;
    struct x80 st1;
};

// One store, its operand, what it should store and the register it leaves.
struct store {
    unsigned long (*op)(unsigned, const struct x80 *, unsigned long *, struct x80 *);
    unsigned fcw;
    struct x80 a;
    unsigned fsw;
    unsigned long stored;
    struct x80 st0;
};

// Return whether two registers' bits are the same.
static int same(struct x80 x, struct x80 y)
{
    return x.lo == y.lo && x.hi == y.hi;
}

// Run each check, failing with first plus its index.
static int run(const struct check *checks, int count, int first)
{
    for (int i = 0; i < count; i++) {
        struct x80 out[3] = {{0, 0}, {0, 0}, {0, 0}};
        unsigned long fsw = checks[i].op(checks[i].fcw, &checks[i].a, &checks[i].b, out);

        if ((fsw & ~FSW_TOP) != checks[i].fsw || ! same(out[0], checks[i].st0) || ! same(out[1], checks[i].st1)) {
            return first + i;
        }
    }
    return 0;
}

// Run each store, failing with first plus its index.
static int run_stores(const struct store *stores, int count, int first)
{
    for (int i = 0; i < count; i++) {
        struct x80 out[2] = {{0, 0}, {0, 0}};
        unsigned long stored = UNWRITTEN;
        unsigned long fsw = stores[i].op(stores[i].fcw, &stores[i].a, &stored, out);

        if ((fsw & ~FSW_TOP) != stores[i].fsw || stored != stores[i].stored || ! same(out[0], stores[i].st0)) {
            return first + i;
        }
    }
    return 0;
}

// An unmasked OE or UE to a register should write the result scaled by
// 2^-24576 or 2^24576, rounded at PC's precision, C1 by that rounding, PE
// only if inexact, UE for a tiny result exact or not, after rounding.
// [Amd64-Vol1:6.8.6.2]: Table 6-22, OE and UE for a register destination
static int check_scaled(void)
{
    static const struct check checks[] = {
        {op_fmulp, UNMASK_OE,   MAX,  MAX, 0x80A8, {0xFFFFFFFFFFFFFFFE, 0x5FFE}, INDEF},
        {op_fmulp, UNMASK_OE24, MAX,  MAX, 0x82A8, {0x8000000000000000, 0x5FFF}, INDEF},
        {op_fmulp, UNMASK_UE,   MIN,  MIN, 0x8090, {0x8000000000000000, 0x2003}, INDEF},
        {op_fmulp, UNMASK_UE,   HALF, MIN, 0x8090, {0x8000000000000000, 0x6000}, INDEF},
        {op_fmulp, UNMASK_UE,   {0xFFFFFFFFFFFFFFFE, 0x3FFE}, {0x8000000000000001, 0x0001}, 0x0220, MIN, INDEF}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 1);
}

// An unmasked IE, ZE or DE should write nothing and keep the stack; fucomip
// sets its flags all the same; an unmasked PE writes and pops.
// [Amd64-Vol1:6.8.6.2]: Table 6-22, the destination and TOP not changed
static int check_kept(void)
{
    static const struct check checks[] = {
        {op_fmulp,   UNMASK_IE, INF,   ZERO,     0x8081, ZERO,     INF},
        {op_fdivrp,  UNMASK_ZE, ONE,   ZERO,     0x8084, ZERO,     ONE},
        {op_faddp,   UNMASK_DE, ONE,   DENORMAL, 0x8082, DENORMAL, ONE},
        {op_faddp,   UNMASK_IE, SNAN1, ONE,      0x8081, ONE,      SNAN1},
        {op_fdivrp,  UNMASK_PE, ONE,   THREE,    0x82A0, {0xAAAAAAAAAAAAAAAB, 0x3FFD}, INDEF},
        {op_faddp,   UNMASK_PE, ONE,   THREE,    0x0000, {0x8000000000000000, 0x4001}, INDEF},
        {op_fucomip, UNMASK_IE, ONE,   SNAN1,    0x8081, SNAN1,    ONE},
        {op_fucomip, UNMASK_DE, ONE,   DENORMAL, 0x8082, DENORMAL, ONE}
    };
    struct x80 out[3] = {{0, 0}, {0, 0}, {0, 0}};
    int failed = run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 10);

    if (failed) {
        return failed;
    }
    op_fucomip(UNMASK_IE, &checks[6].a, &checks[6].b, out);
    if (out[2].lo != UNORDERED) {
        return 18;
    }
    op_fucomip(UNMASK_DE, &checks[7].a, &checks[7].b, out);
    if (out[2].lo != LESS) {
        return 19;
    }
    return 0;
}

// An unmasked OE, UE or IE to memory should store nothing and keep the
// stack, OE and UE without PE; an unmasked PE stores and pops.
// [Amd64-Vol1:6.8.6.2]: Table 6-22, OE and UE for a memory destination
static int check_stores(void)
{
    static const struct store stores[] = {
        {op_fstps,   UNMASK_OE, MAX,   0x8088, UNWRITTEN, MAX},
        {op_fstps,   UNMASK_UE, MIN,   0x8090, UNWRITTEN, MIN},
        {op_fstps,   UNMASK_IE, SNAN1, 0x8081, UNWRITTEN, SNAN1},
        {op_fisttpq, UNMASK_IE, QNAN1, 0x8081, UNWRITTEN, QNAN1},
        {op_fisttpq, UNMASK_IE, TWO63, 0x8081, UNWRITTEN, TWO63},
        {op_fisttpq, UNMASK_PE, HALF,  0x80A0, 0,         INDEF}
    };
    return run_stores(stores, (int) (sizeof(stores) / sizeof(stores[0])), 20);
}

// An unmasked IE should push nothing, on an SNaN or a full register, and
// pop nothing on an empty one; an unmasked DE still loads a denormal.
// [Amd64-Vol1:6.8.4.8]: SF and IE, C1 = 1 for an overflow and 0 for an underflow
static int check_loads(void)
{
    static const struct load loads[] = {
        {op_push9,       UNMASK_IE, ONE,                  0x82C1, ONE,                          ONE},
        {op_empty_faddp, UNMASK_IE, ONE,                  0x80C1, INDEF,                        INDEF},
        {op_flds,        UNMASK_IE, {0x7F800001, 0},      0x8081, INDEF,                        INDEF},
        {op_flds,        UNMASK_DE, {0x00000001, 0},      0x8082, {0x8000000000000000, 0x3F6A}, INDEF}
    };

    for (int i = 0; i < (int) (sizeof(loads) / sizeof(loads[0])); i++) {
        struct x80 out[2] = {{0, 0}, {0, 0}};
        unsigned long fsw = loads[i].op(loads[i].fcw, &loads[i].src, 0, out);

        if ((fsw & ~FSW_TOP) != loads[i].fsw || ! same(out[0], loads[i].st0) || ! same(out[1], loads[i].st1)) {
            return 30 + i;
        }
    }
    return 0;
}

int main(void)
{
    int failed = check_scaled();

    failed = failed ? failed : check_kept();
    failed = failed ? failed : check_stores();
    failed = failed ? failed : check_loads();
    return failed ? failed : 42;
}
