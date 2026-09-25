// Declarations test69_include_guard reads through a guarded include.

#ifndef TEST69_INCLUDE_GUARD_H
#define TEST69_INCLUDE_GUARD_H

#include "test69_include_guard.h"

struct Point { int x; int y; };
typedef struct Point Point;

int point_sum(Point p)
{
    return p.x + p.y;
}

#endif // TEST69_INCLUDE_GUARD_H
