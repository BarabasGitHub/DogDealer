#pragma once
#include <vector>

#include <Math\FloatTypes.h>

#include <Conventions\EntityID.h>
#include <Conventions\Velocity.h>

namespace Logic{

    // Set of entities and parameters triggered for a melee action
    struct MeleeActionTriggers{

        // Entities starting a new strike,
        // parallel to the striking directions
        std::vector<EntityID> strike_readying_entities;
        std::vector<Math::Float2> strike_readying_directions;

        // Entities realsing a held strike
        std::vector<EntityID> strike_releasing_entities;

        // Entities cancelling a started strike
        std::vector<EntityID> strike_cancelling_entities;

        // Entities starting a new weapon block
        std::vector<EntityID> block_starting_entities;

        // Entities cancelling a weapon block
        std::vector<EntityID> block_cancelling_entities;
    };

    // Store parallel parameters for weapon hits
    struct WeaponHitParameters
    {
        std::vector<EntityID> weapon_entities; 
        std::vector<EntityID> target_entities; 
        std::vector<EntityID> attacker_entities;

        std::vector<Math::Float3> directions;
        std::vector<Math::Float3> relative_positions;
        std::vector<Velocity> velocities;
    };

    struct NavigationMesh
    {
        // Store the z coordinate of all vertices separately
        std::vector<Math::Float2> vertices;
        std::vector<float> vertices_z;

        std::vector<unsigned> indices;
    };
    typedef Handle<NavigationMesh> NavigationMeshID;

    // List of edges shared between triangles, forming portals between them
    struct PortalList
    {
        // Each portal is formed by two vertices
        std::vector<unsigned> vertices_0;
        std::vector<unsigned> vertices_1;
    };

}