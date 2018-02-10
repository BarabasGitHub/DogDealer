#pragma once
#include <winerror.h>
#include <exception>    

class WindowsException final : public std::exception
{
private:
    HRESULT m_error_code;
public:
    WindowsException() noexcept;
    WindowsException( HRESULT ) noexcept;
    WindowsException( const WindowsException& ) noexcept;
    //WindowsException& operator= (const exception&) noexcept;
    ~WindowsException() override;
    const char* what() const noexcept override;
    HRESULT ErrorCode() const noexcept;
};


inline void ThrowIfFailed( HRESULT result )
{
    if( FAILED( result ) ) throw( WindowsException( result ) );
}