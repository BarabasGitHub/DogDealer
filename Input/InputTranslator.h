#pragma once

class InputManager;
struct GameInput;
struct InterfaceInput;
struct Keys;
struct MouseState;
namespace Input
{
    struct KeyConfiguration;
}

void TranslateInputToGameInput(const InputManager& input_manager, const Input::KeyConfiguration & key_configuration, GameInput & output);
void TranslateInputToGameInputIgnoreMouse(const InputManager& input_manager, const Input::KeyConfiguration & key_configuration, GameInput & output);
void TranslateInputToGameInput(const Keys& keys, const Input::KeyConfiguration & key_configuration, GameInput & output);
void TranslateInputToInterfaceInput(const InputManager& input_manager, const Input::KeyConfiguration & key_configuration, InterfaceInput & output);
void TranslateInputToInterfaceInputIgnoreMouseKeys(const InputManager& input_manager, const Input::KeyConfiguration & key_configuration, InterfaceInput & output);
void TranslateInputToInterfaceInput(Keys const & keys, MouseState const & mouse_state, const Input::KeyConfiguration & key_configuration, InterfaceInput & output);