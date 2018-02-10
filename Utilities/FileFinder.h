#pragma once

#include "FileDescription.h"

#include <string>
#include <vector>

std::vector<FileDescription> FindFiles(const std::wstring /*The path and the file name, which can include wildcard characters, for example, an asterisk (*) or a question mark (?).*/ );
// append the found files to 'files'
void FindFiles( std::wstring filename /*The path and the file name, which can include wildcard characters, for example, an asterisk (*) or a question mark (?).*/,
                std::vector<FileDescription> & files );
// just finds the first file, if the file is not found the name will be empty 
FileDescription GetFileDescription(std::wstring filename);
std::vector<std::wstring> FindDirectories(const std::wstring /*The directory, which can include wildcard characters, for example, an asterisk (*) or a question mark (?).*/ );
