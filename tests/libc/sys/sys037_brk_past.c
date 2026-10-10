// (Test) Status: 0
// A brk into the stack, near the top of user space, leaves the break where it is and returns it, on the machine and in the emulator.

#define SYS_BRK 12

#define HIGH 0x7ffffffff000

long sys(long nr, long a);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi\n"
         "syscall\n"
         "ret");
#endif

int main(void)
{
    long base = sys(SYS_BRK, 0);

    if (sys(SYS_BRK, HIGH) != base) {
        return 1;
    }
    if (sys(SYS_BRK, 0) != base) {
        return 2;
    }
    return 0;
}
