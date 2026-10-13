// (Test) Status: 0
// A test file for <_file.h> orientation of a stream.

// Include helpers.
#include "_util09_00_file_helpers.inc"

// A stream should take its first orientation and keep it until it is reset.
static void Test_FileOrientation(void)
{
    _File_Stream *stream = _File_MakeStream();

    _TEST_TRUE(stream != _FILE_NULL);
    _TEST_INT(_File_OpenFile(stream, "file", "w+"), 0);
    _TEST_INT(_File_OrientStream(stream, 0), _FILE_ORIENT_NONE);
    _TEST_INT(_File_OrientStream(stream, -5), _FILE_ORIENT_BYTE);
    _TEST_INT(_File_OrientStream(stream, 5), _FILE_ORIENT_BYTE);
    _TEST_INT(_File_OrientStream(stream, 0), _FILE_ORIENT_BYTE);
    _TEST_INT(_File_ResetStream(stream), 0);
    _TEST_INT(_File_OrientStream(stream, 0), _FILE_ORIENT_NONE);
    _TEST_INT(_File_OrientStream(stream, 7), _FILE_ORIENT_WIDE);
    _TEST_INT(_File_OrientStream(stream, -1), _FILE_ORIENT_WIDE);
    stream = opened("a", 1);
    _TEST_INT(_File_OrientStream(stream, 0), _FILE_ORIENT_BYTE);
    _TEST_INT(_File_OrientStream(stream, 1), _FILE_ORIENT_BYTE);
}

int main(void)
{
    Test_FileOrientation();
    return _Test_Status();
}
