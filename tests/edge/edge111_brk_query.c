// (Test) Status: 0
// A brk of 0 returns the program break, page-aligned, above the program's data and the same each time, on the machine and in the emulator.

#define SYS_BRK 12

#define PAGE 4096

long sys(long nr, long a);

static char data[16];

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi\n"
         "syscall\n"
         "ret");
#endif

int main(void)
{
    long first = sys(SYS_BRK, 0);
    long again = sys(SYS_BRK, 0);

    if (first % PAGE != 0) {
        return 1;
    }
    if (first <= (long) (data + sizeof(data))) {
        return 2;
    }
    if (again != first) {
        return 3;
    }
    return 0;
}
