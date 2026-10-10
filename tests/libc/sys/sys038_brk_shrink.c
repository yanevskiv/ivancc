// (Test) Status: 0
// A brk below the break gives back the pages above the new break's, which read as zero when mapped again, and keeps the page it ends in, on the machine and in the emulator.

#define SYS_BRK 12

#define PAGE 4096
#define KEEP 10
#define FILL 0x5a

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

    if (sys(SYS_BRK, base + 3 * PAGE) != base + 3 * PAGE) {
        return 1;
    }
    for (i = 0; i < 3 * PAGE; i++) {
        heap[i] = FILL;
    }
    if (sys(SYS_BRK, base + KEEP) != base + KEEP) {
        return 2;
    }
    if (sys(SYS_BRK, base + 3 * PAGE) != base + 3 * PAGE) {
        return 3;
    }
    for (i = 0; i < KEEP; i++) {
        if (heap[i] != FILL) {
            return 4;
        }
    }
    for (i = PAGE; i < 3 * PAGE; i++) {
        if (heap[i] != 0) {
            return 5;
        }
    }
    return 0;
}
