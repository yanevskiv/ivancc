// (Test) Return: 43
// goto and labels, including a jump out of a loop and a backward jump that
// makes a loop of its own.

int search(int *arr, int n, int want)
{
    int i;

    for (i = 0; i < n; i++) {
        if (arr[i] == want) {
            goto found;
        }
    }
    return -1;

found:
    return i;
}

int main()
{
    int arr[4];
    int total;
    int i;

    arr[0] = 5;
    arr[1] = 7;
    arr[2] = 9;
    arr[3] = 11;

    if (search(arr, 4, 9) != 2) return 1;
    if (search(arr, 4, 42) != -1) return 2;

    total = 0;
    i = 0;
top:
    total += i;
    i++;
    if (i < 5) goto top;
    if (total != 10) return 3;

    goto skip;
    total = 999;
skip:

    return total + 33;
}
