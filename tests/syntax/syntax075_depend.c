// (Test) Status: 200
// (Test) Compiler flags: --MMD --MP --MF=- --MT=syntax075_depend.o --MQ=$(OUT)/syntax075_depend.o --include=h/syntax075_depend_forced.h
// (Test) Compiler output:
// | syntax075_depend.o $$(OUT)/syntax075_depend.o: syntax075_depend.c \
// |  h/syntax075_depend_forced.h h/syntax075_depend.h \
// |  h/syntax075_depend_nested.h
// | h/syntax075_depend_forced.h:
// | h/syntax075_depend.h:
// | h/syntax075_depend_nested.h:
// Dependency rules. --MMD writes a make rule naming the source and every header
// it read, and compiles as usual. --MF=- writes the rule to stdout. --MT names a
// target as given, and --MQ escapes it for make. --MP adds an empty rule for
// each header. A header is named once, however often it is included. A header
// read through --include comes right after the source. A line that grows past
// 72 columns breaks with a backslash.

#include "h/syntax075_depend.h"
#include "h/syntax075_depend.h"

int main()
{
    if (SYNTAX075_FORCED != 100) return 1;
    if (SYNTAX075_HEADER != 60) return 2;
    if (SYNTAX075_NESTED != 40) return 3;

    return SYNTAX075_FORCED + SYNTAX075_HEADER + SYNTAX075_NESTED;
}
