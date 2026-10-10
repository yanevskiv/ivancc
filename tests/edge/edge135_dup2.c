// (Test) Status: 0
// dup2 gives a descriptor a second number that shares its offset, keeps a number given twice, and fails a descriptor that is not open, on the machine and in the emulator.

#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_CLOSE 3
#define SYS_LSEEK 8
#define SYS_DUP2  33

#define O_RDWR  02
#define O_CREAT 0100
#define O_TRUNC 01000

#define SEEK_CUR 1

#define MODE 0644

#define EBADF 9

#define COPY   10
#define CLOSED 99

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

    if (fd < 0) {
        return 1;
    }
    if (sys(SYS_DUP2, fd, COPY, 0) != COPY) {
        return 2;
    }
    if (sys(SYS_WRITE, COPY, (long) "ab", 2) != 2) {
        return 3;
    }
    if (sys(SYS_LSEEK, fd, 0, SEEK_CUR) != 2) {
        return 4;
    }
    if (sys(SYS_DUP2, fd, fd, 0) != fd) {
        return 5;
    }
    if (sys(SYS_DUP2, CLOSED, COPY, 0) != -EBADF) {
        return 6;
    }
    if (sys(SYS_DUP2, CLOSED, CLOSED, 0) != -EBADF) {
        return 7;
    }
    if (sys(SYS_CLOSE, COPY, 0, 0) != 0 || sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 8;
    }
    return 0;
}
