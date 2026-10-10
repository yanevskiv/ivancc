// (Test) Status: 0
// A rename from or to an unmapped path returns -EFAULT before it looks for the file, on the machine and in the emulator.

#define SYS_RENAME 82

#define EFAULT 14

#define UNMAPPED 8

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
    if (sys(SYS_RENAME, UNMAPPED, (long) PATH, 0) != -EFAULT) {
        return 1;
    }
    if (sys(SYS_RENAME, (long) PATH, UNMAPPED, 0) != -EFAULT) {
        return 2;
    }
    return 0;
}
