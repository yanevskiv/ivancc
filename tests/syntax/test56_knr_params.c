// (Test) Return: 200
// The parameter list C had before prototypes: names in the parentheses, types
// in a declaration list between the `)` and the `{`. C90 gave any name the list
// left out the type int; C99 dropped that default, so every name needs a
// declaration. Empty parentheses say nothing about the parameters at all, which
// is a different type from `(void)` and the reason that spelling exists, and an
// argument no parameter type governs is promoted before the call.

// An old-style definition, typed by the list between the `)` and the `{`.
int add(a, b)
int a;
int b;
{
    return a + b;
}

// Several names in one declaration, and a pointer among them.
int pick(s, i, j)
char *s;
int i, j;
{
    return s[i] + s[j];
}

// C99 dropped the rule that an undeclared parameter is an int, so every name in
// the list needs a declaration. Leaving one out is an error, not a default.
int both(a, b)
int a;
int b;
{
    return a - b;
}

// A char parameter receives an argument the caller promoted to int.
int byte(c)
char c;
{
    return c;
}

// Empty parentheses on a definition also promise nothing about the parameters.
int seven()
{
    return 7;
}

// Declared with no parameter list, defined below with one.
int later();

int later(n)
int n;
{
    return n + n;
}

int main()
{
    char c;
    int r;

    if (add(20, 22) != 42) return 1;
    if (pick("AB", 0, 1) != 131) return 2;
    if (both(44, 2) != 42) return 3;
    if (seven() != 7) return 4;
    if (later(21) != 42) return 5;

    // The argument is a char, and the call promotes it on the way in.
    c = 'A';
    if (byte(c) != 65) return 6;

    r = add(1, 2);
    if (r != 3) return 7;

    if (add(add(10, 10), 22) != 42) return 8;
    return 200;
}
