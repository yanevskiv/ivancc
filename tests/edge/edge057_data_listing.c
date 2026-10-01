// (Test) Status: 200
// Initialized data survives -S whatever its zero runs: runs just short of and
// just long enough for `.zero`, long runs of other bytes, 255s, padding,
// addresses between runs, and a large zero tail.

struct Padded { char c; long l; char d; };

int x = 7;
unsigned char pattern[64] = {
    1, 0, 0, 0, 0, 0, 0, 0, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    255, 0, 255
};
struct Padded padded[2] = { { 'a', 1, 'b' }, { 'c', -1, 'd' } };
int *mixed[3] = { 0, &x, 0 };
char big[100000] = { 9, [99999] = 8 };

int main()
{
    int sum = 0;

    for (int i = 0; i < 64; i++) sum += pattern[i];
    if (sum != 1 + 2 + 3 + 4 + 5 * 18 + 9 * 17 + 255 + 255) return 1;
    if (pattern[8] != 2 || pattern[18] != 4 || pattern[39] != 255 || pattern[63] != 0) return 2;
    if (padded[0].d != 'b' || padded[1].l != -1 || padded[1].d != 'd') return 3;
    if (mixed[0] || *mixed[1] != 7 || mixed[2]) return 4;
    if (big[0] != 9 || big[1] != 0 || big[50000] != 0 || big[99999] != 8) return 5;
    return 200;
}
