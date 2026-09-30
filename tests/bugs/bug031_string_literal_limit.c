// (Test) Status: 200
// bug031. A translation unit held at most 1024 string literals, so a program
// with 2000 of them was refused.

#define L1 sizeof "ab" +
#define L10 L1 L1 L1 L1 L1 L1 L1 L1 L1 L1
#define L100 L10 L10 L10 L10 L10 L10 L10 L10 L10 L10
#define L1000 L100 L100 L100 L100 L100 L100 L100 L100 L100 L100

int main()
{
    unsigned long total = L1000 L1000 0;

    return total == 6000 ? 200 : 1;
}
