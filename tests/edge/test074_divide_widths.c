// (Test) Status: 42
// Divisions at 32 and 16 bits take their dividend from %edx:%eax or
// %dx:%ax and leave a quotient and remainder of that width.

long idivl(int n, int d);
long ireml(int n, int d);
long divl(unsigned hi, unsigned lo, unsigned d);
long divw(unsigned hi, unsigned lo, unsigned d);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl idivl\n"
         "idivl: movl %edi, %eax\n"
         ".byte 0x99 # cltd\n"
         ".byte 0xf7, 0xfe # idivl %esi\n"
         ".byte 0x48, 0x63, 0xc0 # movslq %eax, %rax\n"
         "ret\n"
         ".globl ireml\n"
         "ireml: movl %edi, %eax\n"
         ".byte 0x99 # cltd\n"
         ".byte 0xf7, 0xfe # idivl %esi\n"
         ".byte 0x48, 0x63, 0xc2 # movslq %edx, %rax\n"
         "ret\n"
         ".globl divl\n"
         "divl: movl %edx, %ecx\n"
         "movl %edi, %edx\n"
         "movl %esi, %eax\n"
         ".byte 0xf7, 0xf1 # divl %ecx\n"
         "ret\n"
         ".globl divw\n"
         "divw: movl %edx, %ecx\n"
         "movl %edi, %edx\n"
         "movl %esi, %eax\n"
         ".byte 0x66, 0xf7, 0xf1 # divw %cx\n"
         "ret");
#endif

int main(void)
{
    if (idivl(-7, 2) != -3 || ireml(-7, 2) != -1) {
        return 1;
    }
    if (idivl(7, -2) != -3 || ireml(7, -2) != 1) {
        return 2;
    }
    if (idivl(-2147483647 - 1, 1) != -2147483647 - 1) {
        return 3;
    }
    if (divl(1, 5, 3) != 1431655767) {
        return 4;
    }
    if ((divw(1, 7, 3) & 0xffff) != 21847) {
        return 5;
    }
    return 42;
}
