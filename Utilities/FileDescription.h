#pragma once

#include "Time.h"
#include <string>

struct FileDescription
{
    // the number of 100-nanosecond intervals that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
    Time last_write_time = {0};
    // file name including path and extension
    std::wstring full_name;

    FileDescription() = default;
    FileDescription(Time last_write_time, std::wstring full_name) : last_write_time(last_write_time), full_name(move(full_name)) {}
};