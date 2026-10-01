// (Test) Status: 7
// A write of no bytes returns 0 though its buffer is unmapped.

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
    return 7 + (int) sys(1, 1, 0, 0);
}
