// (Test) Compiler flags: --MMD --MP --MF=- --MT=lang075_depend.o --MQ=$(OUT)/lang075_depend.o --include=h/lang075_depend_forced.h
// (Test) Compiler output:
// | lang075_depend.o $$(OUT)/lang075_depend.o: lang075_depend.c \
// |  h/lang075_depend_forced.h h/lang075_depend.h h/lang075_depend_nested.h
// | h/lang075_depend_forced.h:
// | h/lang075_depend.h:
// | h/lang075_depend_nested.h:
// (Test) Status: 200
// Dependency rules. --MMD writes a make rule naming the source and every header
// it read, and compiles as usual. --MF=- writes the rule to stdout. --MT names
// a target as given, and --MQ escapes it for make. --MP adds an empty rule for
// each header. A header is named once, however often it is included. A header
// read through --include comes right after the source. A line that grows past
// 72 columns breaks with a backslash.

#include "h/lang075_depend.h"
#include "h/lang075_depend.h"

int main()
{
    if (LANG075_FORCED != 100) return 1;
    if (LANG075_HEADER != 60) return 2;
    if (LANG075_NESTED != 40) return 3;

    return LANG075_FORCED + LANG075_HEADER + LANG075_NESTED;
}
