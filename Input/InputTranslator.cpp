#include "InputTranslator.h"

#include "InputManager.h"
#include "GameInput.h"
#include "InterfaceInput.h"
#include "Configuration.h"

#include <Math\FloatTypes.h>
#include <Math\MathFunctions.h>
#include <Utilities\StdVectorFunctions.h>

using namespace Input;

namespace
{
    Math::Float3 GetAnglesFromMousePosition( Math::Float2 const & mouse_position )
    {
        return Math::Float3( -mouse_position.y, 0.f, -mouse_position.x );
    }

    Math::Float3 CameraMovementFromKeyOffset( Math::Float2 const & offset )
    {
        return{ offset.x, 0.f, -offset.y };
    }

    Math::Float2 OffsetFromDirectionalKeys( DirectionalKeys const & config, Keys const & keys )
    {
        auto offset = Math::Float2( 0, 0 );

        if( keys.Down(config.up) ) offset.y += 1;
        if( keys.Down(config.down) ) offset.y -= 1;
        if( keys.Down(config.left) ) offset.x -= 1;
        if( keys.Down(config.right) ) offset.x += 1;

        return offset;
    }
    
    void ZeroMouseKeys(Keys & keys)
    {
        for(auto i = 0u; i < Size(keys.state); ++i)
        {
            if(IsMouseKey(KeyCode(i)))
            {
                keys.state[i] = Keys::State::Up;
            }
        }
    }
}


void TranslateInputToGameInput(const InputManager& input_manager, const KeyConfiguration & key_configuration, GameInput & output)
{
    TranslateInputToGameInput(input_manager.GetKeys(), key_configuration, output);

    auto const & mouse_state = input_manager.GetMouseState();

    output.attack_direction = ConditionalNormalize(mouse_state.raw_movement);
}


void TranslateInputToGameInputIgnoreMouse(const InputManager& input_manager, const KeyConfiguration & key_configuration, GameInput & output)
{
    auto keys = input_manager.GetKeys();
    ZeroMouseKeys(keys);
    TranslateInputToGameInput(keys, key_configuration, output);
    output.attack_direction = 0;
}


void TranslateInputToGameInput(const Keys& keys, const KeyConfiguration & key_configuration, GameInput & output)
{
    // Player Movement
    output.movement = OffsetFromDirectionalKeys(key_configuration.player_move, keys);

    // jump jump jump!
    if( keys.Clicked( key_configuration.jump ) )
    {
        Append(output.events, GameInputEvent::Jump);
    }

    // looking around
    if( keys.Down( key_configuration.lookaround ) )
    {
        Append(output.events, GameInputEvent::LookAround);
    }

    // Drop weapons upon pressing the dropping key
    if (keys.Down(key_configuration.drop_weapon))
    {
        Append(output.events, GameInputEvent::DropItem);
    }

    // PICKING UP WEAPONS
    if (keys.Down(key_configuration.pick_up_weapon))
    {
        Append(output.events, GameInputEvent::GrabItem);
    }

    // STRIKING:
    if( keys.Down(key_configuration.attack_action) )
    {
        Append(output.events, GameInputEvent::Strike);
    }

    // Unless holding, release strikes for all mouse controlled entities
    if ( keys.Up(key_configuration.attack_action) )
    {
        Append(output.events, GameInputEvent::ReleaseStrike);
    }

    // Right mouse button cancels all attacks
    if( keys.Down(key_configuration.block_action) )
    {
        Append(output.events, GameInputEvent::Block);
    }

    // Releasing the right mouse button cancels any blocking actions
    if ( keys.Released(key_configuration.block_action) )
    {
        Append(output.events, GameInputEvent::ReleaseBlock);
    }

    // THROWING:
    if (keys.Clicked(key_configuration.throw_action))
    {
        Append(output.events, GameInputEvent::Throw);
    }

}


void TranslateInputToInterfaceInput(const InputManager& input_manager, const KeyConfiguration & key_configuration, InterfaceInput & output)
{
    return TranslateInputToInterfaceInput(input_manager.GetKeys(), input_manager.GetMouseState(), key_configuration, output);
}


void TranslateInputToInterfaceInputIgnoreMouseKeys(const InputManager& input_manager, const KeyConfiguration & key_configuration, InterfaceInput & output)
{
    auto keys = input_manager.GetKeys();
    ZeroMouseKeys(keys);
    TranslateInputToInterfaceInput(keys, input_manager.GetMouseState(), key_configuration, output);
}


void TranslateInputToInterfaceInput(Keys const & keys, MouseState const & mouse_state, const KeyConfiguration & key_configuration, InterfaceInput & output)
{
    output.pointer_position = mouse_state.screen_position;

    // Camera zoom by mousewheel
    auto zoom_factor = 0.001f;
    output.camera.zoom = mouse_state.wheel_rotation * zoom_factor;

    auto camera_angle_factor = 0.005f;
    output.camera.angles = GetAnglesFromMousePosition( mouse_state.raw_movement * camera_angle_factor );

    auto const offset = OffsetFromDirectionalKeys( key_configuration.camera_move, keys );
    output.camera.movement = CameraMovementFromKeyOffset( offset );

    if(keys.Clicked(key_configuration.switch_control_mode))
    {
        Append(output.events, InterfaceInputEvent::SwitchControlMode);
    }


    if(keys.Clicked(KeyCode::LeftMouseButton))
    {
        Append(output.events, InterfaceInputEvent::PointerInteract);
    }

    if(keys.Clicked(key_configuration.pause_simulation))
    {
        if(keys.Down(KeyCode::Shift))
        {
            Append(output.events, InterfaceInputEvent::SingleSimulationStep);
        }
        else
        {
            Append(output.events, InterfaceInputEvent::SwitchPauseSimulation);
        }
    }
}
