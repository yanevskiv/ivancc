// (Test) Status: 200
// Constant division by -1 and its neighbours in every type the folder divides
// in: int wraps through truncation, long wraps in the folder, a short
// promotes and does not wrap, and an unsigned divisor of all ones stays
// unsigned.

#define INT_MIN (-2147483647 - 1)
#define LONG_MIN (-9223372036854775807L - 1)

enum {
    INT_QUOTIENT   = INT_MIN / -1 == INT_MIN,
    INT_REMAINDER  = INT_MIN % -1,
    SHORT_PROMOTES = (short) -32768 / -1,
    PLAIN_QUOTIENT = 7 / -1 + -7 / -1,
    PLAIN_REMAINDER = 7 % -1 + -7 % -1,
    TRUNCATES      = -7 / 2 * 10 + -7 % 2
};

long ones_quotient = LONG_MIN / -1L / -1L;
long mixed = LONG_MIN / -1u;
unsigned long top_by_ones = (unsigned long) LONG_MIN / (unsigned long) -1;
unsigned long ones_by_ones = (unsigned long) -1 / (unsigned long) -1;
unsigned long ones_remainder = (unsigned long) LONG_MIN % (unsigned long) -1;
int int_wrapped = -(INT_MIN / -1);

int main()
{
    if (INT_QUOTIENT != 1 || INT_REMAINDER != 0 || SHORT_PROMOTES != 32768) return 1;
    if (PLAIN_QUOTIENT != 0 || PLAIN_REMAINDER != 0 || TRUNCATES != -31) return 2;
    if (ones_quotient != LONG_MIN || mixed != -2147483648L) return 3;
    if (top_by_ones != 0 || ones_by_ones != 1) return 4;
    if (ones_remainder != 9223372036854775808ul || int_wrapped != INT_MIN) return 5;
    return 200;
}
