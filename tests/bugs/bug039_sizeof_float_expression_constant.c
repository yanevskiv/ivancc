// (Test) Status: 0
// bug039. A sizeof of a floating constant expression was no constant, since
// the folder is integral. `char a[sizeof(1.5 + 1)];` was refused with
// ERR_PAR_ARRAY_LEN_NOT_CONSTANT, where gcc gives 8.

char sum[sizeof(1.5 + 1)];
char single[sizeof(1.5F * 2)];
char wide[sizeof(1.5L - 1)];
char cond[sizeof(1 ? 2 : 3.0)];
char neg[sizeof(-1.5)];

enum {
    SUM = sizeof(1.5 + 1),
    SINGLE = sizeof(1.5F * 2),
    WIDE = sizeof(1.5L - 1),
    CAST = sizeof((float) 1 + 1.5F)
};

int main()
{
    if (sizeof(sum) != 8 || sizeof(single) != 4) return 1;
    if (sizeof(wide) != 16 || sizeof(cond) != 8 || sizeof(neg) != 8) return 2;
    if (SUM != 8 || SINGLE != 4 || WIDE != 16 || CAST != 4) return 3;

    switch (8) {
        case sizeof(1.5 + 1): return 0;
    }
    return 4;
}
