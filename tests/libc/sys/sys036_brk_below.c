// (Test) Status: 0
// A brk below the program leaves the break where it is and returns it, on the machine and in the emulator.

#define SYS_BRK 12

#define LOW 0x1000

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

    if (sys(SYS_BRK, LOW) != base) {
        return 1;
    }
    if (sys(SYS_BRK, 0) != base) {
        return 2;
    }
    return 0;
}
