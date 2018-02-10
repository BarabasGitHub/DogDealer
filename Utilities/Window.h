#pragma once
#include <string>
#include <functional>
#include <Windows\WindowsInclude.h>
#include <vector>

class Window
{
public:
    typedef  std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> ProcessMessageCallBack;

private:
    static std::wstring const c_class_name;
    std::wstring m_window_title;
    HWND m_window;

    std::vector<std::pair<ProcessMessageCallBack, unsigned>> m_callbacks;

public:
    Window(void);
    ~Window(void);

    void SetWindowTitle(std::wstring title) { m_window_title = std::move(title); }

    HWND GetWindowHandle() { return m_window; }

    void Register(HINSTANCE instance);
    bool ShowWindow( int command_show ) { return ::ShowWindow( m_window, command_show ) != 0; }
    bool ShowWindowAsync( int command_show ) { return ::ShowWindowAsync( m_window, command_show ) != 0; }
    bool UpdateWindow() { return ::UpdateWindow( m_window ) != 0; }

    unsigned RegisterForProcessingMessages( ProcessMessageCallBack function );
    void UnRegisterForProcessingMessages( unsigned id );

    LRESULT ProcessMessage( HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam );

};

