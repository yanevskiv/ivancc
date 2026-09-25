// (Test) Return: 200
// Function-like macros. A macro name followed directly by a parenthesis takes
// arguments, and each parameter in its body is replaced by its argument, which
// is expanded first. # spells an argument as a string literal, ## pastes two
// tokens into one, and an empty argument pastes as nothing. A variadic macro
// gathers its extra arguments into __VA_ARGS__. A macro name with no
// parenthesis after it is left alone.

#define ADD(a, b) ((a) + (b))
#define ZERO() 0
#define NEGATE(x) -x
#define SQUARE(x) ((x) * (x))
#define STR(x) #x
#define XSTR(x) STR(x)
#define CAT(a, b) a ## b
#define TEN 10
#define OP ADD

#define FIRST(x, ...) x
#define REST(x, ...) __VA_ARGS__
#define CALL(fn, ...) fn(__VA_ARGS__)

// An identical redefinition is allowed.
#define ADD(a, b) ((a) + (b))

// A variable may share a name with a macro.
#define value(x) (x + 1)

// The expansion of f ends in g, which takes its argument from the text after it.
#define f(a) a * g
#define g(a) f(a)

int value = 7;
int g = 5;
int TEN1 = 4;

int add3(int a, int b, int c)
{
    return a + b + c;
}

int zero()
{
    return 0;
}

int main()
{
    int x = 1;
    int rest[3] = { REST(1, 2, 3, 4) };
    char *s = STR(a + b);
    char *spaced = STR(  a   +
        b  );

    if (ADD(1, 2) != 3) return 1;
    if (ADD(ADD(1, 2), ADD(3, 4)) != 10) return 2;
    if (ADD(add3(1, 2, 3), 1) != 7) return 3;
    if (ZERO() != 0) return 4;
    if (SQUARE(1 + 2) != 9) return 5;
    if (OP(TEN, TEN) != 20) return 6;
    if (ADD(1,
            2) != 3) return 7;

    // The expansion of NEGATE stays one token apart from the minus before it.
    if (-NEGATE(1) != 1) return 8;

    // Only the name before a parenthesis expands.
    if (value(value) != 8) return 9;

    // # spells its argument as written, with each gap as one space.
    if (s[0] != 'a' || s[1] != ' ' || s[2] != '+' || s[3] != ' ' || s[4] != 'b' || s[5] != 0) return 10;
    if (spaced[1] != ' ' || spaced[3] != ' ' || spaced[5] != 0) return 11;
    if (sizeof(STR()) != 1) return 12;
    if (sizeof(STR("q\n")) != 6) return 13;
    if (STR("q\n")[0] != '"' || STR("q\n")[2] != '\\') return 14;
    if (STR(TEN)[0] != 'T') return 15;
    if (XSTR(TEN)[0] != '1') return 16;

    // ## pastes its operands before they can expand.
    if (CAT(val, ue) != 7) return 17;
    if (CAT(1, 2) != 12) return 18;
    if (CAT(TE, N) != 10) return 19;
    if (CAT(TEN, 1) != 4) return 20;
    if (CAT(, 3) != 3) return 21;
    if (CAT(3, ) != 3) return 22;
    x CAT(+, =) 2;
    CAT(,) if (x != 3) return 23;

    // __VA_ARGS__ keeps the commas between the arguments it gathers.
    if (FIRST(5, 6, 7) != 5) return 24;
    if (rest[0] != 2 || rest[2] != 4) return 25;
    if (CALL(add3, 1, 2, 3) != 6) return 26;
    if (CALL(zero, ) != 0) return 27;

    // f(2)(9) becomes 2 * 9 * g, since the g it ends in may not call f again.
    if (f(2)(9) != 90) return 28;

    return 200;
}
