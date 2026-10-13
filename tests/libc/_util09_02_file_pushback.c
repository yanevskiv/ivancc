// (Test) Status: 0
// A test file for <_file.h> characters pushed back.

// Include helpers.
#include "_util09_00_file_helpers.inc"

// A stream should take back _FILE_BACKS bytes and give the last one first.
static void Test_FilePushback(void)
{
    _File_Stream *stream = opened("ab", 2);

    _TEST_INT(_File_UngetChar(stream, 'x'), 'x');
    _TEST_INT(_File_UngetChar(stream, 0xFF), 0xFF);
    _TEST_INT(_File_PeekChar(stream), 0xFF);
    _TEST_INT(_File_GetChar(stream), 0xFF);
    _TEST_INT(_File_GetChar(stream), 'x');
    _TEST_INT(_File_GetChar(stream), 'a');
    _TEST_INT(_File_UngetChar(stream, _FILE_END), _FILE_END);
    _TEST_INT(_File_UngetChar(stream, '1'), '1');
    _TEST_INT(_File_UngetChar(stream, '2'), '2');
    _TEST_INT(_File_UngetChar(stream, '3'), '3');
    _TEST_INT(_File_UngetChar(stream, '4'), '4');
    _TEST_UINT(stream->fs_backs, _FILE_BACKS);
    _TEST_INT(_File_UngetChar(stream, '5'), _FILE_END);
    _TEST_INT(_File_GetChar(stream), '4');
    _TEST_INT(_File_SeekStream(stream, 1, _SYS_SEEK_SET), 0);
    _TEST_UINT(stream->fs_backs, 0);
    _TEST_INT(_File_GetChar(stream), 'b');
    _TEST_INT(_File_GetChar(stream), _FILE_END);
    _TEST_TRUE((stream->fs_flags & _FILE_EOF) != 0);
    _TEST_INT(_File_UngetChar(stream, 'z'), 'z');
    _TEST_TRUE((stream->fs_flags & _FILE_EOF) == 0);
    _TEST_INT(_File_GetChar(stream), 'z');
    _TEST_INT(_File_GetChar(stream), _FILE_END);
}

int main(void)
{
    Test_FilePushback();
    return _Test_Status();
}
