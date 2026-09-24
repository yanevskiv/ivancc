// (Test) Return: 200
// A flexible array member names the storage that follows a struct without
// taking any of its own, so sizeof stops at the member before it.

struct Buf {
    int  len;
    char data[];
};

struct Wide {
    char tag;
    int  vals[];
};

char store[64];
char other[64];

int main()
{
    struct Buf  *b = (struct Buf *) store;
    struct Wide *w = (struct Wide *) other;

    if (sizeof(struct Buf) != 4) return 1;
    if (sizeof(struct Wide) != 4) return 2;

    b->len = 3;
    b->data[0] = 'a';
    b->data[1] = 'b';
    b->data[2] = 'c';
    if (b->data[0] != 'a' || b->data[2] != 'c') return 3;

    w->tag = 1;
    w->vals[0] = 30;
    w->vals[1] = 9;
    if (w->tag != 1) return 4;

    if (b->len + w->vals[0] + w->vals[1] != 42) return 5;
    return 200;
}
