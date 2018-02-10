#include "TimeFunctions.h"

Time TimeFromWindowsFileTime( FILETIME const & file_time )
{
    ULARGE_INTEGER large_integer;
    large_integer.HighPart = file_time.dwHighDateTime;
    large_integer.LowPart = file_time.dwLowDateTime;
    Time time;
    time.ticks = large_integer.QuadPart;
    return time;
}