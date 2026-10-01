// (Test) Status: 42
// (Test) Output:
// | asm
// Basic inline assembly: `__asm__`, optionally `volatile` or `inline`, and a
// template of one or more string literals, assembled where the statement
// stands. Here a `write` and an `exit` through `syscall`.

const char message[] = "asm\n";

int main(void)
{
#ifdef __x86_64__
    __asm__ volatile ("leaq message(%rip), %rsi\n\tmovq $1, %rdi\n\tmovq $4, %rdx\n\tmovq $1, %rax\n\tsyscall");
    __asm__ inline ("movq $42, %rdi\n\t" "movq $60, %rax\n\t" "syscall");
#endif
    return 0;
}
