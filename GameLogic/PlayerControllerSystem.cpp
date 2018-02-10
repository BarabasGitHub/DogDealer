#include "PlayerControllerSystem.h"

#include <Input\MouseState.h>

#include <Math\FloatTypes.h>
#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>

#include <Utilities\StdVectorFunctions.h>

void Logic::GeneratePlayerMovement(
    std::vector<EntityID> const & entity_ids,
    Math::Float2 keys_movement,
    std::vector<EntityID> & moving_ids,
    std::vector<Math::Float2> & movements )
{
    Append( moving_ids, entity_ids );
    auto movement = ConditionalNormalize( keys_movement );
    Append(movements, movement, Size( entity_ids ));
}

void Logic::GenerateCameraTargetAngles(
    std::vector<EntityID> const & controllers,
    float const camera_z_angle,
    // output
    std::vector<EntityID> & output_entity_ids,
    std::vector<float> & target_z_angles )
{
    Append( output_entity_ids, controllers );
    target_z_angles.resize( Size( controllers ) + Size( target_z_angles ), camera_z_angle );
}


void Logic::GenerateMeleeStrikes(
    std::vector<EntityID> const & controllers,
    Math::Float2 mouse_position,
    // output
    std::vector<EntityID> & striking_entities,
    std::vector<Math::Float2> & directions
    )
{
    Append( striking_entities, controllers );
    directions.resize( Size( striking_entities ), mouse_position );
}