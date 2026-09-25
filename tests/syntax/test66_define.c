// (Test) Return: 200
// Object-like macros. A name defined with #define is replaced by its tokens
// wherever it appears, and the result is scanned again for more names. A macro
// is never replaced inside its own expansion, so a name that expands to itself
// stays as it is. #undef forgets a name, and an #include whose operand is a
// macro includes the file the macro expands to.

#define TEN 10
#define TWENTY (TEN * 2)
#define NEG -1
#define EMPTY
#define TYPE int
#define SIZE 4

#define HEADER "inc/test66_define.h"
#include HEADER

// A macro that names itself, or two that name each other, stop expanding.
#define count count
#define ping pong
#define pong ping

// An identical redefinition is allowed.
#define TEN 10

int count = 3;
int ping = 5;

int main()
{
    TYPE values[SIZE] = { TEN, TWENTY, 0, 0 };

    if (values[0] != 10) return 1;
    if (values[1] != 20) return 2;
    if (sizeof(values) != 16) return 3;

    // The expansion of NEG stays one token apart from the minus before it.
    if (-NEG != 1) return 4;

    // A macro may expand to nothing.
    EMPTY if (TEN EMPTY != 10) return 5;

    if (count != 3) return 6;
    if (ping != 5) return 7;
    if (from_header != 6) return 8;

#undef TEN
#define TEN 11
    if (TEN != 11) return 9;
    if (TWENTY != 22) return 10;

#undef TEN
    int TEN = 12;
    if (TEN != 12) return 11;

    return 200;
}
