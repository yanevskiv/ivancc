// (Test) Status: 42
// bug037. The assembler toggled its in-string state on every `"`, an escaped
// `\"` included, so a `#` or a `;` after one was taken for a comment or a
// separator, and `.ascii "a\"#b"` raised ERR_TXT_STRING_NOT_TERMINATED. Now
// the scanner reads a string as one token, its escapes included.

extern const char text[];

#ifdef __x86_64__
__asm__ (".data\n"
         ".globl text\n"
         "text: .ascii \"a\\\"#b;c\"\n"
         ".byte 0");
#endif

int main(void)
{
    const char *want = "a\"#b;c";

    for (int i = 0; want[i]; i++) {
        if (text[i] != want[i]) {
            return i + 1;
        }
    }
    return text[6] == '\0' ? 42 : 7;
}
