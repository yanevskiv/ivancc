// (Test) Return: 200
// Includes. A quoted include looks first in the directory of the file that
// names it, and the header's text stands where the directive stood. Everything
// the header declares is in scope below the directive, and the file can define
// what the header only declared.

int before = 1;

#include "inc/test65_include.h"

int pair_scale(Pair *p, int by)
{
    p->a = p->a * by;
    p->b = p->b * by;
    return p->a + p->b;
}

int main()
{
    Pair p = { 4, 5 };

    if (before != 1) return 1;
    if (pair_count != 3) return 2;
    if (pair_sum(p) != 9) return 3;
    if (pair_scale(&p, 2) != 18) return 4;
    if (p.a != 8) return 5;
    return 200;
}
