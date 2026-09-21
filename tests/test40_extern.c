// (Test) Return: 30
// extern declares without defining, so the definition later in the file is the
// one that counts and no second slot appears.

extern int shared;
extern int missing_is_fine;

int reader()
{
    return shared;
}

int shared = 12;

int main()
{
    if (shared != 12) return 1;
    if (reader() != 12) return 2;

    shared += 6;
    if (reader() != 18) return 3;

    return shared + 12;
}
