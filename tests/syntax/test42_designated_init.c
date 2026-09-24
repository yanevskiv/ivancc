// (Test) Return: 200
// Designated initializers for arrays: [i] = v places an element, and the
// elements after it carry on from that index.

int sparse[6] = {[2] = 7, [4] = 9};
int mixed[5] = {1, [3] = 4, 5};

int main()
{
    int desig[5] = {[1] = 8, [3] = 9};
    int follow[4] = {[1] = 2, 3};

    if (sparse[0] != 0 || sparse[1] != 0) return 1;
    if (sparse[2] != 7 || sparse[4] != 9) return 2;
    if (mixed[0] != 1 || mixed[3] != 4 || mixed[4] != 5) return 3;
    if (mixed[1] != 0 || mixed[2] != 0) return 4;

    if (desig[1] != 8 || desig[3] != 9) return 5;
    if (desig[0] != 0 || desig[2] != 0 || desig[4] != 0) return 6;
    if (follow[1] != 2 || follow[2] != 3) return 7;
    if (follow[0] != 0 || follow[3] != 0) return 8;

    if (sparse[2] + sparse[4] + desig[1] + follow[2] + 14 != 41) return 9;
    return 200;
}
