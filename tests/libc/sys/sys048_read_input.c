// (Test) Status: 0
// (Test) Output:
// | one line
// | and another, longer than the buffer
// (Test) Input:
// | one line
// | and another, longer than the buffer
// read takes standard input a few bytes at a time to its end, which write copies out, on the machine and in the emulator.

#define SYS_READ  0
#define SYS_WRITE 1

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
    char buf[4];
    long n;

    while ((n = sys(SYS_READ, 0, (long) buf, sizeof(buf))) > 0) {
        if (sys(SYS_WRITE, 1, (long) buf, n) != n) {
            return 1;
        }
    }
    return n == 0 ? 0 : 2;
}
