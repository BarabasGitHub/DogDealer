#pragma once

#include "FileType.h"

#include <string>

FileType DetermineFileType(std::wstring const & file_name);