// (Test) Compiler flags: -lm
// (Test) Status: 0
// (Test) Output:
// | fmax 0 0
// | fmin 1 1
// | fmaxf 0 0
// | fminl 1 1
// Non-standard: fmax and fmin should order -0 below +0, in either order.
// Liberty: C99 asks it only ideally, and glibc's give either zero by type.
// [C99:F.9.9.2p2]: fmax, its footnote 323 asking +0 of -0.0 and +0.0 ideally
// [C99:F.9.9.3p1]: fmin, as fmax

#include <math.h>
#include <stdio.h>

int main(void)
{
    printf("fmax %d %d\n", signbit(fmax(-0.0, 0.0)) != 0, signbit(fmax(0.0, -0.0)) != 0);
    printf("fmin %d %d\n", signbit(fmin(-0.0, 0.0)) != 0, signbit(fmin(0.0, -0.0)) != 0);
    printf("fmaxf %d %d\n", signbit(fmaxf(-0.0f, 0.0f)) != 0, signbit(fmaxf(0.0f, -0.0f)) != 0);
    printf("fminl %d %d\n", signbit(fminl(-0.0L, 0.0L)) != 0, signbit(fminl(0.0L, -0.0L)) != 0);
    return 0;
}
