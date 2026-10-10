// (Test) Status: 0
// open creates a file that a second open reads back to its end, and unlink removes it, on the machine and in the emulator.

#define SYS_READ   0
#define SYS_WRITE  1
#define SYS_OPEN   2
#define SYS_CLOSE  3
#define SYS_UNLINK 87

#define O_RDONLY 00
#define O_WRONLY 01
#define O_CREAT  0100
#define O_TRUNC  01000

#define MODE 0644

#define ENOENT 2

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
    long fd = sys(SYS_OPEN, (long) PATH, O_WRONLY | O_CREAT | O_TRUNC, MODE);
    char buf[8] = {0};

    if (fd < 0) {
        return 1;
    }
    if (sys(SYS_WRITE, fd, (long) "abc", 3) != 3) {
        return 2;
    }
    if (sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 3;
    }
    fd = sys(SYS_OPEN, (long) PATH, O_RDONLY, 0);
    if (fd < 0) {
        return 4;
    }
    if (sys(SYS_READ, fd, (long) buf, sizeof(buf)) != 3 || buf[0] != 'a' || buf[2] != 'c') {
        return 5;
    }
    if (sys(SYS_READ, fd, (long) buf, sizeof(buf)) != 0) {
        return 6;
    }
    if (sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 7;
    }
    if (sys(SYS_UNLINK, (long) PATH, 0, 0) != 0) {
        return 8;
    }
    if (sys(SYS_OPEN, (long) PATH, O_RDONLY, 0) != -ENOENT) {
        return 9;
    }
    return 0;
}
