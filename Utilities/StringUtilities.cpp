#include "StringUtilities.h"

#include <Windows\WindowsInclude.h>
#include <array>
#include <string>
#include <codecvt>
#include <locale>
#include <iomanip>
#include <sstream>

typedef std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf8utf16Converter;

// Replace any instance of old_sub in path with new_sub
std::string ReplaceSubString( std::string path, std::string const & old_sub, std::string const & new_sub )
{
    auto found = path.find( old_sub );
    while( found != std::string::npos )
    {
        path.replace( found, old_sub.length(), new_sub );
        found = path.find( old_sub );
    }

    return path;
}


std::string DiscardPath( std::string const & input )
{
    auto path_ending = input.rfind( "\\" );
    return input.substr( path_ending + 1 );
}


std::wstring DiscardPath( std::wstring const & input )
{
    auto path_ending = input.rfind( L"\\" );
    return input.substr( path_ending + 1 );
}


std::string DiscardFileName( std::string input )
{
    auto path_ending = input.rfind( "\\" );
    input.resize( path_ending + 1 );
    return input;
}


std::wstring DiscardFileName( std::wstring input )
{
    auto path_ending = input.rfind( L"\\" );
    input.resize( path_ending + 1 );
    return input;
}


std::string DiscardExtension( std::string input )
{
    auto path_ending = input.rfind( "." );
    input.resize( path_ending );
    return input;
}


std::string GetExtension( std::string input )
{
    auto path_ending = input.rfind( "." );
    input.erase(0, path_ending);
    return input;
}


std::wstring DiscardExtension( std::wstring input )
{
    auto path_ending = input.rfind( L"." );
    input.resize( path_ending );
    return input;
}


std::wstring GetExtension( std::wstring input )
{
    auto path_ending = input.rfind( L"." );
    input.erase(0, path_ending);
    return input;
}


std::string GetExecutableFilePathString()
{
    std::array< char, MAX_PATH> buffer;
    auto size = GetModuleFileNameA( NULL, buffer.data(), static_cast<DWORD>( buffer.size() ) );
    return{ buffer.data(), buffer.data() + size };
}


std::wstring GetExecutableFilePathWString()
{
    std::array< wchar_t, MAX_PATH> buffer;
    auto size = GetModuleFileNameW( NULL, buffer.data(), static_cast<DWORD>( buffer.size() ) );
    return{ buffer.data(), buffer.data() + size };
}


std::string GetCurrentWorkingDirectoryString()
{
    std::array< char, MAX_PATH> buffer;
    auto size = GetCurrentDirectoryA(MAX_PATH, buffer.data());
    return{ buffer.data(), buffer.data() + size };
}


std::wstring GetCurrentWorkingDirectoryWString()
{
    std::array< wchar_t, MAX_PATH> buffer;
    auto size = GetCurrentDirectoryW(MAX_PATH, buffer.data());
    return{ buffer.data(), buffer.data() + size };
}


void SplitString(std::wstring const & string, std::vector<std::wstring> & split_strings)
{
    SplitString(string, ' ', split_strings);
}


void SplitString(std::wstring const & string, wchar_t split_character, std::vector<std::wstring> & split_strings)
{
    auto data = begin(string);
    auto start = data;
    while(data != end(string))
    {
        while(data != end(string) && *data != split_character)
        {
            ++data;
        }
        split_strings.emplace_back(start, data);
        data += data != end(string);
        start = data;
    }
}