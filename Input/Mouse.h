#pragma once
#include <Windows\WindowsInclude.h>
#include "MouseState.h"

struct Mouse
{
    MouseState                    m_state;
private:
    bool                          m_visible = false;
    bool                          m_active = true;

public:
    void UpdateRaw( LPARAM lParam );
    void UpdateScreen(LPARAM lParam, Math::Unsigned2 screen_area_size);

    void BeginTick();

    bool IsVisible() const;
    void SetVisible(bool visible);

    void Activate();
    void Deactivate();
    bool IsActive() const;
};
