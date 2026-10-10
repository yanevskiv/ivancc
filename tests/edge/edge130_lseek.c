// (Test) Status: 0
// (Test) Cleanup: edge130_lseek.tmp
// lseek moves a file's offset from its start, the offset and its end, and fails a negative offset, an unknown whence and a closed descriptor, on the machine and in the emulator.

#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_CLOSE 3
#define SYS_LSEEK 8

#define O_RDWR  02
#define O_CREAT 0100
#define O_TRUNC 01000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_BAD 99

#define MODE 0644

#define EBADF  9
#define EINVAL 22

#define CLOSED 99

#define PATH "edge130_lseek.tmp"

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
    char buf[4] = {0};

    if (fd < 0 || sys(SYS_WRITE, fd, (long) "abcdef", 6) != 6) {
        return 1;
    }
    if (sys(SYS_LSEEK, fd, 0, SEEK_CUR) != 6) {
        return 2;
    }
    if (sys(SYS_LSEEK, fd, 2, SEEK_SET) != 2) {
        return 3;
    }
    if (sys(SYS_READ, fd, (long) buf, 4) != 4 || buf[0] != 'c' || buf[3] != 'f') {
        return 4;
    }
    if (sys(SYS_LSEEK, fd, -1L, SEEK_END) != 5) {
        return 5;
    }
    if (sys(SYS_READ, fd, (long) buf, 4) != 1 || buf[0] != 'f') {
        return 6;
    }
    if (sys(SYS_LSEEK, fd, -1L, SEEK_SET) != -EINVAL) {
        return 7;
    }
    if (sys(SYS_LSEEK, fd, 0, SEEK_BAD) != -EINVAL) {
        return 8;
    }
    if (sys(SYS_LSEEK, CLOSED, 0, SEEK_SET) != -EBADF) {
        return 9;
    }
    return sys(SYS_CLOSE, fd, 0, 0) == 0 ? 0 : 10;
}
