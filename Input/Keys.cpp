#include "Keys.h"

#include <Windows\WindowsInclude.h>


Keys::Keys()
{
    state.fill(0);
    // count.fill(0);
}


bool Keys::Up( KeyCode code ) const
{
    return (State::Position & state[static_cast<uint8_t>(code)]) == State::Up;
}


bool Keys::Down( KeyCode code ) const
{
    return (State::Position & state[static_cast<uint8_t>(code)]) == State::Down;
}


bool Keys::Clicked( KeyCode code ) const
{
    return state[static_cast<uint8_t>(code)] == State::Clicked;
}


bool Keys::Released( KeyCode code ) const
{
    return state[static_cast<uint8_t>(code)] == State::Released;
}


// uint16_t const & Keys::Count( KeyCode code ) const
// {
//     return count[static_cast<uint8_t>(code)];
// }


void Keys::Update()
{
    BYTE key_states[256];
    if(GetKeyboardState(key_states) != 0)
    {
        for (auto i = 0u; i < 256; ++i)
        {
            uint8_t key_state = key_states[i];
            key_state  = (key_state & 0x80) ? Keys::State::Down : Keys::State::Up;
            key_state |= ((this->state[i] & Keys::State::Position) == key_state) ? 0 : Keys::State::Changed;
            this->state[i] = key_state;
        }
    }
}
