// (Test) Status: 0
// A test file for <time.h> strftime conversions.

// Include helpers.
#include "libc14_00_time_helpers.inc"

// strftime should write each conversion as the "C" locale does.
// [C99:7.23.3.5p7]: strftime's texts in the "C" locale
static void Test_TimeConversions(void)
{
    _TEST_TRUE(formats(LAST_1999, "%a|%A|%b|%B|%c|%C|%d|%D|%e|%F|%g|%G|%h|%H|%I|%j|%m", "Fri|Friday|Dec|December|Fri Dec 31 23:59:58 1999|19|31|12/31/99|31|1999-12-31|99|1999|Dec|23|11|365|12"));
    _TEST_TRUE(formats(LAST_1999, "%M|%n|%p|%r|%R|%S|%t|%T|%u|%U|%V|%w|%W|%x|%X|%y|%Y|%%", "59|\n|PM|11:59:58 PM|23:59|58|\t|23:59:58|5|52|52|5|52|12/31/99|23:59:58|99|1999|%"));
    _TEST_TRUE(formats(LAST_1999, "%Ec|%EC|%Ex|%EX|%Ey|%EY", "Fri Dec 31 23:59:58 1999|19|12/31/99|23:59:58|99|1999"));
    _TEST_TRUE(formats(LAST_1999, "%Od|%Oe|%OH|%OI|%Om|%OM|%OS|%Ou|%OU|%OV|%Ow|%OW|%Oy", "31|31|23|11|12|59|58|5|52|52|5|52|99"));
}

int main(void)
{
    Test_TimeConversions();
    return _Test_Status();
}
