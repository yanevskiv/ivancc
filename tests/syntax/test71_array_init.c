// (Test) Status: 200
// Arrays sized by their initializer. An array declared with [] takes its length
// from the highest index its initializer reaches. A string literal initializes
// an array of characters, braced or not, and sizes it with its terminator. A
// file-scope array that never gets a length has one element.

typedef int Row[];

struct Point {
    int x;
    int y;
};

struct Named {
    char name[6];
    int id;
};

int counts[] = {1, 2, 3};
int pairs[][2] = {1, 2, 3};
int sparse[] = {[5] = 7, [1] = 2};
struct Point points[] = {{1, 2}, {3, 4}, 5};
char word[] = "abc";
char padded[8] = "hi";
char exact[3] = "abc";
char braced[] = {"xy"};
char grid[2][4] = {"abc", "de"};
int wide[] = L"wide";
struct Named named = {"bob", 4};
const char *words[] = {"a", "bc"};
Row short_row = {1, 2};
Row long_row = {1, 2, 3, 4};
int later[];
int tentative[];
int later[] = {9, 8, 7, 6};

int sum(int *p, int n)
{
    int s = 0;

    for (int i = 0; i < n; i++) {
        s += p[i];
    }
    return s;
}

int second(int (*p)[])
{
    return (*p)[1];
}

int name_size(void)
{
    return sizeof(__func__);
}

int main()
{
    int local[] = {4, 5, 6, 7};
    char text[] = "local";
    static short kept[] = {1, 2, 3};
    char full[4] = "abcd";
    int *literal = (Row){10, 20, 30};

    if (sizeof(counts) != 12 || counts[2] != 3) return 1;
    if (sizeof(pairs) != 16 || pairs[1][0] != 3 || pairs[1][1] != 0) return 2;
    if (sizeof(sparse) != 24 || sparse[5] != 7 || sparse[1] != 2) return 3;
    if (sizeof(points) != 24 || points[2].x != 5 || points[2].y != 0) return 4;
    if (sizeof(word) != 4 || word[2] != 'c' || word[3] != 0) return 5;
    if (sizeof(padded) != 8 || padded[1] != 'i' || padded[2] != 0 || padded[7] != 0) return 6;
    if (exact[0] != 'a' || exact[2] != 'c') return 7;
    if (sizeof(braced) != 3 || braced[1] != 'y') return 8;
    if (grid[0][2] != 'c' || grid[1][1] != 'e' || grid[1][2] != 0) return 9;
    if (sizeof(wide) != 20 || wide[1] != 'i' || wide[4] != 0) return 10;
    if (named.name[2] != 'b' || named.name[3] != 0 || named.id != 4) return 11;
    if (sizeof(words) != 16 || words[1][1] != 'c') return 12;
    if (sizeof(short_row) != 8 || sizeof(long_row) != 16) return 13;
    if (sizeof(later) != 16 || sum(later, 4) != 30) return 14;
    if (sizeof(local) != 16 || sum(local, 4) != 22) return 15;
    if (sizeof(text) != 6 || text[4] != 'l' || text[5] != 0) return 16;
    if (sizeof(kept) != 6 || kept[2] != 3) return 17;
    if (full[3] != 'd') return 18;
    if (literal[2] != 30) return 19;
    if (second(&counts) != 2) return 20;
    if (name_size() != 10) return 21;

    tentative[0] = 5;
    if (tentative[0] != 5) return 22;
    return 200;
}
