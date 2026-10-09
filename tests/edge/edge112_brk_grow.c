// (Test) Status: 0
// A brk past the break returns the address asked for, unaligned, and maps zeroed memory up to the end of its page, on the machine and in the emulator.

#define SYS_BRK 12

#define PAGE 4096
#define GROW (PAGE + 123)

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
    long i;

    if (sys(SYS_BRK, base + GROW) != base + GROW) {
        return 1;
    }
    if (sys(SYS_BRK, 0) != base + GROW) {
        return 2;
    }
    for (i = 0; i < 2 * PAGE; i++) {
        if (heap[i] != 0) {
            return 3;
        }
        heap[i] = (char) i;
    }
    for (i = 0; i < 2 * PAGE; i++) {
        if (heap[i] != (char) i) {
            return 4;
        }
    }
    return 0;
}
