// (Test) Status: 200
// Thousands of string literals, each an address in one static table, and more
// used in expressions, well past the old limits of 1024 literals per file and
// 256 addresses per object.

#define N1 "ab",
#define N10 N1 N1 N1 N1 N1 N1 N1 N1 N1 N1
#define N100 N10 N10 N10 N10 N10 N10 N10 N10 N10 N10
#define N1000 N100 N100 N100 N100 N100 N100 N100 N100 N100 N100
#define C1 "xyz"[1] +
#define C10 C1 C1 C1 C1 C1 C1 C1 C1 C1 C1

const char *names[3000] = { N1000 N1000 N1000 };

int main()
{
    int sum = C10 C10 C10 C10 C10 0;
    int count = 0;

    for (int i = 0; i < 3000; i++) count += names[i][1] == 'b';
    if (count != 3000) return 1;
    if (sum != 50 * 'y') return 2;
    return 200;
}
