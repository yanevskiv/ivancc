// Declarations core069_include_guard reads through a guarded include.

#ifndef CORE069_INCLUDE_GUARD_H
#define CORE069_INCLUDE_GUARD_H

#include "core069_include_guard.h"

struct Point { int x; int y; };
typedef struct Point Point;

int point_sum(Point p)
{
    return p.x + p.y;
}

#endif // CORE069_INCLUDE_GUARD_H
