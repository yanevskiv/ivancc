// (Test) Status: 0
// A test file for <_wfile.h> wide strings written and lines read.

// Include helpers.
#include "_util13_00_wfile_helpers.inc"

// A line should end at its new-line or at max characters, none at the end.
static void Test_WfileLines(void)
{
    const int text[] = { 'h', EURO, '\n', 'x', 'y', 'z', 0 };
    _File_Stream *stream = opened("", 0);
    int line[8];

    _TEST_TRUE(_Wfile_PutString(stream, text) >= 0);
    _TEST_INT(_File_SeekStream(stream, 0, _SYS_SEEK_SET), 0);
    _TEST_TRUE(_Wfile_GetLine(stream, line, 7) == line);
    _TEST_MEM(line, text, 3 * sizeof(int));
    _TEST_INT(line[3], 0);
    _TEST_TRUE(_Wfile_GetLine(stream, line, 2) == line);
    _TEST_INT(line[0], 'x');
    _TEST_INT(line[1], 'y');
    _TEST_INT(line[2], 0);
    _TEST_TRUE(_Wfile_GetLine(stream, line, 7) == line);
    _TEST_INT(line[0], 'z');
    _TEST_INT(line[1], 0);
    _TEST_TRUE(_Wfile_GetLine(stream, line, 7) != line);
}

int main(void)
{
    Test_WfileLines();
    return _Test_Status();
}
