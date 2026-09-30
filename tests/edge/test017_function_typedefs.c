// (Test) Status: 200
// Typedefs of function types declare prototypes at file and block scope,
// adjust to pointers as parameters, and build pointers, arrays of pointers,
// casts and further typedefs.

typedef int Unary(int);
typedef long Binary(long, long);
typedef Binary *BinaryPtr;
typedef void Sink(void);
typedef int Format(const char *, ...);

Unary inc, dec;
extern Binary add;
Format count;

int inc(int x) { return x + 1; }
int dec(int x) { return x - 1; }
long add(long a, long b) { return a + b; }
int count(const char *s, ...) { return s[0] - '0'; }

int hits;
void touch(void) { hits++; }

int call(Unary f, int x) { return f(x); }
long fold(BinaryPtr f, long a, long b) { return f(a, b); }

int main()
{
    Unary *table[2] = { inc, dec };
    Sink touch;
    Sink *s = touch;
    typedef Unary *Ptr;
    Ptr p = (Unary *) inc;

    if (table[0](5) != 6 || table[1](5) != 4) return 1;
    if (call(inc, 1) != 2 || call(dec, 1) != 0) return 2;
    if (fold(add, 40, 2) != 42) return 3;
    s();
    touch();
    if (hits != 2) return 4;
    if (p(9) != 10 || sizeof(Ptr) != 8) return 5;
    if (count("7", 1, 2) != 7) return 6;
    return 200;
}
