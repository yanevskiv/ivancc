// (Test) Status: 0
// A test file for <setjmp.h> jmp_buf.

// Include helpers.
#include "libc12_00_setjmp_helpers.inc"

#define AFTER 77

// [C99:7.13p2] A jmp_buf should be an array, and take a jump in a struct too.
static void Test_SetjmpType(void)
{
    jmp_buf many[BUFS];
    volatile int count = 0;
    struct Holder holder;

    holder.ho_tag = 'h';
    holder.ho_after = AFTER;
    _TEST_TRUE(sizeof(jmp_buf) >= sizeof(env[0]));
    _TEST_UINT(sizeof(jmp_buf) % sizeof(env[0]), 0);
    _TEST_UINT(sizeof(holder.ho_env), sizeof(jmp_buf));
    _TEST_UINT(sizeof(many), BUFS * sizeof(jmp_buf));
    _TEST_TRUE(&env[0] == env);

    if (setjmp(holder.ho_env) == 0) {
        once(&count, holder.ho_env, 3, 2);
    }
    _TEST_INT(count, 1);
    _TEST_INT(holder.ho_tag, 'h');
    _TEST_INT(holder.ho_after, AFTER);

    count = 0;
    if (setjmp(many[1]) == 0) {
        once(&count, many[1], 3, 2);
    }
    _TEST_INT(count, 1);
}

int main(void)
{
    Test_SetjmpType();
    return _Test_Status();
}
