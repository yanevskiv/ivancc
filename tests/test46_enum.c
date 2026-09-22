// (Test) Return: 42
// Enumeration constants count from zero, and an explicit value moves the count
// to that point. The constants are ints wherever a name can appear.

enum Color { RED, GREEN, BLUE };
enum Status { OK = 10, BUSY, FAILED = 20, GONE };

int classify(enum Status s)
{
    switch (s) {
        case OK: {
            return 1;
        } break;
        case FAILED: {
            return 2;
        } break;
    }
    return 0;
}

int main()
{
    enum Color c = BLUE;
    int table[GONE];

    if (RED != 0 || GREEN != 1 || BLUE != 2) return 1;
    if (OK != 10 || BUSY != 11) return 2;
    if (FAILED != 20 || GONE != 21) return 3;
    if (sizeof(table) != 84) return 4;
    if (classify(OK) != 1 || classify(FAILED) != 2 || classify(BUSY) != 0) return 5;

    return c + FAILED + OK + GONE - 11;
}
