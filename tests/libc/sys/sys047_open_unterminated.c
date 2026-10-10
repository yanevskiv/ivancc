// (Test) Status: 242
// An open of a path that runs to the end of the break's last page unterminated returns -EFAULT, which the program exits with, on the machine and in the emulator.

#define SYS_OPEN 2
#define SYS_BRK  12
#define SYS_EXIT 60

#define O_RDONLY 00

#define PAGE 4096
#define TAIL 4

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
    long base = sys(SYS_BRK, 0, 0, 0);
    long end = (base + PAGE + PAGE - 1) / PAGE * PAGE;
    char *tail = (char *) end - TAIL;

    if (sys(SYS_BRK, base + PAGE, 0, 0) != base + PAGE) {
        return 1;
    }
    for (int i = 0; i < TAIL; i++) {
        tail[i] = 'a';
    }
    sys(SYS_EXIT, sys(SYS_OPEN, (long) tail, O_RDONLY, 0), 0, 0);
    return 2;
}
