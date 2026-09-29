// Functions test70_predefined calls to see a header's own file and lines.

const char *header_file(void)
{
    return __FILE__;
}

int header_line(void)
{
    return __LINE__;
}

#line 500 "renamed.h"
const char *renamed_file(void)
{
    return __FILE__;
}

int renamed_line(void)
{
    return __LINE__;
}
