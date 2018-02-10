#pragma once
#include "KeyCodes.h"

#include <array>
#include <cstdint>

struct Keys
{
    Keys();

    struct State
    {
        static uint8_t const Position = 0x01;
        static uint8_t const Up = 0x00;
        static uint8_t const Down = 0x01;
        static uint8_t const Changed = 0x02;
        static uint8_t const Clicked = Down | Changed;
        static uint8_t const Released = Up | Changed;
    };

    std::array<uint8_t, 256> state;
    // std::array<uint16_t, 256> count;

    bool Up( KeyCode code ) const;
    bool Down( KeyCode code ) const;
    bool Clicked( KeyCode code ) const;
    bool Released( KeyCode code ) const;
    // uint16_t const & Count( KeyCode code ) const;
    void Update();
};
