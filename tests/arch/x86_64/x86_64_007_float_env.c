// (Test) Status: 42
// The floating-point environment's instructions in file-scope assembly:
// MXCSR, the x87 control word and the x87 status word start as the psABI
// sets them, take what is loaded and give it back, and the x87 environment
// is stored, loaded and cleared, on the machine and in the emulator.

void round_trip_mxcsr(unsigned *mxcsr);
void round_trip_fcw(unsigned short *fcw);
void store_mxcsr(unsigned *mxcsr);
void store_fcw(unsigned short *fcw);
void store_fsw(unsigned short *fsw);
long store_fsw_ax(void);
void store_env(unsigned *env);
void store_env_masking(unsigned *env, unsigned short *fcw);
void load_env_status(const unsigned *env, unsigned short *fsw);
void store_env_holding(unsigned *env, const float *first, const float *second);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl round_trip_mxcsr\n"
         "round_trip_mxcsr: subq $8, %rsp\n"
         "stmxcsr (%rsp)\n"
         "ldmxcsr (%rdi)\n"
         "stmxcsr (%rdi)\n"
         "ldmxcsr (%rsp)\n"
         "addq $8, %rsp\n"
         "ret\n"
         ".globl round_trip_fcw\n"
         "round_trip_fcw: subq $8, %rsp\n"
         "fnstcw (%rsp)\n"
         "fldcw (%rdi)\n"
         "fnstcw (%rdi)\n"
         "fldcw (%rsp)\n"
         "addq $8, %rsp\n"
         "ret\n"
         ".globl store_mxcsr\n"
         "store_mxcsr: stmxcsr (%rdi)\n"
         "ret\n"
         ".globl store_fcw\n"
         "store_fcw: fnstcw (%rdi)\n"
         "ret\n"
         ".globl store_fsw\n"
         "store_fsw: fnstsw (%rdi)\n"
         "ret\n"
         ".globl store_fsw_ax\n"
         "store_fsw_ax: movq $0, %rax\n"
         "fnstsw %ax\n"
         "ret\n"
         ".globl store_env\n"
         "store_env: fnstenv (%rdi)\n"
         "ret\n"
         ".globl store_env_masking\n"
         "store_env_masking: subq $8, %rsp\n"
         "fnstcw (%rsp)\n"
         "fldcw (%rsi)\n"
         "fnstenv (%rdi)\n"
         "fnstcw (%rsi)\n"
         "fldcw (%rsp)\n"
         "addq $8, %rsp\n"
         "ret\n"
         ".globl load_env_status\n"
         "load_env_status: subq $32, %rsp\n"
         "fnstenv (%rsp)\n"
         "fldenv (%rdi)\n"
         "fnstsw (%rsi)\n"
         "movq $0, %rax\n"
         "fnstsw %ax\n"
         "movw %ax, 2(%rsi)\n"
         "fnclex\n"
         "fnstsw 4(%rsi)\n"
         "fwait\n"
         "fldenv (%rsp)\n"
         "addq $32, %rsp\n"
         "ret\n"
         ".globl store_env_holding\n"
         "store_env_holding: flds (%rsi)\n"
         "flds (%rdx)\n"
         "fnstenv (%rdi)\n"
         "fstp %st(0)\n"
         "fstp %st(0)\n"
         "ret");
#endif

// A float's bits.
union bits {
    unsigned u;
    float f;
};

// Every exception flag should be clear and every exception masked at startup.
// [Amd64-SysV:3.4.1]: the control words at process startup
static int check_startup(void)
{
    unsigned mxcsr = 0;
    unsigned short fcw = 0;
    unsigned short fsw = 0;

    store_mxcsr(&mxcsr);
    if (mxcsr != 0x1F80) {
        return 1;
    }
    store_fcw(&fcw);
    if (fcw != 0x037F) {
        return 2;
    }
    store_fsw(&fsw);
    if (fsw != 0 || store_fsw_ax() != 0) {
        return 3;
    }
    return 0;
}

// MXCSR should give back what was loaded: flags, masks and rounding.
// [Amd64-Vol1:4.2.2]: MXCSR's fields, all read and written
static int check_mxcsr(void)
{
    unsigned mxcsr = 0x7FBF;

    round_trip_mxcsr(&mxcsr);
    if (mxcsr != 0x7FBF) {
        return 4;
    }
    mxcsr = 0;
    round_trip_mxcsr(&mxcsr);
    if (mxcsr != 0) {
        return 5;
    }
    return 0;
}

// The x87 control word should give back what was loaded.
// [Amd64-Vol1:6.2.3]: the x87 control word's fields
static int check_fcw(void)
{
    unsigned short fcw = 0x0F7F;

    round_trip_fcw(&fcw);
    if (fcw != 0x0F7F) {
        return 6;
    }
    fcw = 0x0040;
    round_trip_fcw(&fcw);
    if (fcw != 0x0040) {
        return 7;
    }
    return 0;
}

// The environment at startup should hold the control word, a clear status
// word and every register empty.
// [Amd64-Vol1:6.2.4]: the x87 tag word, 11b for an empty register
static int check_env_startup(unsigned *start)
{
    store_env(start);
    if ((start[0] & 0xFFFF) != 0x037F || (start[1] & 0xFFFF) != 0 || (start[2] & 0xFFFF) != 0xFFFF) {
        return 8;
    }
    return 0;
}

// FNSTENV should store the control word as it was, then mask every exception.
// [Amd64-Vol5:FSTENV]: the environment stored, then the exceptions masked
static int check_env_masking(void)
{
    unsigned env[7] = {0};
    unsigned short fcw = 0x0F40;

    store_env_masking(env, &fcw);
    if ((env[0] & 0xFFFF) != 0x0F40 || fcw != 0x0F7F) {
        return 9;
    }
    return 0;
}

// FLDENV should load the status word, and FNCLEX clear its flags.
// [Amd64-Vol5:FCLEX]: the exception, stack fault, summary and busy flags
static int check_env_flags(const unsigned *start)
{
    unsigned env[7] = {0};
    unsigned short fsw[3] = {0};

    for (int i = 0; i < 7; i++) {
        env[i] = start[i];
    }
    env[1] = 0x473F;
    load_env_status(env, fsw);
    if (fsw[0] != 0x473F || fsw[1] != 0x473F || (fsw[2] & 0x80FF) != 0) {
        return 10;
    }
    return 0;
}

// An unmasked exception's flag should set the summary and busy bits.
// [Amd64-Vol1:6.2.2]: ES set by an unmasked exception, B equal to ES
static int check_env_summary(const unsigned *start)
{
    unsigned env[7] = {0};
    unsigned short fsw[3] = {0};

    for (int i = 0; i < 7; i++) {
        env[i] = start[i];
    }
    env[0] = 0x037E;
    env[1] = 0x0001;
    load_env_status(env, fsw);
    if (fsw[0] != 0x8081 || fsw[1] != 0x8081 || (fsw[2] & 0x80FF) != 0) {
        return 11;
    }
    return 0;
}

// Each full register's tag should say what it holds, and TOP where the stack
// is.
// [Amd64-Vol2:11.5.10.1]: Table 11-8, the tag of each kind of value
static int check_env_tags(void)
{
    unsigned env[7] = {0};
    union bits zero = {0x00000000};
    union bits one = {0x3F800000};
    union bits inf = {0x7F800000};

    store_env_holding(env, &zero.f, &one.f);
    if ((env[2] & 0xFFFF) != 0x4FFF || (env[1] >> 11 & 7) != 6) {
        return 12;
    }
    store_env_holding(env, &inf.f, &one.f);
    if ((env[2] & 0xFFFF) != 0x8FFF) {
        return 13;
    }
    return 0;
}

int main(void)
{
    unsigned start[7] = {0};
    int failed = check_startup();

    failed = failed ? failed : check_mxcsr();
    failed = failed ? failed : check_fcw();
    failed = failed ? failed : check_env_startup(start);
    failed = failed ? failed : check_env_masking();
    failed = failed ? failed : check_env_flags(start);
    failed = failed ? failed : check_env_summary(start);
    failed = failed ? failed : check_env_tags();
    return failed ? failed : 42;
}
