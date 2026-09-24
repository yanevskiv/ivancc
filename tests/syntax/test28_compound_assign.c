// (Test) Return: 200
// Every compound assignment, including on a narrow lvalue, and the rule that
// the target's address is evaluated once however complicated it is.

int bump(int *n)
{
    *n += 1;
    return 1;
}

int main()
{
    int a;
    int arr[4];
    int calls;
    char c;

    a = 10;
    a += 5;  if (a != 15) return 1;
    a -= 3;  if (a != 12) return 2;
    a *= 2;  if (a != 24) return 3;
    a /= 5;  if (a != 4) return 4;
    a %= 3;  if (a != 1) return 5;
    a <<= 4; if (a != 16) return 6;
    a >>= 2; if (a != 4) return 7;
    a |= 9;  if (a != 13) return 8;
    a &= 6;  if (a != 4) return 9;
    a ^= 7;  if (a != 3) return 10;

    c = 'A';
    c += 2;
    if (c != 'C') return 11;

    arr[0] = 3;
    arr[1] = 7;
    arr[1] += arr[0];
    if (arr[1] != 10) return 12;

    calls = 0;
    arr[bump(&calls)] += 100;
    if (calls != 1) return 13;

    if (arr[1] != 110) return 14;
    return 200;
}
