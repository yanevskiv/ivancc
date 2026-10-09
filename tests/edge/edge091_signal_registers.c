// (Test) Status: 0
// A handler for a fault that clobbers every register returns to the interrupted code with each one restored, flags, SSE and x87 included, on the machine and in the emulator.

#define SYS_RT_SIGACTION 13

#define SA_SIGINFO  0x4
#define SA_RESTORER 0x04000000

#define SIGILL 4

// Linux's struct sigaction, as rt_sigaction reads it.
struct sigaction_linux {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

long sys(long nr, long a, long b, long c, long d);
void restorer(void);
void on_ill(int sig);

long regs[36];
long saved_rsp;
double fp_in = 2.5;
double fp_out;

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx; movq %r8, %r10\n"
         "syscall\n"
         "ret\n"
         ".globl restorer\n"
         "restorer: movq $15, %rax # rt_sigreturn\n"
         "syscall\n"
         ".globl on_ill\n"
         "on_ill: movq 168(%rdx), %rax # the interrupted %rip\n"
         "addq $2, %rax\n"
         "movq %rax, 168(%rdx)\n"
         "movq $85, %rax\n"
         "movq %rax, %xmm0\n"
         "movq %rax, %xmm1\n"
         "movq %rax, %xmm2\n"
         "movq %rax, %xmm3\n"
         "movq %rax, %xmm4\n"
         "movq %rax, %xmm5\n"
         "movq %rax, %xmm6\n"
         "movq %rax, %xmm7\n"
         "movq %rax, %xmm8\n"
         "movq %rax, %xmm9\n"
         "movq %rax, %xmm10\n"
         "movq %rax, %xmm11\n"
         "movq %rax, %xmm12\n"
         "movq %rax, %xmm13\n"
         "movq %rax, %xmm14\n"
         "movq %rax, %xmm15\n"
         "movq $86, %rbx\n"
         "movq $85, %rcx\n"
         "movq $85, %rdx\n"
         "movq $85, %rsi\n"
         "movq $85, %rdi\n"
         "movq $85, %rbp\n"
         "movq $85, %r8\n"
         "movq $85, %r9\n"
         "movq $85, %r10\n"
         "movq $85, %r11\n"
         "movq $85, %r12\n"
         "movq $85, %r13\n"
         "movq $85, %r14\n"
         "movq $85, %r15\n"
         "fldl (%rsp)\n"
         "cmpq %rbx, %rax\n"
         "ret");
#endif

int main(void)
{
    struct sigaction_linux act = {0};

    act.handler = on_ill;
    act.flags = SA_SIGINFO | SA_RESTORER;
    act.restorer = restorer;
    if (sys(SYS_RT_SIGACTION, SIGILL, (long) &act, 0, 8) != 0) {
        return 100;
    }
#ifdef __x86_64__
    __asm__ (
        "push %rbx\n"
        "push %rbp\n"
        "push %r12\n"
        "push %r13\n"
        "push %r14\n"
        "push %r15\n"
        "leaq saved_rsp(%rip), %rax\n"
        "movq %rsp, (%rax)\n"
        "leaq fp_in(%rip), %rax\n"
        "fldl (%rax)\n"
        "movq $-100, %rax\n"
        "movq %rax, %xmm0\n"
        "movq $-101, %rax\n"
        "movq %rax, %xmm1\n"
        "movq $-102, %rax\n"
        "movq %rax, %xmm2\n"
        "movq $-103, %rax\n"
        "movq %rax, %xmm3\n"
        "movq $-104, %rax\n"
        "movq %rax, %xmm4\n"
        "movq $-105, %rax\n"
        "movq %rax, %xmm5\n"
        "movq $-106, %rax\n"
        "movq %rax, %xmm6\n"
        "movq $-107, %rax\n"
        "movq %rax, %xmm7\n"
        "movq $-108, %rax\n"
        "movq %rax, %xmm8\n"
        "movq $-109, %rax\n"
        "movq %rax, %xmm9\n"
        "movq $-110, %rax\n"
        "movq %rax, %xmm10\n"
        "movq $-111, %rax\n"
        "movq %rax, %xmm11\n"
        "movq $-112, %rax\n"
        "movq %rax, %xmm12\n"
        "movq $-113, %rax\n"
        "movq %rax, %xmm13\n"
        "movq $-114, %rax\n"
        "movq %rax, %xmm14\n"
        "movq $-115, %rax\n"
        "movq %rax, %xmm15\n"
        "movq $-1, %rax\n"
        "movq $-2, %rbx\n"
        "movq $-3, %rcx\n"
        "movq $-4, %rdx\n"
        "movq $-5, %rsi\n"
        "movq $-6, %rdi\n"
        "movq $-7, %rbp\n"
        "movq $-8, %r8\n"
        "movq $-9, %r9\n"
        "movq $-10, %r10\n"
        "movq $-11, %r11\n"
        "movq $-12, %r12\n"
        "movq $-13, %r13\n"
        "movq $-14, %r14\n"
        "movq $-15, %r15\n"
        "cmpq %rax, %rax\n"
        ".byte 0x0f, 0x0b # ud2\n"
        "push %rax\n"
        "leaq regs(%rip), %rax\n"
        "movq %rbx, 8(%rax)\n"
        "movq %rcx, 16(%rax)\n"
        "movq %rdx, 24(%rax)\n"
        "movq %rsi, 32(%rax)\n"
        "movq %rdi, 40(%rax)\n"
        "movq %rbp, 48(%rax)\n"
        "movq %r8, 56(%rax)\n"
        "movq %r9, 64(%rax)\n"
        "movq %r10, 72(%rax)\n"
        "movq %r11, 80(%rax)\n"
        "movq %r12, 88(%rax)\n"
        "movq %r13, 96(%rax)\n"
        "movq %r14, 104(%rax)\n"
        "movq %r15, 112(%rax)\n"
        "movq %rsp, 120(%rax)\n"
        "pop %rbx\n"
        "movq %rbx, (%rax)\n"
        "movq $0, %rbx\n"
        "setb %bl\n"
        "movq %rbx, 128(%rax)\n"
        "movq $0, %rbx\n"
        "sete %bl\n"
        "movq %rbx, 136(%rax)\n"
        "movq $0, %rbx\n"
        "setl %bl\n"
        "movq %rbx, 144(%rax)\n"
        "movq %xmm0, %rbx\n"
        "movq %rbx, 152(%rax)\n"
        "movq %xmm1, %rbx\n"
        "movq %rbx, 160(%rax)\n"
        "movq %xmm2, %rbx\n"
        "movq %rbx, 168(%rax)\n"
        "movq %xmm3, %rbx\n"
        "movq %rbx, 176(%rax)\n"
        "movq %xmm4, %rbx\n"
        "movq %rbx, 184(%rax)\n"
        "movq %xmm5, %rbx\n"
        "movq %rbx, 192(%rax)\n"
        "movq %xmm6, %rbx\n"
        "movq %rbx, 200(%rax)\n"
        "movq %xmm7, %rbx\n"
        "movq %rbx, 208(%rax)\n"
        "movq %xmm8, %rbx\n"
        "movq %rbx, 216(%rax)\n"
        "movq %xmm9, %rbx\n"
        "movq %rbx, 224(%rax)\n"
        "movq %xmm10, %rbx\n"
        "movq %rbx, 232(%rax)\n"
        "movq %xmm11, %rbx\n"
        "movq %rbx, 240(%rax)\n"
        "movq %xmm12, %rbx\n"
        "movq %rbx, 248(%rax)\n"
        "movq %xmm13, %rbx\n"
        "movq %rbx, 256(%rax)\n"
        "movq %xmm14, %rbx\n"
        "movq %rbx, 264(%rax)\n"
        "movq %xmm15, %rbx\n"
        "movq %rbx, 272(%rax)\n"
        "leaq fp_out(%rip), %rax\n"
        "fstpl (%rax)\n"
        "leaq saved_rsp(%rip), %rax\n"
        "movq (%rax), %rsp\n"
        "pop %r15\n"
        "pop %r14\n"
        "pop %r13\n"
        "pop %r12\n"
        "pop %rbp\n"
        "pop %rbx\n"
    );
#endif
    for (int i = 0; i < 15; i++) {
        if (regs[i] != -(i + 1)) {
            return i + 1;
        }
    }
    if (regs[15] != saved_rsp - 8) {
        return 16;
    }
    if (regs[16] != 0 || regs[17] != 1 || regs[18] != 0) {
        return 17;
    }
    for (int i = 0; i < 16; i++) {
        if (regs[19 + i] != -(100 + i)) {
            return 20 + i;
        }
    }
    if (fp_out != 2.5) {
        return 40;
    }
    return 0;
}
