// (Test) Status: 0
// bug_0042. A sizeof of a string literal or of a named object was no constant,
// since neither node has a type until Sem runs. `char a[sizeof("abc")];` was
// refused with ERR_PAR_ARRAY_LEN_NOT_CONSTANT, where gcc gives 4.

int nums[5];
long big;
char small;
struct pair { int first; char second; } pair;

char text[sizeof("abc")];
char bare[sizeof "abcd"];
char wide[sizeof(L"ab")];
char joined[sizeof("ab" "cd") - 1];
char whole[sizeof(nums)];
char plain[sizeof nums];
char record[sizeof(pair)];
char decayed[sizeof(nums + 1)];
char literal[sizeof("abc" + 1)];
char promoted[sizeof(small + 1)];
char negated[sizeof(-small)];
char widened[sizeof(small + big)];
char compared[sizeof(big == 1)];
char ordered[sizeof(big < small)];
char inverted[sizeof(! big)];
char either[sizeof(small || big)];
char picked[sizeof(small ? big : 2)];

typedef char check[sizeof(nums) == 5 * sizeof(int) && sizeof("abc") == 4 ? 1 : -1];

enum {
    TEXT = sizeof("abc"),
    WHOLE = sizeof(nums),
    RECORD = sizeof(pair)
};

struct bits {
    unsigned field : sizeof(small) + 2;
};

int main()
{
    int count = 3;
    unsigned ones = 15;
    char local[7];
    char sized[count];
    char after[sizeof(sized)];
    static char kept[sizeof(local)];
    static char named[] = { [sizeof("ab")] = 1 };
    struct bits bits = { 0 };

    if (sizeof(text) != 4 || sizeof(bare) != 5 || sizeof(wide) != 12 || sizeof(joined) != 4) return 1;
    if (sizeof(whole) != 20 || sizeof(plain) != 20 || sizeof(record) != 8) return 2;
    if (sizeof(decayed) != 8 || sizeof(literal) != 8) return 3;
    if (sizeof(promoted) != 4 || sizeof(negated) != 4 || sizeof(widened) != 8) return 4;
    if (sizeof(compared) != 4 || sizeof(ordered) != 4 || sizeof(inverted) != 4 || sizeof(either) != 4) return 5;
    if (sizeof(picked) != 8) return 6;
    if (TEXT != 4 || WHOLE != 20 || RECORD != 8) return 7;
    if (sizeof(kept) != 7 || sizeof(named) != 4 || named[3] != 1) return 8;
    if (sizeof(after) != 3) return 9;
    bits.field = ones;
    if (bits.field != 7) return 10;

    switch (4) {
        case sizeof("abc"): return 0;
    }
    return 11;
}
