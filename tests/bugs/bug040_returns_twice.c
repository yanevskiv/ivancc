// (Test) Status: 0
// bug040. An operand pushed before a call was popped after it, so a call that
// returns twice, as setjmp does, found the slot overwritten the second time.
// `if (save(env) == 0)` compared with garbage and `int r = save(env);` stored
// through it, where gcc keeps both in registers or recomputes them.

long save(long *env);
void jump(long *env, int val);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl save\n"
         "save:\n"
         "mov %rbx, (%rdi)\n"
         "mov %rbp, 8(%rdi)\n"
         "mov %r12, 16(%rdi)\n"
         "mov %r13, 24(%rdi)\n"
         "mov %r14, 32(%rdi)\n"
         "mov %r15, 40(%rdi)\n"
         "lea 8(%rsp), %rdx\n"
         "mov %rdx, 48(%rdi)\n"
         "mov (%rsp), %rdx\n"
         "mov %rdx, 56(%rdi)\n"
         "xor %rax, %rax\n"
         "ret\n"
         ".globl jump\n"
         "jump:\n"
         "movslq %esi, %rax\n"
         "mov (%rdi), %rbx\n"
         "mov 8(%rdi), %rbp\n"
         "mov 16(%rdi), %r12\n"
         "mov 24(%rdi), %r13\n"
         "mov 32(%rdi), %r14\n"
         "mov 40(%rdi), %r15\n"
         "mov 48(%rdi), %rsp\n"
         "mov 56(%rdi), %rdx\n"
         "push %rdx\n"
         "ret");
#endif

long env[8];
long global;

// Jump back to env from a few calls down.
void deep(int n, int val)
{
    if (n == 0) {
        jump(env, val);
        return;
    }
    deep(n - 1, val);
}

int main(void)
{
    volatile int count = 0;
    long local;

    if (save(env) == 0) {
        count++;
        deep(3, 7);
    }
    if (count != 1) {
        return 1;
    }

    count = 0;
    if (save(env) != -1) {
        count++;
        deep(3, -1);
    }
    if (count != 1) {
        return 2;
    }

    count = 0;
    while (save(env) < 3) {
        count++;
        deep(2, count);
    }
    if (count != 3) {
        return 3;
    }

    count = 0;
    local = save(env);
    if (local == 0) {
        count++;
        deep(4, 5);
    }
    if (count != 1 || local != 5) {
        return 4;
    }

    count = 0;
    global = save(env);
    if (global == 0) {
        count++;
        deep(4, 6);
    }
    if (count != 1 || global != 6) {
        return 5;
    }
    return 0;
}
