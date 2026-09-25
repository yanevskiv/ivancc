// (Test) Status: 200
// (Test) Compiler flags: --MMD --MP --MF=- --MT=test75_depend.o --MQ=$(OUT)/test75_depend.o --include=inc/test75_depend_forced.h
// (Test) Compiler output:
// | test75_depend.o $$(OUT)/test75_depend.o: test75_depend.c \
// |  inc/test75_depend_forced.h inc/test75_depend.h \
// |  inc/test75_depend_nested.h
// | inc/test75_depend_forced.h:
// | inc/test75_depend.h:
// | inc/test75_depend_nested.h:
// Dependency rules. --MMD writes a make rule naming the source and every header
// it read, and compiles as usual. --MF=- writes the rule to stdout. --MT names a
// target as given, and --MQ escapes it for make. --MP adds an empty rule for
// each header. A header is named once, however often it is included. A header
// read through --include comes right after the source. A line that grows past
// 72 columns breaks with a backslash.

#include "inc/test75_depend.h"
#include "inc/test75_depend.h"

int main()
{
    if (TEST75_FORCED != 100) return 1;
    if (TEST75_HEADER != 60) return 2;
    if (TEST75_NESTED != 40) return 3;

    return TEST75_FORCED + TEST75_HEADER + TEST75_NESTED;
}
