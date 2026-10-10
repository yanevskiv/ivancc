// (Test) Status: 139
// A store past the page the break ends in kills the program with SIGSEGV, on the machine and in the emulator.

#define SYS_BRK 12

#define PAGE 4096

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
    volatile char *heap = (volatile char *) base;

    if (sys(SYS_BRK, base + PAGE) != base + PAGE) {
        return 1;
    }
    heap[PAGE - 1] = 1;
    heap[PAGE] = 1;
    return 0;
}
