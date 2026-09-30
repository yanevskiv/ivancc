// (Test) Compiler error: [ERR_PAR_BODY_NOT_FUNCTION]
// A body after an object's declarator, following a function definition, is
// refused rather than replacing that function's body.

int f(void) { return 1; }

int x { return 2; }

int main()
{
    return f();
}
