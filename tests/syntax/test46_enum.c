// (Test) Return: 200
// Enumeration constants count from zero, and an explicit value moves the count
// to that point. The constants are ints wherever a name can appear, and any
// constant expression may set one, including an earlier constant.

enum Color { RED, GREEN, BLUE };
enum Status { OK = 10, BUSY, FAILED = 20, GONE };
enum Signed { BELOW = -2, ZERO, ABOVE };
enum Built { BASE = 5 * 8, NEXT = BASE + 2, WIDE = sizeof(int) };

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
    if (BELOW != -2 || ZERO != -1 || ABOVE != 0) return 6;
    if (BASE != 40 || NEXT != 42 || WIDE != 4) return 7;
    if (classify(OK) != 1 || classify(FAILED) != 2 || classify(BUSY) != 0) return 5;

    if (c + FAILED + OK + GONE - 11 != 42) return 8;
    return 200;
}
