// (Test) Compiler error: [ERR_SEM_CASE_DUPLICATE]
// Two case constants that differ only until they are converted to the
// promoted type of the controlling expression are duplicates.

int main()
{
    unsigned x = 0;

    switch (x) {
        case -1: return 1;
        case 4294967295u: return 2;
    }
    return 0;
}
