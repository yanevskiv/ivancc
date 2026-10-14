// Declarations lang069_include_guard reads through a guarded include.

#ifndef LANG069_INCLUDE_GUARD_H
#define LANG069_INCLUDE_GUARD_H

#include "lang069_include_guard.h"

struct Point { int x; int y; };
typedef struct Point Point;

int point_sum(Point p)
{
    return p.x + p.y;
}

#endif // LANG069_INCLUDE_GUARD_H
