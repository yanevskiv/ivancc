// (Test) Return: 200
// Array initializers, at file scope and inside a function. What a list leaves
// out is zero, which for a local means the code has to write those zeros.

int table[5] = {10, 20, 30};
char letters[4] = {'a', 'b'};

int main()
{
    int local[4] = {1, 2, 3, 4};
    int partial[4] = {5, 6};
    int total = 0;

    if (table[0] != 10 || table[2] != 30) return 1;
    if (table[3] != 0 || table[4] != 0) return 2;
    if (letters[0] != 'a' || letters[1] != 'b' || letters[2] != 0) return 3;

    for (int i = 0; i < 4; i++) {
        total += local[i];
    }
    if (total != 10) return 4;

    if (partial[0] != 5 || partial[1] != 6) return 5;
    if (partial[2] != 0 || partial[3] != 0) return 6;

    if (total + table[1] + 12 != 42) return 7;
    return 200;
}
