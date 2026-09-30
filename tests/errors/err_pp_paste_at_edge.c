// (Test) Compiler error: [ERR_PP_PASTE_AT_EDGE]
// A replacement list that starts with `##`.

#define F(a) ## a

int main(void)
{
    return 0;
}
