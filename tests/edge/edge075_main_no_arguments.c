// (Test) Status: 0
// With no arguments, main sees argc of 1, argv[0] set and argv[1] null.

int main(int argc, char **argv)
{
    if (argc != 1) return 1;
    if (argv[0] == 0) return 2;
    if (argv[0][0] == '\0') return 3;
    if (argv[1] != 0) return 4;
    return 0;
}
