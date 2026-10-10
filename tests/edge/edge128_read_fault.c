// (Test) Status: 242
// (Test) Input:
// | x
// A read of standard input into an unmapped buffer returns -EFAULT, which the program exits with, on the machine and in the emulator.

#define SYS_READ 0
#define SYS_EXIT 60

#define UNMAPPED 8

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
    sys(SYS_EXIT, sys(SYS_READ, 0, UNMAPPED, 1), 0, 0);
    return 1;
}
