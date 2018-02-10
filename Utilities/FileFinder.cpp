#include "FileFinder.h"

#include <Windows\WindowsInclude.h>

#include "StringUtilities.h"
#include "TimeFunctions.h"

namespace
{
    // to automagically close the handle in case we exit unexpectedly
    struct SearchHandle
    {
        HANDLE handle = INVALID_HANDLE_VALUE;
        SearchHandle() = default;
        SearchHandle( HANDLE handle ) : handle( handle )
        {
        }
        ~SearchHandle()
        {
            if( handle != INVALID_HANDLE_VALUE )
            {
                FindClose( handle );
            }
        }
        operator HANDLE ()
        {
            return handle;
        }
    };


    bool NextDirectory( HANDLE search_handle, WIN32_FIND_DATAW & find_data )
    {
        auto result = FindNextFileW( search_handle, &find_data ) != 0;
        if( result )
        {
            if( find_data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
            {
                return NextDirectory( search_handle, find_data );
            }
        }
        return result;
    }


    bool NextFile( HANDLE search_handle, WIN32_FIND_DATAW & find_data )
    {
        auto result = FindNextFileW( search_handle, &find_data ) != 0;
        if( result )
        {
            if( find_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
            {
                return NextFile( search_handle, find_data );
            }
        }
        return result;
    }

}

std::vector<std::wstring> FindDirectories( std::wstring directory_name )
{
    WIN32_FIND_DATAW find_data;
    SearchHandle search_handle = FindFirstFileExW( directory_name.c_str(), FindExInfoBasic, &find_data, FindExSearchLimitToDirectories, nullptr, FIND_FIRST_EX_LARGE_FETCH );
    std::vector<std::wstring> directories;
    if( search_handle == INVALID_HANDLE_VALUE )
    {
        // error!
        //auto error_code = GetLastError();
        return directories;
    }

    auto const path = DiscardFileName( std::move( directory_name ) );

    do
    {
        if( find_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
        {
            directories.emplace_back( path + find_data.cFileName );
        }

    } while( FindNextFileW( search_handle, &find_data ) );

    return directories;
}


std::vector<FileDescription> FindFiles( std::wstring filename )
{
    std::vector<FileDescription> files;
    FindFiles( move( filename ), files );
    return files;
}


FileDescription GetFileDescription(std::wstring filename)
{
    FileDescription file_description;
    WIN32_FIND_DATAW find_data;
    SearchHandle search_handle = FindFirstFileExW(filename.c_str(), FindExInfoBasic, &find_data, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
    if(search_handle != INVALID_HANDLE_VALUE)
    {
        if(find_data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
        {
            auto last_write_time = TimeFromWindowsFileTime(find_data.ftLastWriteTime);
            file_description.full_name = DiscardFileName(move(filename));
            file_description.full_name += find_data.cFileName;
            file_description.last_write_time = last_write_time;
        }
    }
    return file_description;
}


void FindFiles( std::wstring filename, std::vector<FileDescription> & files )
{
    WIN32_FIND_DATAW find_data;
    SearchHandle search_handle = FindFirstFileExW( filename.c_str(), FindExInfoBasic, &find_data, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH );
    if( search_handle == INVALID_HANDLE_VALUE )
    {
        // error!
        //auto error_code = GetLastError();
        return;
    }

    auto const path = DiscardFileName( std::move( filename ) );

    do
    {
        if( find_data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
        {
            auto last_write_time = TimeFromWindowsFileTime( find_data.ftLastWriteTime );
            files.emplace_back( last_write_time, path + find_data.cFileName);
        }

    } while( FindNextFileW( search_handle, &find_data ) );

    //auto FileSize = (m_find_data.nFileSizeHigh * (MAXDWORD+1)) + m_find_data.nFileSizeLow;
}

