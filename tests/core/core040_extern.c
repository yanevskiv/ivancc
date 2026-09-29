// (Test) Status: 200
// extern declares without defining, so the definition later in the file is the
// one that counts and no second slot appears. A later declaration keeps what an
// earlier one settled, and extern inside a block names the file-scope object.

extern int shared;
extern int missing_is_fine;

int reader()
{
    return shared;
}

int shared = 12;

int defined = 5;
int defined;
extern int defined;

int tentative;
int tentative = 9;

static int hidden = 3;
extern int hidden;

extern int initialized = 4;

int lengths[];
int lengths[] = { 1, 2, 3 };

int inner()
{
    int shared = 1;
    {
        extern int shared;
        return shared + 1;
    }
}

int main()
{
    if (shared != 12) return 1;
    if (reader() != 12) return 2;

    shared += 6;
    if (reader() != 18) return 3;

    if (shared + 12 != 30) return 4;

    if (defined != 5) return 5;
    if (tentative != 9) return 6;
    if (hidden != 3) return 7;
    if (initialized != 4) return 8;
    if (sizeof lengths != 3 * sizeof(int)) return 9;
    if (lengths[2] != 3) return 10;
    if (inner() != 19) return 11;
    return 200;
}
