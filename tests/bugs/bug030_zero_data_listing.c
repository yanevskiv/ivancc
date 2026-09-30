// (Test) Status: 200
// bug030. -S wrote a zero-filled object as one `.byte 0` line per byte, so a
// 3 MB buffer became 3 million lines that ivanas could not finish in time.

static char big[3000000];
char tail[4096] = { 1, [4095] = 2 };

int main()
{
    big[2999999] = 3;
    if (big[0] != 0 || big[2999999] != 3) return 1;
    if (tail[0] != 1 || tail[1] != 0 || tail[4095] != 2) return 2;
    return 200;
}
