// (Test) Status: 0
// A test's program runs in a directory of its own, without its source, and leaves a file there that a directory shared with another pipeline would already hold, on the machine and in the emulator.

#define SYS_OPEN  2
#define SYS_CLOSE 3
#define SYS_LSTAT 6

#define O_WRONLY 01
#define O_CREAT  0100
#define O_EXCL   0200

#define MODE 0644

#define ENOENT 2

#define SOURCE "edge145_run_dir.c"
#define PATH   "file"

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
    long fd = 0;
    unsigned long buf[18];

    if (sys(SYS_LSTAT, (long) SOURCE, (long) buf, 0) != -ENOENT) {
        return 1;
    }
    fd = sys(SYS_OPEN, (long) PATH, O_WRONLY | O_CREAT | O_EXCL, MODE);
    return fd >= 0 && sys(SYS_CLOSE, fd, 0, 0) == 0 ? 0 : 2;
}
