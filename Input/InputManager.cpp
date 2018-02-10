#include "InputManager.h"
#include "Windows\WindowsInclude.h"


// Raw input definitions for mouse
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#include <Math\IntegerOperators.h>
#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\Conversions.h>

LRESULT InputManager::WindowProcess( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{


    switch( message )
    {
    case WM_CREATE:
        {
			RecalculateWindowSize(window);
            RegisterInputDevices(window);
            return 0;
       }
    case WM_DESTROY:
		{
            PostQuitMessage( 0 );
            return 0;
		}
        // this one is never received, because the windows message thread quits when it gets the WM_QUIT message
        //case WM_QUIT:
        // User input messages
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_MOUSEMOVE:
    case WM_MOUSEWHEEL:
    case WM_INPUT:
    {
		return UserInputMessages(window, message, wParam, lParam);
    }
        // graphics_manager messages
    case WM_SIZE:
    case WM_PAINT:
    case WM_EXITSIZEMOVE:
    case WM_WINDOWPOSCHANGED:
    case WM_WINDOWPOSCHANGING:
        {
			return GraphicsMessages(window, message, wParam, lParam);
        }

        // I don't understand and/or know which one of these is the right one or if we maybe need both.
    case WM_ACTIVATE:
        {
            // high word contains minimized info
        switch( LOWORD( wParam ) )
            {
            case WA_ACTIVE:
                //{
                //    break;
                //}
            case WA_CLICKACTIVE:
                {
					Activate();
                    break;
                }
            case WA_INACTIVE:
                {
					Deactivate();
                    break;
                }
            }
            return 0;
        }
    }
    return DefWindowProcW( window, message, wParam, lParam );
}



InputManager::InputManager()
{
    m_configuration.keys.camera_move = { KeyCode::Up, KeyCode::Down, KeyCode::Left, KeyCode::Right };
    m_configuration.keys.player_move = { KeyCode::W, KeyCode::S, KeyCode::A, KeyCode::D };
    m_configuration.keys.lookaround = KeyCode::V;

    m_configuration.keys.drop_weapon = KeyCode::G;
    m_configuration.keys.pick_up_weapon = KeyCode::E;
    m_configuration.keys.jump = KeyCode::Space;

    m_configuration.keys.throw_action = KeyCode::LeftMouseButton;
    m_configuration.keys.attack_action = KeyCode::LeftMouseButton;
    m_configuration.keys.block_action = KeyCode::RightMouseButton;

    m_configuration.keys.switch_control_mode = KeyCode::Escape;
    m_configuration.keys.pause_simulation = KeyCode::P;

    m_mouse.SetVisible(false);
}

void InputManager::RegisterInputDevices(HWND window)
{
	// Initialize raw input for mouse
	RAWINPUTDEVICE Rid[1] = {};

	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = window;
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
}

LRESULT InputManager::GraphicsMessages( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch (message)
    {
    case WM_SIZE:
        {
            switch(wParam)
            {
                case SIZE_MAXIMIZED: // The window has been maximized.
                case SIZE_MAXSHOW: // Message is sent to all pop-up windows when some other window has been restored to its former size.
                case SIZE_RESTORED: // The window has been resized, but neither the SIZE_MINIMIZED nor SIZE_MAXIMIZED value applies.
                {
                    m_window_is_hidden = false;
                    break;
                }
                case SIZE_MINIMIZED: // The window has been minimized.
                case SIZE_MAXHIDE: // Message is sent to all pop-up windows when some other window is maximized.
                {
                    m_window_is_hidden = true;
                    break;
                }
            }
            RecalculateWindowSize( window );
            return 0;
        }
    case WM_EXITSIZEMOVE:
        {
            RecalculateWindowSize( window );
            break;
        }
    case WM_WINDOWPOSCHANGED:
        {
            RecalculateWindowSize( window );
            break;
        }
//    case WM_WINDOWPOSCHANGING:
//        {
//            //auto position_structure = reinterpret_cast<WINDOWPOS*>( lParam );
//            //auto aspectratio = 800.0f/600.0f;
//            //position_structure->cx = position_structure->cy*aspectratio;
//            break;
//        }

// Let DefWindowProc handle the WM_PAINT message, we will update the window again when we come to the render call in our game loop
//    case WM_PAINT:
    }
    return DefWindowProcW( window, message, wParam, lParam );
}


LRESULT InputManager::UserInputMessages( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
	if( WindowIsActive() )
    {
        switch( message )
        {
        case WM_KEYDOWN:
            {
                //unsigned short repeat = lParam & 0xFFFF; // The repeat count for the current message. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key. If the keystroke is held long enough, multiple messages are sent. However, the repeat count is not cumulative.
                // bool  was_up = ( lParam & (1u << 30) ) == 0; //  The previous key state. The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
                // if( was_up )
                // {
                //     m_keys[wParam] = 0;
                // }
                // m_keys_down[wParam] = true;

                return 0;
            }
        case WM_KEYUP:
            {
                // m_keys_down[wParam] = false;
                // m_keys[wParam] = 0;
                return 0;
            }
        case WM_MOUSEMOVE:
            {
                m_mouse.UpdateScreen(lParam, m_client_area_size);
                return 0;
            }

        case WM_MOUSEWHEEL:
            {
                //m_mouse.m_state.wheel_rotation = GET_WHEEL_DELTA_WPARAM( wParam );

                return 0;
            }
        case WM_INPUT:
            {
                m_mouse.UpdateRaw( lParam );
                return 0;
            }
        }
    }
    return DefWindowProcW( window, message, wParam, lParam );
}


void InputManager::ProcessInput()
{
    Reset();
    MSG message;
    while( PeekMessageW( &message, nullptr, 0, 0, PM_REMOVE ) )
    {
        TranslateMessage( &message );
        DispatchMessageW( &message );

        if( message.message == WM_QUIT )
        {
            m_want_to_quit = true;
            break;
        }
    }
    if(WindowIsActive())
    {
        m_keys.Update();
    }
}


void InputManager::ResetMouse()
{
    m_mouse.BeginTick();
}


void InputManager::CenterMouse()
{
    auto window_center = m_client_area_position + Math::Int2FromUnsigned2(m_client_area_size / 2);
    SetCursorPos( window_center.x, window_center.y );
}


void InputManager::SetMouseVisible( bool visible )
{
    m_mouse.SetVisible(visible);
}


bool InputManager::IsMouseVisible() const
{
    return m_mouse.IsVisible();
}

void InputManager::Reset()
{
    ResetMouse();
}


void InputManager::RecalculateWindowSize( HWND window )
{
    RECT client_rectangle;
    GetClientRect( window, &client_rectangle );

	RECT window_rectangle;
    GetWindowRect( window, &window_rectangle );

	m_client_area_size.x = uint32_t(client_rectangle.right);
    m_client_area_size.y = uint32_t(client_rectangle.bottom);

	Math::Unsigned2 window_size;
    window_size.x = uint32_t(window_rectangle.right - window_rectangle.left);
    window_size.y = uint32_t(window_rectangle.bottom - window_rectangle.top);

	auto border_thickness = window_size.x - m_client_area_size.x;
    auto title_bar_height = window_size.y - m_client_area_size.y - border_thickness;
    border_thickness /= 2;

	m_client_area_position.x = window_rectangle.left + int32_t(border_thickness);
    m_client_area_position.y = window_rectangle.top + int32_t(border_thickness + title_bar_height);
}

void InputManager::RestrictCursorToRenderArea()
{
    RECT window_rectangle;
    window_rectangle.right = window_rectangle.left = m_client_area_position.x;
    window_rectangle.bottom = window_rectangle.top = m_client_area_position.y;
    window_rectangle.right += m_client_area_size.x;
    window_rectangle.bottom += m_client_area_size.y;
    ClipCursor( &window_rectangle );
}


void InputManager::FreeCursor()
{
    ClipCursor( nullptr );
}


bool InputManager::ShouldQuit() const
{
    return m_want_to_quit;
}

void InputManager::Activate()
{
    m_window_is_active = true;
    RestrictCursorToRenderArea();
    m_mouse.Activate();
}

void InputManager::Deactivate()
{
    m_window_is_active = false;
    m_mouse.Deactivate();
    FreeCursor();
}

bool InputManager::WindowIsActive() const
{
    return m_window_is_active;
}

