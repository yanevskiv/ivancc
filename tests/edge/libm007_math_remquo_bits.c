// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | 0
// | -7
// | 5
// | 0
// Non-standard: remquo should give the quotient's low 3 bits with its sign,
// never 8, where glibc's gives 8 for a quotient of 8 rounded up from 7.5.
// Liberty: n, at least 3, is implementation-defined, and so is quo's
// magnitude beyond its congruence.
// [C99:7.12.10.3p2]: remquo, the quotient's magnitude modulo 2^n, n >= 3

#include <math.h>
#include <stdio.h>

int main(void)
{
    int quo = 1;

    remquo(15.0, 2.0, &quo);
    printf("%d\n", quo);
    remquof(-7.0f, 1.0f, &quo);
    printf("%d\n", quo);
    remquol(0x1p60L, 3.0L, &quo);
    printf("%d\n", quo);
    remquo(-16.0, -2.0, &quo);
    printf("%d\n", quo);
    return 0;
}
