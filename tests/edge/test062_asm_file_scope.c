// (Test) Status: 42
// A basic asm at file scope: a function and its data written in assembly,
// called and read from C, with `;` and `#` inside a string left alone.

long twice(long x);
extern char text[];

#ifdef __x86_64__
__asm__ (".data\n"
         ".globl text\n"
         "text: .ascii \"a;b#c\"\n"
         ".text\n"
         ".globl twice\n"
         "twice: leaq (%rdi), %rax; addq %rdi, %rax\n"
         "ret");
#endif

int main(void)
{
    if (text[1] != ';' || text[3] != '#' || text[4] != 'c') {
        return 1;
    }
    return (int) twice(21);
}
