// (Test) Compiler error: [ERR_PP_PASTE_INVALID]
// Pasting `+` and `-` makes no single token.

#define F(a, b) a ## b

int y = F(+, -) 1;

int main(void)
{
    return 0;
}
