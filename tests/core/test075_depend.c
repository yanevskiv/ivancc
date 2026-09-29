// (Test) Status: 200
// (Test) Compiler flags: --MMD --MP --MF=- --MT=test075_depend.o --MQ=$(OUT)/test075_depend.o --include=h/test075_depend_forced.h
// (Test) Compiler output:
// | test075_depend.o $$(OUT)/test075_depend.o: test075_depend.c \
// |  h/test075_depend_forced.h h/test075_depend.h h/test075_depend_nested.h
// | h/test075_depend_forced.h:
// | h/test075_depend.h:
// | h/test075_depend_nested.h:
// Dependency rules. --MMD writes a make rule naming the source and every header
// it read, and compiles as usual. --MF=- writes the rule to stdout. --MT names a
// target as given, and --MQ escapes it for make. --MP adds an empty rule for
// each header. A header is named once, however often it is included. A header
// read through --include comes right after the source. A line that grows past
// 72 columns breaks with a backslash.

#include "h/test075_depend.h"
#include "h/test075_depend.h"

int main()
{
    if (TEST075_FORCED != 100) return 1;
    if (TEST075_HEADER != 60) return 2;
    if (TEST075_NESTED != 40) return 3;

    return TEST075_FORCED + TEST075_HEADER + TEST075_NESTED;
}
