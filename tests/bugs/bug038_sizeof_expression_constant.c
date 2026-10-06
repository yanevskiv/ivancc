// (Test) Status: 0
// bug038. A sizeof of a constant expression that is not a single number was
// no constant, since the folder asked the operand for a type Sem had not yet
// given it. `char a[sizeof(1 + 1)];` was refused with
// ERR_PAR_ARRAY_LEN_NOT_CONSTANT, and so was `sizeof(sizeof 0)`, where gcc
// folds both.

char sum[sizeof(1 + 1)];
char wide[sizeof(1L + 1)];
char nested[sizeof(sizeof 0)];
char cond[sizeof(1 ? 2 : 3L)];

enum {
    SUM = sizeof(1 + 1),
    WIDE = sizeof(1L + 1),
    NESTED = sizeof(sizeof 0 + 1),
    SHIFT = sizeof(1 << 2L)
};

int main()
{
    if (sizeof(sum) != 4 || sizeof(wide) != 8) return 1;
    if (sizeof(nested) != 8 || sizeof(cond) != 8) return 2;
    if (SUM != 4 || WIDE != 8 || NESTED != 8 || SHIFT != 4) return 3;

    switch (8) {
        case sizeof(1L + 1): return 0;
    }
    return 4;
}
