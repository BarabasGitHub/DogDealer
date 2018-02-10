#pragma once
#include <string>
#include <codecvt>
#include <locale>
#include <vector>

typedef std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf8utf16Converter;

// Replace any instance of old_sub in path with new_sub
std::string ReplaceSubString( std::string path, std::string const & old_sub, std::string const & new_sub );

std::string DiscardPath( std::string const & input );

std::wstring DiscardPath( std::wstring const & input );

std::string DiscardFileName( std::string input );

std::wstring DiscardFileName( std::wstring input );

// removes the extension
std::string DiscardExtension( std::string input );
std::wstring DiscardExtension( std::wstring input );

std::string GetExtension( std::string input );
std::wstring GetExtension( std::wstring input );

std::string GetExecutableFilePathString( );

std::wstring GetExecutableFilePathWString( );

std::string GetCurrentWorkingDirectoryString();

std::wstring GetCurrentWorkingDirectoryWString();

void SplitString(std::wstring const & string, std::vector<std::wstring> & split_strings);
void SplitString(std::wstring const & string, wchar_t split_character, std::vector<std::wstring> & split_strings);


#include <sstream>
#include <iomanip>

template< typename T >
std::string IntegerToHexadecimalString( T i )
{
    std::stringstream stream;
    stream << std::showbase
        << std::setfill( '0' ) << std::setw( sizeof( T ) * 2 )
        << std::hex << i;
    return stream.str();
}


template< typename T >
std::wstring IntegerToHexadecimalWString( T i )
{
    std::wstringstream stream;
    stream << std::showbase
        << std::setfill( '0' ) << std::setw( sizeof( T ) * 2 )
        << std::hex << i;
    return stream.str();
}