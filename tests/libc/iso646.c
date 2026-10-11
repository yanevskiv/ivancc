// (Test) Status: 0
// <iso646.h> defines and, and_eq, bitand, bitor, compl, not, not_eq, or, or_eq, xor and xor_eq, each expanding to its token (S7.9p1),
// so each acts as the operator it spells: the logical ones leaving their right operand alone where the left decides (S6.5.13p4, S6.5.14p4), and binding and above or,
// the bitwise ones and compl on the bits, and the three assignments storing what they yield.

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>

#define TEXT(token)  #token
#define SPELL(token) TEXT(token)

#define SIX   6
#define THREE 3

static int calls;

// Count a call and return 1, to tell whether an operand was evaluated.
static int called(void)
{
    calls++;
    return 1;
}

// (S7.9p1) Each macro should expand to its operator's token and no more.
static void Test_Iso646Spellings(void)
{
    _TEST_STR(SPELL(and), "&&");
    _TEST_STR(SPELL(and_eq), "&=");
    _TEST_STR(SPELL(bitand), "&");
    _TEST_STR(SPELL(bitor), "|");
    _TEST_STR(SPELL(compl), "~");
    _TEST_STR(SPELL(not), "!");
    _TEST_STR(SPELL(not_eq), "!=");
    _TEST_STR(SPELL(or), "||");
    _TEST_STR(SPELL(or_eq), "|=");
    _TEST_STR(SPELL(xor), "^");
    _TEST_STR(SPELL(xor_eq), "^=");
}

// (S7.9p1) and, or and not should yield 0 or 1, and skip an operand unneeded.
static void Test_Iso646Logical(void)
{
    _TEST_INT(1 and 2, 1);
    _TEST_INT(1 and 0, 0);
    _TEST_INT(0 or 0, 0);
    _TEST_INT(0 or THREE, 1);
    _TEST_INT(not 0, 1);
    _TEST_INT(not THREE, 0);
    _TEST_INT(1 or 0 and 0, 1);
    _TEST_INT(not 1 or 1, 1);
    _TEST_INT(1 not_eq 1, 0);
    _TEST_INT(1 not_eq 2, 1);

    _TEST_INT(0 and called(), 0);
    _TEST_INT(1 or called(), 1);
    _TEST_INT(calls, 0);
    _TEST_INT(1 and called(), 1);
    _TEST_INT(0 or called(), 1);
    _TEST_INT(calls, 2);
}

// (S7.9p1) bitand, bitor, xor and compl should work on their operands' bits.
static void Test_Iso646Bitwise(void)
{
    _TEST_INT(SIX bitand THREE, 2);
    _TEST_INT(SIX bitor THREE, 7);
    _TEST_INT(SIX xor THREE, 5);
    _TEST_INT(compl 0, -1);
    _TEST_INT(compl SIX bitand THREE, 1);
    _TEST_INT(SIX xor THREE bitand 1, 7);
}

// (S7.9p1) and_eq, or_eq and xor_eq should store their result and yield it.
static void Test_Iso646Assignment(void)
{
    int value = SIX;

    value and_eq THREE;
    _TEST_INT(value, 2);
    value or_eq 5;
    _TEST_INT(value, 7);
    value xor_eq 2;
    _TEST_INT(value, 5);
    _TEST_INT(value and_eq 4, 4);
    _TEST_INT(value or_eq 1, 5);
    _TEST_INT(value xor_eq 5, 0);
    _TEST_INT(value, 0);
}

int main(void)
{
    Test_Iso646Spellings();
    Test_Iso646Logical();
    Test_Iso646Bitwise();
    Test_Iso646Assignment();
    return _Test_Status();
}
