// (Test) Status: 200
// bug032. An initialized object held at most 256 addresses, so a static table
// of 300 pointers was refused.

#define A1 &x,
#define A10 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1
#define A100 A10 A10 A10 A10 A10 A10 A10 A10 A10 A10

int x = 5;
int *table[300] = { A100 A100 A100 };

int main()
{
    int sum = 0;

    for (int i = 0; i < 300; i++) sum += *table[i];
    return sum == 1500 ? 200 : 1;
}
