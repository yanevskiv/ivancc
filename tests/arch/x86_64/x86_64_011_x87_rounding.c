// (Test) Status: 42
// x87 arithmetic, loads and stores in file-scope assembly round as the
// hardware does with every exception masked: PC's precision in the extended
// exponent range, RC's direction, C1 for a rounding up, tininess after
// rounding, overflow by mode, and conversions that ignore PC, on the machine
// and in the emulator.

// An x87 register's bits: the significand, then the sign and exponent.
struct x80 {
    unsigned long lo;
    unsigned long hi;
};

unsigned long op_faddp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fmulp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fsubrp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fdivrp(unsigned fcw, const struct x80 *a, const struct x80 *b, struct x80 *out);
unsigned long op_fstps(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);
unsigned long op_fstpl(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);
unsigned long op_fisttpq(unsigned fcw, const struct x80 *a, unsigned long *dst, struct x80 *out);
unsigned long op_flds(unsigned fcw, const unsigned long *src, unsigned long *unused, struct x80 *out);
unsigned long op_fildq(unsigned fcw, const unsigned long *src, unsigned long *unused, struct x80 *out);

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
         X87_BINOP("op_fmulp", "fmulp")
         X87_BINOP("op_fsubrp", "fsubrp")
         X87_BINOP("op_fdivrp", "fdivrp")
         X87_STORE("op_fstps", "fstps")
         X87_STORE("op_fstpl", "fstpl")
         X87_STORE("op_fisttpq", "fisttpq")
         X87_LOAD("op_flds", "flds")
         X87_LOAD("op_fildq", "fildq"));
#endif

// Control words: every exception masked, at each precision and rounding.
#define PC64_RN 0x037F
#define PC53_RN 0x027F
#define PC24_RN 0x007F
#define PC_RSVD 0x017F
#define PC64_RD 0x077F
#define PC64_RU 0x0B7F
#define PC64_RZ 0x0F7F
#define PC24_RZ 0x0C7F

// The status word's TOP, which the checks leave out.
#define FSW_TOP 0x3800

// The indefinite an empty register stores as, so a popped %st(1).
#define INDEFINITE_LO 0xC000000000000000
#define INDEFINITE_HI 0xFFFF

// One arithmetic operation, its operands and what it should give.
struct check {
    unsigned long (*op)(unsigned, const struct x80 *, const struct x80 *, struct x80 *);
    unsigned fcw;
    struct x80 a;
    struct x80 b;
    unsigned fsw;
    struct x80 result;
};

// One store, its operand and what it should store.
struct store {
    unsigned long (*op)(unsigned, const struct x80 *, unsigned long *, struct x80 *);
    unsigned fcw;
    struct x80 a;
    unsigned fsw;
    unsigned long stored;
};

// One load, its operand and the register it should give.
struct load {
    unsigned long (*op)(unsigned, const unsigned long *, unsigned long *, struct x80 *);
    unsigned fcw;
    unsigned long src;
    unsigned fsw;
    struct x80 result;
};

// Run each check, failing with first plus its index.
static int run(const struct check *checks, int count, int first)
{
    for (int i = 0; i < count; i++) {
        struct x80 out[2] = {{0, 0}, {0, 0}};
        unsigned long fsw = checks[i].op(checks[i].fcw, &checks[i].a, &checks[i].b, out);

        if ((fsw & ~FSW_TOP) != checks[i].fsw || out[0].lo != checks[i].result.lo || out[0].hi != checks[i].result.hi) {
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
        unsigned long fsw = stores[i].op(stores[i].fcw, &stores[i].a, &stored, out);

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
        unsigned long fsw = loads[i].op(loads[i].fcw, &loads[i].src, 0, out);

        if ((fsw & ~FSW_TOP) != loads[i].fsw || out[0].lo != loads[i].result.lo || out[0].hi != loads[i].result.hi) {
            return first + i;
        }
    }
    return 0;
}

// 1/3 should round to PC's precision in RC's direction, C1 set when the
// magnitude rounded up; PC's reserved 01b rounds as 11b.
// [Amd64-Vol1:6.3.5]:    Table 6-11, the precision of each PC
// [Amd64-Vol1:6.2.2.10]: C1, the last rounding's direction
static int check_precision(void)
{
    static const struct check checks[] = {
        {op_fdivrp, PC24_RN, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 0x0220, {0xAAAAAB0000000000, 0x3FFD}},
        {op_fdivrp, PC53_RN, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 0x0020, {0xAAAAAAAAAAAAA800, 0x3FFD}},
        {op_fdivrp, PC64_RN, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 0x0220, {0xAAAAAAAAAAAAAAAB, 0x3FFD}},
        {op_fdivrp, PC_RSVD, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 0x0220, {0xAAAAAAAAAAAAAAAB, 0x3FFD}},
        {op_fdivrp, PC64_RU, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 0x0220, {0xAAAAAAAAAAAAAAAB, 0x3FFD}},
        {op_fdivrp, PC64_RD, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 0x0020, {0xAAAAAAAAAAAAAAAA, 0x3FFD}},
        {op_fdivrp, PC64_RZ, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 0x0020, {0xAAAAAAAAAAAAAAAA, 0x3FFD}},
        {op_fdivrp, PC64_RU, {0x8000000000000000, 0xBFFF}, {0xC000000000000000, 0x4000}, 0x0020, {0xAAAAAAAAAAAAAAAA, 0xBFFD}},
        {op_fdivrp, PC64_RD, {0x8000000000000000, 0xBFFF}, {0xC000000000000000, 0x4000}, 0x0220, {0xAAAAAAAAAAAAAAAB, 0xBFFD}},
        {op_faddp,  PC64_RN, {0x8000000000000000, 0x3FFF}, {0x8000000000000000, 0x3FBF}, 0x0020, {0x8000000000000000, 0x3FFF}},
        {op_faddp,  PC64_RN, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x3FBF}, 0x0220, {0x8000000000000001, 0x3FFF}},
        {op_fsubrp, PC64_RN, {0x8000000000000000, 0x3FFF}, {0xC000000000000000, 0x4000}, 0x0000, {0x8000000000000000, 0xC000}}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 1);
}

// A result rounding past the greatest finite number should give infinity
// to nearest, C1 set, and the greatest finite number at PC's precision
// toward zero, OE and PE set either way.
// [Amd64-Vol1:6.8.4.5]: OE, a rounded result too large for the destination
static int check_overflow(void)
{
    static const struct check checks[] = {
        {op_fdivrp, PC64_RN, {0xFFFFFFFFFFFFFFFF, 0x7FFE}, {0x8000000000000000, 0x3FFE}, 0x0228, {0x8000000000000000, 0x7FFF}},
        {op_fdivrp, PC64_RZ, {0xFFFFFFFFFFFFFFFF, 0x7FFE}, {0x8000000000000000, 0x3FFE}, 0x0028, {0xFFFFFFFFFFFFFFFF, 0x7FFE}},
        {op_fdivrp, PC24_RZ, {0xFFFFFFFFFFFFFFFF, 0x7FFE}, {0x8000000000000000, 0x3FFE}, 0x0028, {0xFFFFFF0000000000, 0x7FFE}},
        {op_fdivrp, PC24_RN, {0xFFFFFFFFFFFFFFFF, 0x7FFE}, {0x8000000000000000, 0x3FFF}, 0x0228, {0x8000000000000000, 0x7FFF}}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 20);
}

// A result tiny before rounding but not after, at each precision, should
// set PE alone; one tiny and exact nothing; one tiny and inexact UE and PE.
// [Amd64-Vol1:6.8.4.6]: UE, a result too small, and inexact when masked
static int check_tiny(void)
{
    static const struct check checks[] = {
        {op_fmulp, PC64_RN, {0xFFFFFFFFFFFFFFFE, 0x3FFE}, {0x8000000000000001, 0x0001}, 0x0220, {0x8000000000000000, 0x0001}},
        {op_fmulp, PC53_RN, {0xFFFFFFFFFFFFF000, 0x3FFE}, {0x8000000000000800, 0x0001}, 0x0220, {0x8000000000000000, 0x0001}},
        {op_fmulp, PC24_RN, {0xFFFFFE0000000000, 0x3FFE}, {0x8000010000000000, 0x0001}, 0x0220, {0x8000000000000000, 0x0001}},
        {op_fmulp, PC64_RN, {0x8000000000000000, 0x3FFE}, {0x8000000000000000, 0x0001}, 0x0000, {0x4000000000000000, 0x0000}},
        {op_fmulp, PC64_RN, {0x8000000000000000, 0x3FFE}, {0x8000000000000001, 0x0001}, 0x0030, {0x4000000000000000, 0x0000}}
    };
    return run(checks, (int) (sizeof(checks) / sizeof(checks[0])), 30);
}

// A store should round to its format in RC's direction whatever PC says,
// C1 set when it rounded up, tininess after rounding; FISTTP truncates.
// [Amd64-Vol1:6.2.3.2]: PC affects only addition, subtraction, multiplication, division and FSQRT
static int check_stores(void)
{
    static const struct store stores[] = {
        {op_fstps,   PC64_RN, {0xAAAAAAAAAAAAAAAB, 0x3FFD}, 0x0220, 0x3EAAAAAB},
        {op_fstps,   PC64_RZ, {0xAAAAAAAAAAAAAAAB, 0x3FFD}, 0x0020, 0x3EAAAAAA},
        {op_fstps,   PC24_RN, {0xAAAAAAAAAAAAAAAB, 0x3FFD}, 0x0220, 0x3EAAAAAB},
        {op_fstps,   PC64_RN, {0xFFFFFFFFFFFFFFFF, 0x7FFE}, 0x0228, 0x7F800000},
        {op_fstps,   PC64_RN, {0x8000000000000000, 0x0001}, 0x0030, 0x00000000},
        {op_fstps,   PC64_RN, {0xC000000000000000, 0x3F6A}, 0x0230, 0x00000002},
        {op_fstps,   PC64_RN, {0xFFFFFFFFFFFFFFFF, 0x3F80}, 0x0220, 0x00800000},
        {op_fstpl,   PC64_RN, {0xAAAAAAAAAAAAAAAB, 0x3FFD}, 0x0020, 0x3FD5555555555555},
        {op_fisttpq, PC64_RN, {0xA000000000000000, 0x4000}, 0x0020, 2},
        {op_fisttpq, PC64_RU, {0xA000000000000000, 0xC000}, 0x0020, (unsigned long) -2}
    };
    return run_stores(stores, (int) (sizeof(stores) / sizeof(stores[0])), 40);
}

// A load should be exact whatever PC says, clearing C1, a denormal raising DE.
// [Amd64-Vol1:6.3.2.1]: an operand loaded is converted to double-extended precision
static int check_loads(void)
{
    static const struct load loads[] = {
        {op_flds,  PC64_RN, 0x00000001,         0x0002, {0x8000000000000000, 0x3F6A}},
        {op_fildq, PC24_RN, 0x7FFFFFFFFFFFFFFF, 0x0000, {0xFFFFFFFFFFFFFFFE, 0x403D}},
        {op_fildq, PC64_RN, (unsigned long) -3, 0x0000, {0xC000000000000000, 0xC000}}
    };
    return run_loads(loads, (int) (sizeof(loads) / sizeof(loads[0])), 60);
}

int main(void)
{
    int failed = check_precision();

    failed = failed ? failed : check_overflow();
    failed = failed ? failed : check_tiny();
    failed = failed ? failed : check_stores();
    failed = failed ? failed : check_loads();
    return failed ? failed : 42;
}
