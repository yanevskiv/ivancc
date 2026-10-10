// (Test) Status: 254
// An open of a missing file returns -ENOENT, which the program exits with, on the machine and in the emulator.

#define SYS_OPEN 2
#define SYS_EXIT 60

#define O_RDONLY 00

#define PATH "missing"

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
    sys(SYS_EXIT, sys(SYS_OPEN, (long) PATH, O_RDONLY, 0), 0, 0);
    return 1;
}
