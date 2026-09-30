// (Test) Compiler warning: [ERR_PP_ONCE_IN_MAIN_FILE]
// (Test) Status: 0
// #pragma once in the file being compiled warns and does nothing else.

#pragma once

int main(void)
{
    return 0;
}
