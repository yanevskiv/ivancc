// (Test) Status: 200
// Digraphs. <: :> <% %> %: and %:%: act as [ ] { } # and ##. They start
// directives, stringize, paste and bracket code like the tokens they stand
// for. Unlike trigraphs they are tokens, not text. So a literal keeps them as
// written, and a stringized digraph keeps its spelling.

%:define ANSWER 200
%:define JOIN(a, b) a %:%: b
%:define STR(x) %:x
%:define XSTR(x) STR(x)
%:define HASH %:

int same(const char *a, const char *b)
<%
    while (*a && *a == *b) <%
        a++;
        b++;
    %>
    return *a == *b;
%>

int main()
<%
    int a<:3:> = <% 1, 2, 3 %>;
    int b<::> = <% 4, 5 %>;

    // Brackets and braces.
    if (a<:1:> != 2) return 1;
    if (sizeof(b) != 2 * sizeof(int)) return 2;
    if (a JOIN(<, :) 2 JOIN(:, >) != 3) return 3;

    // Hash starts directives, stringizes and pastes.
%:
%:if ANSWER != 200
    return 4;
%:elif defined(JOIN)
    int joined = JOIN(1, 2);
%:else
    return 5;
%:endif
    if (joined != 12) return 6;
    if (! same(STR(a), "a")) return 7;

    // A stringized digraph keeps its spelling.
    if (! same(STR(<: :> <% %>), "<: :> <% %>")) return 8;
    if (! same(XSTR(HASH), "%:")) return 9;
    if (! same(STR(%: %:%: # ##), "%: %:%: # ##")) return 10;
    if (! same(STR("<:"), "\"<:\"")) return 11;

    // Literals keep digraphs as written.
    if (sizeof("<:%:%>") != 7) return 12;
    if ('%:' == '#') return 13;

    return ANSWER;
%>
