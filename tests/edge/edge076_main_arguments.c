// (Test) Status: 0
// (Test) Arguments: one two three
// Each argument reaches main in order, and argv ends with a null.

int same(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

int main(int argc, char **argv)
{
    if (argc != 4) return 1;
    if (! same(argv[1], "one")) return 2;
    if (! same(argv[2], "two")) return 3;
    if (! same(argv[3], "three")) return 4;
    if (argv[4] != 0) return 5;
    return 0;
}
