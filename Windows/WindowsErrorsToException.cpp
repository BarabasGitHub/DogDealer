#include "WindowsErrorsToException.h"
#include <Windows\WindowsInclude.h>

WindowsException::WindowsException() noexcept:
m_error_code( NO_ERROR )
{
}

WindowsException::WindowsException( HRESULT error_code ) noexcept :
m_error_code( error_code )
{
}

WindowsException::WindowsException( const WindowsException& in ) noexcept :
m_error_code( in.m_error_code )
{
}

//WindowsException& WindowsException::operator= (const exception& in) noexcept
//{
//    
//}

WindowsException::~WindowsException()
{
}

const char* WindowsException::what() const noexcept
{
    char* message = nullptr;
#pragma warning(suppress: 6387)    
    auto status = FormatMessageA( 
        (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS), nullptr,
        DWORD(m_error_code), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ) /* Default language */, reinterpret_cast<char*>( &message ), 0, nullptr );
    (void) status;
    return message;
}

HRESULT WindowsException::ErrorCode() const noexcept
{
    return m_error_code;
}