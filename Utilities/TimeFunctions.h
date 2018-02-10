#pragma once

#include "Time.h"

#include <Windows\WindowsInclude.h>

Time TimeFromWindowsFileTime( FILETIME const & file_time );