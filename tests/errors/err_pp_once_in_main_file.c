// (Test) Compiler warning: [ERR_PP_ONCE_IN_MAIN_FILE]
// (Test) Status: 0
// Shouldn't use #pragma once in the file being compiled.

#pragma once

int main(void)
{
    return 0;
}
