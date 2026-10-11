// (Test) Status: 0
// bug043. A postfix ++ or -- stored a _Bool's old value and one more or less
// as it was, so `flag++` on a true flag left 2 and `flag--` on a false one 255,
// and a step or a compound assignment of a _Bool bit-field kept the low bit of
// the sum, so `bits += 2` left 0. gcc converts each to 0 or 1 (S6.3.1.2p1).

struct holder {
    _Bool flag;
    _Bool first : 1;
    _Bool bits : 1;
    _Bool last : 1;
    unsigned three : 3;
};

_Bool global;

int main()
{
    _Bool flag = 0;
    _Bool list[3] = { 0, 1, 0 };
    _Bool *ptr = &flag;
    struct holder held = { 0, 1, 0, 1, 7 };
    unsigned char byte = 255;
    signed char small = 127;
    int got;

    flag++;
    if (flag != 1) return 1;
    flag++;
    if (flag != 1) return 2;
    flag--;
    if (flag != 0) return 3;
    flag--;
    if (flag != 1) return 4;

    flag = 1;
    got = flag++;
    if (got != 1 || flag != 1) return 5;
    got = flag--;
    if (got != 1 || flag != 0) return 6;
    got = flag--;
    if (got != 0 || flag != 1) return 7;

    (*ptr)++;
    if (flag != 1) return 8;
    list[1]++;
    list[2]--;
    if (list[0] != 0 || list[1] != 1 || list[2] != 1) return 9;
    global = 1;
    global++;
    if (global != 1) return 10;
    held.flag = 1;
    held.flag++;
    if (held.flag != 1) return 11;

    held.bits = 1;
    held.bits++;
    if (held.bits != 1) return 12;
    got = held.bits--;
    if (got != 1 || held.bits != 0) return 13;
    held.bits--;
    if (held.bits != 1) return 14;
    held.bits = 0;
    held.bits += 2;
    if (held.bits != 1) return 15;
    held.bits += 1;
    if (held.bits != 1) return 16;
    held.bits <<= 1;
    if (held.bits != 1) return 17;
    held.bits = 0;
    held.bits |= 4;
    if (held.bits != 1) return 18;
    ++held.bits;
    if (held.bits != 1) return 19;
    held.bits = 0;
    got = (held.bits -= 1);
    if (got != 1 || held.bits != 1) return 20;
    if (held.first != 1 || held.last != 1 || held.flag != 1) return 21;

    held.three++;
    if (held.three != 0) return 22;
    held.three--;
    if (held.three != 7) return 23;
    got = byte++;
    if (got != 255 || byte != 0) return 24;
    got = byte--;
    if (got != 0 || byte != 255) return 25;
    got = small--;
    if (got != 127 || small != 126) return 26;
    return 0;
}
