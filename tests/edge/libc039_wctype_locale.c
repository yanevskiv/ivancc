// (Test) Status: 0
// (Test) Output:
// | C 0 0 0 e9 c9 0
// | UTF-8 1 1 1 c9 e9 1
// | UTF-8 1 0 1 1 0 20ac 178
// | none 0 0 0 0 0
// Non-standard: a wide character past ASCII should be in no class and have
// no other case in "C", and Unicode's classes and cases in "C.UTF-8"; WEOF
// and a class no name describes should hold of nothing.
// Liberty: (S7.25.2.1.2p2) the letters past the basic ones are a set each
// locale chooses, as are the other classes and (S7.25.3.1) the cases.

#include <locale.h>
#include <stdio.h>
#include <wctype.h>

// Print how the locale named classes and maps an e with an acute accent.
static void show(const char *name)
{
    int lower = iswlower(0xE9) != 0;
    int upper = iswupper(0xC9) != 0;
    int alpha = iswalpha(0xE9) != 0;
    int space = iswspace(0x3000) != 0;

    printf("%s %d %d %d %x %x %d\n", name, alpha, lower, upper, towupper(0xE9), towlower(0xC9), space);
}

int main(void)
{
    show("C");
    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        return 1;
    }
    show("UTF-8");
    printf("UTF-8 %d %d %d ", iswalpha(0x4E2D) != 0, iswdigit(0x0660) != 0, iswpunct(0x20AC) != 0);
    printf("%d %d %x %x\n", iswcntrl(0x2028) != 0, iswprint(0x2028) != 0, towupper(0x20AC), towupper(0xFF));
    printf("none %d %d %d ", iswalpha(WEOF) != 0, iswcntrl(WEOF) != 0, iswprint(0x110000) != 0);
    printf("%d %d\n", iswctype(L'a', 0) != 0, iswctype(L'a', wctype("nonsense")) != 0);
    return 0;
}
