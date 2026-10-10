// (Test) Status: 220
// An open takes a path of 4095 bytes and returns -ENAMETOOLONG for one of 4096, which the program exits with, on the machine and in the emulator.

#define SYS_OPEN  2
#define SYS_CLOSE 3
#define SYS_EXIT  60

#define O_RDONLY 00

#define PATH_MAX 4096

long sys(long nr, long a, long b, long c);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx\n"
         "syscall\n"
         "ret");
#endif

// The current directory as `./` repeated, a byte longer than the longest path.
char path[PATH_MAX + 1];

int main(void)
{
    long fd;

    for (int i = 0; i < PATH_MAX; i++) {
        path[i] = i % 2 ? '/' : '.';
    }
    path[PATH_MAX - 1] = 0;
    fd = sys(SYS_OPEN, (long) path, O_RDONLY, 0);
    if (fd < 0 || sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 1;
    }
    path[PATH_MAX - 1] = '/';
    sys(SYS_EXIT, sys(SYS_OPEN, (long) path, O_RDONLY, 0), 0, 0);
    return 2;
}
