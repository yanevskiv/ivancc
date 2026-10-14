// (Test) Status: 0
// bug045. An object of static storage duration was not aligned: gen wrote
// each one after the last with no padding, and `.data` and `.bss` had an
// alignment of 1, so after `char c;` the address of `int g;` was odd. Every
// such object sits on a multiple of its type's alignment [C99:3.2], a
// global, a file-scope static and a static local alike, with an initializer
// or without, and an array of 16 bytes or more on a multiple of 16, as the
// SysV ABI has it.

struct wide {
    char      wi_small;
    long long wi_large;
};

char        first = 1;
short       two = 2;
char        second = 1;
int         four = 4;
char        third = 1;
double      eight = 8.0;
char        fourth = 1;
long double sixteen = 16.0L;
char        fifth = 1;
struct wide record = { 1, 2 };
char        sixth = 1;
char        text[16] = "fifteen letters";
char        seventh = 1;
void       *pointer = &four;

char        bare_first;
int         bare_four;
char        bare_second;
double      bare_eight;
char        bare_third;
long double bare_sixteen;
char        bare_fourth;
struct wide bare_record;
char        bare_fifth;
char        bare_text[17];
char        bare_small[15];

static char   hidden_first = 1;
static long   hidden_eight = 8;
static char   hidden_bare;
static double hidden_bare_eight;

// Count the objects whose address is no multiple of their alignment.
static int misplaced(void)
{
    static char  local_first = 1;
    static long  local_eight = 8;
    static char  local_bare;
    static float local_bare_four;
    int          lost = 0;

    lost += (unsigned long) &two % 2 != 0;
    lost += (unsigned long) &four % 4 != 0;
    lost += (unsigned long) &eight % 8 != 0;
    lost += (unsigned long) &sixteen % 16 != 0;
    lost += (unsigned long) &record % 8 != 0;
    lost += (unsigned long) text % 16 != 0;
    lost += (unsigned long) &pointer % 8 != 0;
    lost += (unsigned long) &bare_four % 4 != 0;
    lost += (unsigned long) &bare_eight % 8 != 0;
    lost += (unsigned long) &bare_sixteen % 16 != 0;
    lost += (unsigned long) &bare_record % 8 != 0;
    lost += (unsigned long) bare_text % 16 != 0;
    lost += (unsigned long) &hidden_eight % 8 != 0;
    lost += (unsigned long) &hidden_bare_eight % 8 != 0;
    lost += (unsigned long) &local_eight % 8 != 0;
    lost += (unsigned long) &local_bare_four % 4 != 0;
    lost += first + second + third + fourth + fifth + sixth + seventh != 7;
    lost += hidden_first + local_first != 2;
    lost += bare_first + bare_second + bare_third + bare_fourth + bare_fifth != 0;
    lost += hidden_bare + local_bare + bare_small[14] != 0;
    return lost;
}

int main(void)
{
    if (two != 2 || four != 4 || eight != 8.0 || sixteen != 16.0L) {
        return 100;
    }
    if (record.wi_small != 1 || record.wi_large != 2 || text[14] != 's') {
        return 101;
    }
    if (pointer != &four || hidden_eight != 8 || bare_sixteen != 0.0L) {
        return 102;
    }
    return misplaced();
}
