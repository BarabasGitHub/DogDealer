#include "Mouse.h"
#include <Math\MathFunctions.h>
#include <Math\Conversions.h>
#include <Windowsx.h>

void Mouse::UpdateRaw( LPARAM lParam )
{
    if(!this->IsActive()) return;

    RAWINPUT raw_input;
    unsigned dwSize = sizeof( raw_input );
    GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ), RID_INPUT,
                     &raw_input, &dwSize, sizeof( RAWINPUTHEADER ) );

    if( raw_input.header.dwType == RIM_TYPEMOUSE )
    {
        Math::Float2 position;
        //mouse_position.x = 0.007f * m_sensivity * raw_input.data.mouse.lLastX;
        //mouse_position.y = 0.007f * m_sensivity * raw_input.data.mouse.lLastY;
        position.x = static_cast<float>(raw_input.data.mouse.lLastX);
        position.y = static_cast<float>(raw_input.data.mouse.lLastY);
        if(raw_input.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) // absolute
        {
            this->m_state.raw_movement += position - this->m_state.raw_position;
            this->m_state.raw_position = position;
        }
        else // relative
        {
            this->m_state.raw_movement += position;
            this->m_state.raw_position += position;
        }

        if(raw_input.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
        {
            this->m_state.wheel_rotation += static_cast<short>(raw_input.data.mouse.usButtonData);
        }
    }
}


void Mouse::UpdateScreen(LPARAM lParam, Math::Unsigned2 screen_area_size)
{
    if(!this->IsActive()) return;
    auto size = Float2FromUnsigned2(screen_area_size);
    auto pos = Float2FromInt2(Math::Int2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
    auto scale = Math::Float2(2, -2) / size;
    auto offset = Math::Float2(-1, 1);
    auto screen_position = pos * scale + offset;
    this->m_state.screen_movement += screen_position - this->m_state.screen_position;
    this->m_state.screen_position = screen_position;
}

void Mouse::BeginTick()
{
    this->m_state.raw_movement = this->m_state.screen_movement = 0;
    this->m_state.wheel_rotation = 0;
}


bool Mouse::IsVisible() const
{
    return m_visible;
}


void Mouse::SetVisible(bool visible)
{
    if(m_active)
    {
        if(visible)
        {
            // this uses a counter, so increase untill it's positive and shows the cursor
            while( ShowCursor( TRUE ) <= 0 );
        }
        else
        {
            // this uses a counter, so decrease untill it's negative and hides the cursor
            while( ShowCursor( FALSE ) >= 0 );
        }
    }
    // {
    //     ShowCursor(visible);
    // }
    m_visible = visible;
}


void Mouse::Activate()
{
    if(this->IsVisible())
    {
        // this uses a counter, so increase untill it's positive and shows the cursor
        while( ShowCursor( TRUE ) <= 0 );
    }
    else
    {
        // this uses a counter, so decrease untill it's negative and hides the cursor
        while( ShowCursor( FALSE ) >= 0 );
    }
    // ShowCursor(m_visible);
    m_active = true;
}


void Mouse::Deactivate()
{
    // this uses a counter, so increase untill it's positive and shows the cursor
    while( ShowCursor( TRUE ) <= 0 );
    // ShowCursor(!m_visible);
    m_active = false;
}


bool Mouse::IsActive() const
{
    return m_active;
}
