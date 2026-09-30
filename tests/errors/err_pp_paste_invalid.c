// (Test) Compiler error: [ERR_PP_PASTE_INVALID]
// Can't paste `+` and `-` into one token.

#define F(a, b) a ## b

int y = F(+, -) 1;
