// (Test) Status: 200
// Trigraphs. Each of the nine ??x sequences is replaced by the character it
// stands for, before anything else reads the source. So they work in
// directives, in code, and inside string and character literals. A trigraph
// backslash before a newline splices two lines, even at the end of a comment.
// Any other ?? stays as written.

??=define ANSWER 200
??=define JOIN(a, b) a ??=??= b
??=define STR(x) ??=x
??=define SUM 1 + ??/
    2

int same(const char *a, const char *b)
??<
    while (*a && *a == *b) ??<
        a++;
        b++;
    ??>
    return *a == *b;
??>

int main()
??<
    int a??(3??) = ??< 1, 2, 3 ??>;
    int x = 1;

    // Brackets and braces.
    if (a??(1??) != 2) return 1;
    if (sizeof(a) != 3 * sizeof(int)) return 2;

    // Caret, bar and tilde.
    if ((6 ??' 3) != 5) return 3;
    if ((4 ??! 1) != 5) return 4;
    if ((1 ??!??! 0) != 1) return 5;
    if (??-0 != -1) return 6;

    // Hash starts directives, stringizes and pastes.
??=if SUM != 3
    return 7;
??=endif
    if (JOIN(1, 2) != 12) return 8;
    if (! same(STR(a), "a")) return 9;

    // Literals hold the replaced characters.
    if (! same("??(??)??<??>", "[]{}")) return 10;
    if (! same("??=??'??!??-", "#^|~")) return 11;
    if (sizeof("??/n") != 2 || "??/n"[0] != '\n') return 12;
    if ('??/'' != '\'' || '??/??/' != '\\') return 13;
    if ('??=' != '#') return 14;

    // Anything else keeps its question marks.
    if (sizeof("??a") != 4 || sizeof("? ?=") != 5) return 15;
    if (! same("???=", "?#")) return 16;
    if (! same("????!", "??|")) return 17;

    // The comment below swallows the line after it.
    // It ends in a trigraph ??/
    x = 2;
    if (x != 1) return 18;

    return ANSWER;
??>
