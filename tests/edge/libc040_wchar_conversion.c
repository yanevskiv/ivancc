// (Test) Status: 0
// (Test) Output:
// | C -1 -1 0 2 2 2
// | UTF-8 -2 -2 -1 0 -1 6 2 2
// Non-standard: in "C" a byte past ASCII should be no character and a tag
// character no bytes; in UTF-8 an overlong or surrogate sequence should be
// refused at its last byte alone, and a sequence of five bytes and a code
// past Unicode's last be refused, which glibc takes; a string conversion
// that fails should leave *src at the character it failed on.
// Liberty: [C99:7.24.6p3] the encoding is the locale's, and [C99:7.24.6.4] what
// *src holds after an encoding error is not said.

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

// Print how the locale named converts what C99 leaves to it.
static void show(const char *name)
{
    mbstate_t state;
    wchar_t wc = 0;
    char bytes[16];
    wchar_t wide[16];
    const char *text = "ab\xFF" "c";
    const char *src = text;
    static const wchar_t tags[] = {L'a', 0xE0041, L'c', 0};
    static const wchar_t none[] = {L'a', L'b', -1, L'c', 0};
    const wchar_t *wsrc = tags;

    printf("%s", name);
    memset(&state, 0, sizeof(state));
    printf(" %ld", (long) mbrtowc(&wc, "\xE0\x80", 2, &state));
    memset(&state, 0, sizeof(state));
    printf(" %ld", (long) mbrtowc(&wc, "\xED\xA0", 2, &state));
    memset(&state, 0, sizeof(state));
    if (strcmp(name, "C") != 0) {
        printf(" %ld", (long) mbrtowc(&wc, "\xF8\x88\x80\x80\x80", 5, &state));
        printf(" %x", (unsigned int) wc);
        printf(" %ld", (long) wcrtomb(bytes, 0x110000, &state));
    } else {
        printf(" %ld", (long) wcrtomb(bytes, 0xE0041, &state));
    }
    printf(" %ld", (long) wcsrtombs(bytes, &wsrc, sizeof(bytes), &state));
    memset(&state, 0, sizeof(state));
    mbsrtowcs(wide, &src, 16, &state);
    printf(" %ld", (long) (src - text));
    memset(&state, 0, sizeof(state));
    wsrc = none;
    wcsrtombs(bytes, &wsrc, sizeof(bytes), &state);
    printf(" %ld\n", (long) (wsrc - none));
}

int main(void)
{
    show("C");
    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        return 1;
    }
    show("UTF-8");
    return 0;
}
