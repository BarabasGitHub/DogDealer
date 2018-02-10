#include "Window.h"

#include <cassert>
#include <cstdint>
#include <algorithm>

namespace {
    LRESULT CALLBACK WindowProcess( HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam )
    {
        auto window = reinterpret_cast<Window*>( GetWindowLongPtrW( window_handle, 0 ) );
        if( message == WM_NCCREATE || message == WM_CREATE )
        {
            window = *reinterpret_cast<Window**>( lParam );
            SetWindowLongPtrW( window_handle, 0, reinterpret_cast<LONG_PTR>( window ) );
        }

        if(window) return window->ProcessMessage( window_handle, message, wParam, lParam );
        else return DefWindowProcW(window_handle, message, wParam, lParam);
    }
}

std::wstring const Window::c_class_name = L"DogWorld";

Window::Window(void):
    m_window(0)
{

}


Window::~Window(void)
{
}




void Window::Register(HINSTANCE instance)
{
    // Register window class and create the window


    WNDCLASSEXW wc;
    wc.cbSize = sizeof( WNDCLASSEXW );
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProcess;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof( void* ); //size of a pointer
    wc.hInstance = instance;
    wc.hIcon = LoadIconW( nullptr, IDI_APPLICATION );
    wc.hCursor = LoadCursorW( nullptr, IDC_ARROW );
    wc.hbrBackground = reinterpret_cast<HBRUSH>( COLOR_BTNSHADOW );
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = c_class_name.c_str();
    wc.hIconSm = LoadIconW( nullptr, IDI_APPLICATION );
    RegisterClassExW( &wc );

    m_window = CreateWindowExW( 0L, c_class_name.c_str(), m_window_title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, instance, this );
    if( !m_window )
    {
        throw WindowsException( HRESULT_FROM_WIN32(GetLastError()) );
    }
}


LRESULT Window::ProcessMessage( HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam )
{
    LRESULT result = 0;
    for( auto& item : m_callbacks )
    {
        // return first nonzero result if any
        auto function_result = item.first( window_handle, message, wParam, lParam );
        result = result != 0 ? result : function_result;
    }
    return result;
}


unsigned Window::RegisterForProcessingMessages( ProcessMessageCallBack function)
{
    auto id = m_callbacks.size() > 0 ? m_callbacks.back().second + 1 : 0;
    m_callbacks.emplace_back(function, id);
    return id;
}


void Window::UnRegisterForProcessingMessages( unsigned id )
{
    int64_t reverse_index = static_cast<int64_t>(m_callbacks.size()) - id - 1;
    reverse_index = reverse_index > 0 ?  reverse_index : 0;
    auto item = std::find_if(m_callbacks.rbegin() + reverse_index, m_callbacks.rend(), [&](decltype(m_callbacks[0]) in )
    {
        return in.second == id;
    }).base();

    m_callbacks.erase( item );
}