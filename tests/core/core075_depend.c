// (Test) Status: 200
// (Test) Compiler flags: --MMD --MP --MF=- --MT=core075_depend.o --MQ=$(OUT)/core075_depend.o --include=h/core075_depend_forced.h
// (Test) Compiler output:
// | core075_depend.o $$(OUT)/core075_depend.o: core075_depend.c \
// |  h/core075_depend_forced.h h/core075_depend.h h/core075_depend_nested.h
// | h/core075_depend_forced.h:
// | h/core075_depend.h:
// | h/core075_depend_nested.h:
// Dependency rules. --MMD writes a make rule naming the source and every header
// it read, and compiles as usual. --MF=- writes the rule to stdout. --MT names a
// target as given, and --MQ escapes it for make. --MP adds an empty rule for
// each header. A header is named once, however often it is included. A header
// read through --include comes right after the source. A line that grows past
// 72 columns breaks with a backslash.

#include "h/core075_depend.h"
#include "h/core075_depend.h"

int main()
{
    if (CORE075_FORCED != 100) return 1;
    if (CORE075_HEADER != 60) return 2;
    if (CORE075_NESTED != 40) return 3;

    return CORE075_FORCED + CORE075_HEADER + CORE075_NESTED;
}
