#pragma once

#include "Keys.h"
#include "Mouse.h"
#include "Configuration.h"

#include <Windows\WindowsInclude.h>

#include <Math\FloatTypes.h>
#include <Math\IntegerTypes.h>

#include <cstdint>

class InputManager
{
public:
    InputManager( void );
	static void RegisterInputDevices(HWND window);

    LRESULT GraphicsMessages( HWND window, UINT message, WPARAM wParam, LPARAM lParam );

    LRESULT UserInputMessages( HWND window, UINT message, WPARAM wParam, LPARAM lParam );


    Keys const & GetKeys() const
    {
        return m_keys;
    }
    MouseState const & GetMouseState() const
    {
        return m_mouse.m_state;
    };

    void ProcessInput();

    LRESULT WindowProcess( HWND window, UINT message, WPARAM wParam, LPARAM lParam );

    void SetMouseVisible( bool visible );
    bool IsMouseVisible() const;

    bool ShouldQuit() const;

    Math::Unsigned2 GetWindowSize() const { return m_client_area_size; }

    bool WindowIsHidden() const { return m_window_is_hidden; }

    Input::WorldConfiguration m_configuration;

private:
    Keys m_keys;
    Mouse m_mouse;

    Math::Int2                      m_client_area_position = 0;
    Math::Unsigned2                 m_client_area_size = 1;

    bool m_want_to_quit = false;
    bool m_window_is_active = true;
    bool m_window_is_hidden = false;

    void ResetMouse();
    void CenterMouse();

    void RecalculateWindowSize( HWND window );
    void RestrictCursorToRenderArea();
    void FreeCursor();
    void Reset();

    void Activate();
    void Deactivate();
    bool WindowIsActive() const;

};
