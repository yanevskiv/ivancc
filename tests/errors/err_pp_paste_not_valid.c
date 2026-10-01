// (Test) Compiler error: [ERR_PP_PASTE_NOT_VALID]
// Can't paste `+` and `-` into one token.

#define F(a, b) a ## b

int y = F(+, -) 1;
