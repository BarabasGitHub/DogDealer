#pragma once

#include <Conventions\EntityID.h>
#include <Math\ForwardDeclarations.h>

#include <vector>

struct MouseState;

namespace Logic
{

    void GeneratePlayerMovement(
        std::vector<EntityID> const & entity_ids,
        Math::Float2 keys_movement,
        // output
        std::vector<EntityID> & moving_ids,
        std::vector<Math::Float2> & movements );


    void GenerateCameraTargetAngles(
        std::vector<EntityID> const & controllers,
        float const camera_z_angle,
        // output
        std::vector<EntityID> & output_entity_ids,
        std::vector<float> & target_z_angles  );


    void GenerateMeleeStrikes(
        std::vector<EntityID> const & controllers,
        Math::Float2 mouse_position,
        // output
        std::vector<EntityID> & striking_entities,
        std::vector<Math::Float2> & directions
        );
}