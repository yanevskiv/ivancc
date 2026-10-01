// Declarations syntax069_include_guard reads through a guarded include.

#ifndef SYNTAX069_INCLUDE_GUARD_H
#define SYNTAX069_INCLUDE_GUARD_H

#include "syntax069_include_guard.h"

struct Point { int x; int y; };
typedef struct Point Point;

int point_sum(Point p)
{
    return p.x + p.y;
}

#endif // SYNTAX069_INCLUDE_GUARD_H
