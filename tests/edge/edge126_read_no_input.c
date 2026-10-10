// (Test) Status: 0
// A read of standard input with no Input given is at its end at once, on the machine and in the emulator.

#define SYS_READ 0

long sys(long nr, long a, long b, long c);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx\n"
         "syscall\n"
         "ret");
#endif

int main(void)
{
    char buf[4];

    return sys(SYS_READ, 0, (long) buf, sizeof(buf)) == 0 ? 0 : 1;
}
