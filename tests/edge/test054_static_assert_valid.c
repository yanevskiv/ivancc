// (Test) Status: 200
// Static assertions written as arrays that are sized 1 when they hold compile,
// as typedefs, externs, members and locals, and arrays of computed lengths
// keep their sizes.

typedef char check_long[sizeof(long) == 8 ? 1 : -1];
extern char check_int[sizeof(int) == 4 ? 1 : -1];
struct Checked { char ok[1 - 2 * (sizeof(short) != 2)]; int value; };

int main()
{
    char local[(int) sizeof(check_long) + 2];
    int grid[3][1 + 1];
    struct Checked c = { { 0 }, 7 };

    if (sizeof(local) != 3 || sizeof(grid) != 24) return 1;
    if (sizeof(check_long) != 1 || c.value != 7) return 2;
    return 200;
}
