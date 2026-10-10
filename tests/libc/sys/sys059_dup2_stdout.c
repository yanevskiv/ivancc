// (Test) Status: 0
// (Test) Output:
// | seen: hidden
// dup2 sends standard output to a file and back, on the machine and in the emulator.

#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_LSEEK 8
#define SYS_DUP2  33

#define O_RDWR  02
#define O_CREAT 0100
#define O_TRUNC 01000

#define SEEK_SET 0

#define MODE 0644

#define SAVED 20

#define PATH "file"

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
    long fd = sys(SYS_OPEN, (long) PATH, O_RDWR | O_CREAT | O_TRUNC, MODE);
    char buf[16];
    long n;

    if (fd < 0 || sys(SYS_DUP2, 1, SAVED, 0) != SAVED || sys(SYS_DUP2, fd, 1, 0) != 1) {
        return 1;
    }
    if (sys(SYS_WRITE, 1, (long) "hidden\n", 7) != 7) {
        return 2;
    }
    if (sys(SYS_DUP2, SAVED, 1, 0) != 1 || sys(SYS_LSEEK, fd, 0, SEEK_SET) != 0) {
        return 3;
    }
    n = sys(SYS_READ, fd, (long) buf, sizeof(buf));
    if (n != 7) {
        return 4;
    }
    if (sys(SYS_WRITE, 1, (long) "seen: ", 6) != 6 || sys(SYS_WRITE, 1, (long) buf, n) != n) {
        return 5;
    }
    return 0;
}
