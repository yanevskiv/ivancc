// (Test) Status: 0
// A handler enters with MXCSR and the x87 control word as a process starts
// with them, and returns to the interrupted code with its own restored, on
// the machine and in the emulator.

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
void load_mxcsr(const unsigned *mxcsr);
void store_mxcsr(unsigned *mxcsr);
void load_fcw(const unsigned short *fcw);
void store_fcw(unsigned short *fcw);

unsigned entry_mxcsr;
unsigned short entry_fcw;
unsigned handler_mxcsr = 0x7F80;
unsigned short handler_fcw = 0x0F7F;

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
         "on_signal: leaq entry_mxcsr(%rip), %rax\n"
         "stmxcsr (%rax)\n"
         "leaq entry_fcw(%rip), %rax\n"
         "fnstcw (%rax)\n"
         "leaq handler_mxcsr(%rip), %rax\n"
         "ldmxcsr (%rax)\n"
         "leaq handler_fcw(%rip), %rax\n"
         "fldcw (%rax)\n"
         "ret\n"
         ".globl load_mxcsr\n"
         "load_mxcsr: ldmxcsr (%rdi)\n"
         "ret\n"
         ".globl store_mxcsr\n"
         "store_mxcsr: stmxcsr (%rdi)\n"
         "ret\n"
         ".globl load_fcw\n"
         "load_fcw: fldcw (%rdi)\n"
         "ret\n"
         ".globl store_fcw\n"
         "store_fcw: fnstcw (%rdi)\n"
         "ret");
#endif

int main(void)
{
    struct sigaction_linux act = {0};
    unsigned mxcsr = 0x5F80;
    unsigned short fcw = 0x077F;
    unsigned start_mxcsr = 0x1F80;
    unsigned short start_fcw = 0x037F;

    act.handler = on_signal;
    act.flags = SA_SIGINFO | SA_RESTORER;
    act.restorer = restorer;
    if (sys(SYS_RT_SIGACTION, SIGUSR1, (long) &act, 0, 8) != 0) {
        return 100;
    }
    load_mxcsr(&mxcsr);
    load_fcw(&fcw);
    if (sys(SYS_KILL, sys(SYS_GETPID, 0, 0, 0, 0), SIGUSR1, 0, 0) != 0) {
        return 101;
    }
    store_mxcsr(&mxcsr);
    store_fcw(&fcw);
    load_mxcsr(&start_mxcsr);
    load_fcw(&start_fcw);
    if (entry_mxcsr != 0x1F80 || entry_fcw != 0x037F) {
        return 1;
    }
    if (mxcsr != 0x5F80 || fcw != 0x077F) {
        return 2;
    }
    return 0;
}
