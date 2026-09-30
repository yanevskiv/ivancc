// (Test) Status: 23
// A basic asm template: concatenated literals, statements split by newlines
// and `;`, `#` comments, a label with a statement after it, and a template
// run once per turn of a loop.

long counter;

int main(void)
{
#ifdef __x86_64__
    for (int i = 0; i < 5; i++) {
        __asm__ volatile ("leaq counter(%rip), %rcx; movq (%rcx), %rax # load\n"
                          "\taddq $2, %rax ; movq %rax, (%rcx)");
    }
    __asm__ ("leaq counter(%rip), %rcx\n"
             "movq (%rcx), %rax\n"
             "cmpq $10, %rax\n"
             "jne skip # not taken\n"
             "addq $13, %rax\n"
             "skip: movq %rax, (%rcx)");
#endif
    return (int) counter;
}
