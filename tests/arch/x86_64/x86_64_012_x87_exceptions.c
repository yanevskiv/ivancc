// (Test) Status: 42
// x87 operations in file-scope assembly raise the status word's flags as the
// hardware does with every exception masked: the NaN each pair of NaNs
// gives, unsupported encodings, a division by zero and a denormal operand,
// stack faults and C1, fucomip's flags, and the conversions' invalid cases,
// on the machine and in the emulator.

// An x87 register's bits: the significand, then the sign and exponent.
struct x80 {
    unsigned long lo;
    unsigned long hi;
};

unsigned long op_faddp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fsubrp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fdivrp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fucomip(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fstps(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);
unsigned long op_fstpl(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);
unsigned long op_fisttpq(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);
unsigned long op_flds(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_fldl(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_fldt(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_fchs(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_push9(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_empty_faddp(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_empty_fchs(unsigned fcw, const struct x80 *src, unsigned long *unused, struct x80 *out);
unsigned long op_empty_fstpl(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);

// Give the status word in %rax, %st and %st(1) at out under every mask, and
// empty the stack and clear the status word for the next call.
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

// Load *src by INSN.
#define X87_LOAD(NAME, INSN)      \
    X87_START(NAME)               \
    INSN " (%rsi)\n"              \
    X87_FINISH

#ifdef __x86_64__
__asm__ (".text\n"
         X87_BINOP("op_faddp", "faddp")
         X87_BINOP("op_fsubrp", "fsubrp")
         X87_BINOP("op_fdivrp", "fdivrp")
         X87_STORE("op_fstps", "fstps")
         X87_STORE("op_fstpl", "fstpl")
         X87_STORE("op_fisttpq", "fisttpq")
         X87_LOAD("op_flds", "flds")
         X87_LOAD("op_fldl", "fldl")
         X87_LOAD("op_fldt", "fldt")
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
         X87_START("op_fchs")
         "fldt (%rsi)\n"
         "fchs\n"
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
         X87_FINISH
         X87_START("op_empty_fchs")
         "fchs\n"
         X87_FINISH
         X87_START("op_empty_fstpl")
         "fstpl (%rdx)\n"
         X87_FINISH);
#endif

// Every exception masked.
#define MASKED 0x037F

// The status word's TOP, which the checks leave out.
#define FSW_TOP 0x3800

// What fucomip leaves: ZF, PF and CF as bits 2, 1 and 0.
#define EQUAL     4
#define LESS      1
#define UNORDERED 7

// Operands: one, two, NaNs, the unsupported encodings and denormals.
#define ONE       {0x8000000000000000, 0x3FFF}
#define TWO       {0x8000000000000000, 0x4000}
#define ZERO      {0x0000000000000000, 0x0000}
#define NZERO     {0x0000000000000000, 0x8000}
#define INF       {0x8000000000000000, 0x7FFF}
#define NINF      {0x8000000000000000, 0xFFFF}
#define QNAN1     {0xC000000000000001, 0x7FFF}
#define QNAN2     {0xC000000000000002, 0x7FFF}
#define NQNAN1    {0xC000000000000001, 0xFFFF}
#define NQNAN2    {0xC000000000000002, 0xFFFF}
#define SNAN1     {0x8000000000000001, 0x7FFF}
#define SNAN2     {0x8000000000000002, 0x7FFF}
#define UNNORMAL  {0x4000000000000000, 0x3FFF}
#define PSEUDOINF {0x0000000000000000, 0x7FFF}
#define PSEUDONAN {0x4000000000000001, 0x7FFF}
#define DENORMAL  {0x0000000000000001, 0x0000}
#define PDENORMAL {0x8000000000000001, 0x0000}
#define INDEF     {0xC000000000000000, 0xFFFF}

// The indefinite an empty register stores as, so a popped %st(1).
#define INDEFINITE_LO 0xC000000000000000
#define INDEFINITE_HI 0xFFFF

// One operation on two registers, and what it should give.
struct check {
    unsigned long (*op)(unsigned, const struct x80 *, const struct x80 *, struct x80 *);
    struct x80 a;
    struct x80 b;
    unsigned fsw;
    struct x80 result;
};

// One store, its operand and what it should store.
struct store {
    unsigned long (*op)(unsigned, const struct x80 *, unsigned long *, struct x80 *);
    struct x80 a;
    unsigned fsw;
    unsigned long stored;
};

// One operation on one or no register, and the register it should give.
struct load {
    unsigned long (*op)(unsigned, const struct x80 *, unsigned long *, struct x80 *);
    struct x80 src;
    unsigned fsw;
    struct x80 result;
};

// Run each check, failing with first plus its index; out[2] holds fucomip's
// flags and out[0] the register the pop leaves.
static int run(const struct check *checks, int count, int first)
{
    for (int i = 0; i < count; i++) {
        struct x80 out[3] = {{0, 0}, {0, 0}, {0, 0}};
        unsigned long fsw = checks[i].op(MASKED, &checks[i].a, &checks[i].b, out);
        struct x80 got = checks[i].op == op_fucomip ? out[2] : out[0];

        if ((fsw & ~FSW_TOP) != checks[i].fsw || got.lo != checks[i].result.lo || got.hi != checks[i].result.hi) {
            return first + i;
        }
        if (out[1].lo != INDEFINITE_LO || out[1].hi != INDEFINITE_HI) {
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
        unsigned long stored = 0;
        unsigned long fsw = stores[i].op(MASKED, &stores[i].a, &stored, out);

        if ((fsw & ~FSW_TOP) != stores[i].fsw || stored != stores[i].stored || out[0].lo != INDEFINITE_LO) {
            return first + i;
        }
    }
    return 0;
}

// Run each load, failing with first plus its index.
static int run_loads(const struct load *loads, int count, int first)
{
    for (int i = 0; i < count; i++) {
        struct x80 out[2] = {{0, 0}, {0, 0}};
        unsigned long fsw = loads[i].op(MASKED, &loads[i].src, 0, out);

        if ((fsw & ~FSW_TOP) != loads[i].fsw || out[0].lo != loads[i].result.lo || out[0].hi != loads[i].result.hi) {
            return first + i;
        }
    }
    return 0;
}

// Of two NaNs the quiet one should pass, else the one of the larger
// significand, quieted, a positive one of two that differ in sign alone; a
// NaN keeps its sign through a subtraction, and an SNaN raises IE.
// [Amd64-Vol1:6.3.4.1]: Table 6-7, the NaN results from NaN operands
static int check_nan(void)
{
    static const struct check checks[] = {
        {op_faddp,  QNAN1,  QNAN2,  0x0000, QNAN2},
        {op_faddp,  SNAN1,  SNAN2,  0x0001, QNAN2},
        {op_faddp,  SNAN2,  QNAN1,  0x0001, QNAN1},
        {op_faddp,  QNAN1,  SNAN2,  0x0001, QNAN1},
        {op_faddp,  NQNAN2, QNAN1,  0x0000, NQNAN2},
        {op_faddp,  QNAN1,  NQNAN1, 0x0000, QNAN1},
        {op_faddp,  NQNAN1, NQNAN1, 0x0000, NQNAN1},
        {op_faddp,  SNAN1,  ONE,    0x0001, QNAN1},
        {op_fsubrp, ONE,    NQNAN1, 0x0000, NQNAN1}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 1);
}

// An unsupported operand should give the indefinite and IE, even beside a
// NaN, as an invalid operation does.
// [Amd64-Vol1:6.3.4.2]: Table 6-9, unsupported operands raise IE
// [Amd64-Vol1:6.3.4.3]: Table 6-10, the floating-point indefinite
static int check_invalid(void)
{
    static const struct check checks[] = {
        {op_faddp,  UNNORMAL,  ONE,      0x0001, INDEF},
        {op_faddp,  QNAN1,     UNNORMAL, 0x0001, INDEF},
        {op_faddp,  PSEUDOINF, ONE,      0x0001, INDEF},
        {op_faddp,  PSEUDONAN, ONE,      0x0001, INDEF},
        {op_faddp,  INF,       NINF,     0x0001, INDEF},
        {op_fdivrp, ZERO,      ZERO,     0x0001, INDEF}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 20);
}

// A division by zero should set ZE alone, a denormal operand DE but beside
// a QNaN, and a pseudo-denormal read as a normal number of the least exponent.
// [Amd64-Vol1:6.3.3.3]: a pseudo-denormal reads with a biased exponent of 1, raising DE
// [Amd64-Vol1:6.8.5]:   Table 6-19, a QNaN operand before DE
static int check_denormal(void)
{
    static const struct check checks[] = {
        {op_fdivrp,  ONE,       ZERO,     0x0004, INF},
        {op_fdivrp,  DENORMAL,  ZERO,     0x0004, INF},
        {op_fdivrp,  ONE,       DENORMAL, 0x022A, INF},
        {op_faddp,   PDENORMAL, ZERO,     0x0002, {0x8000000000000001, 0x0001}},
        {op_faddp,   DENORMAL,  ZERO,     0x0002, DENORMAL},
        {op_faddp,   DENORMAL,  QNAN1,    0x0000, QNAN1},
        {op_fucomip, QNAN1,     DENORMAL, 0x0000, {UNORDERED, 0}}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 30);
}

// fucomip should set ZF, PF and CF by the order of %st to %st(1), IE for an
// SNaN or an unsupported operand alone, DE for a denormal.
// [Amd64-Vol1:6.4.6]: Table 6-14, the rFLAGS of each order
static int check_compare(void)
{
    static const struct check checks[] = {
        {op_fucomip, ONE,  TWO,      0x0000, {0,         0}},
        {op_fucomip, TWO,  ONE,      0x0000, {LESS,      0}},
        {op_fucomip, ONE,  ONE,      0x0000, {EQUAL,     0}},
        {op_fucomip, ZERO, NZERO,    0x0000, {EQUAL,     0}},
        {op_fucomip, ONE,  QNAN1,    0x0000, {UNORDERED, 0}},
        {op_fucomip, ONE,  SNAN1,    0x0001, {UNORDERED, 0}},
        {op_fucomip, ONE,  UNNORMAL, 0x0001, {UNORDERED, 0}},
        {op_fucomip, ONE,  DENORMAL, 0x0002, {LESS,      0}}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 40);
}

// A store should quiet an SNaN, keeping its payload's top bits, and store
// the indefinite for an unsupported value, both under IE; FISTTP the integer
// indefinite for a NaN, an unsupported value or one out of range; a denormal
// stores with no DE.
// [Amd64-Vol1:6.8.4.2]: Table 6-18, the causes of IE
static int check_stores(void)
{
    static const struct store stores[] = {
        {op_fstps,   SNAN1,                                0x0001, 0x7FC00000},
        {op_fstps,   {0xC000030000000001, 0xFFFF},         0x0000, 0xFFC00003},
        {op_fstps,   UNNORMAL,                             0x0001, 0xFFC00000},
        {op_fstps,   PSEUDOINF,                            0x0001, 0xFFC00000},
        {op_fstps,   DENORMAL,                             0x0030, 0x00000000},
        {op_fstpl,   {0x8000000000000801, 0x7FFF},         0x0001, 0x7FF8000000000001},
        {op_fisttpq, {0x8000000000000000, 0x403E},         0x0001, 0x8000000000000000},
        {op_fisttpq, {0x8000000000000000, 0xC03E},         0x0000, 0x8000000000000000},
        {op_fisttpq, QNAN1,                                0x0001, 0x8000000000000000},
        {op_fisttpq, UNNORMAL,                             0x0001, 0x8000000000000000},
        {op_fisttpq, DENORMAL,                             0x0020, 0}
    };
    return run_stores(stores, (int) (sizeof(stores) / sizeof(stores[0])), 50);
}

// A load of binary32 or binary64 should quiet an SNaN under IE, its payload
// left-aligned; FLD m80 and FCHS take any bits as they are.
// [Amd64-Vol1:6.3.3.6]: an SNaN operand raises IE, a QNaN given when masked
static int check_loads(void)
{
    static const struct load loads[] = {
        {op_flds, {0x7F800001, 0},         0x0001, {0xC000010000000000, 0x7FFF}},
        {op_fldl, {0xFFF4000000000001, 0}, 0x0001, {0xE000000000000800, 0xFFFF}},
        {op_fldt, SNAN1,                   0x0000, SNAN1},
        {op_fldt, UNNORMAL,                0x0000, UNNORMAL},
        {op_fchs, SNAN1,                   0x0000, {0x8000000000000001, 0xFFFF}},
        {op_fchs, UNNORMAL,                0x0000, {0x4000000000000000, 0xBFFF}}
    };
    return run_loads(loads, (int) (sizeof(loads) / sizeof(loads[0])), 70);
}

// A push onto a full register should push the indefinite with IE, SF and
// C1 set; a read of an empty one write the indefinite with IE and SF.
// [Amd64-Vol1:6.8.4.8]: SF and IE, C1 = 1 for an overflow and 0 for an underflow
static int check_stack(void)
{
    static const struct load loads[] = {
        {op_push9,       ONE, 0x0241, INDEF},
        {op_empty_faddp, ONE, 0x0041, INDEF},
        {op_empty_fchs,  ONE, 0x0041, INDEF}
    };
    struct x80 out[2] = {{0, 0}, {0, 0}};
    unsigned long stored = 0;
    int failed = run_loads(loads, (int) (sizeof(loads) / sizeof(loads[0])), 80);

    if (failed) {
        return failed;
    }
    if ((op_empty_fstpl(MASKED, 0, &stored, out) & ~FSW_TOP) != 0x0041 || stored != 0xFFF8000000000000) {
        return 89;
    }
    return 0;
}

int main(void)
{
    int failed = check_nan();

    failed = failed ? failed : check_invalid();
    failed = failed ? failed : check_denormal();
    failed = failed ? failed : check_compare();
    failed = failed ? failed : check_stores();
    failed = failed ? failed : check_loads();
    failed = failed ? failed : check_stack();
    return failed ? failed : 42;
}
