#pragma once

#include <Conventions\EntityID.h>
#include <Conventions\EntityTemplateID.h>

#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>

#include <vector>


// for the throwproperties
#include <Conventions\Orientation.h>

namespace Logic
{
    // TODO: Move someplace else
    struct ThrowProperties
    {
        // the offset on the bone
        Orientation offset;
        // from what bone it is thrown
        uint32_t bone;
        // how much the offset to throwing upward is
        float upward_launch_offset;
        // the speed at which the object is launched/thrown
        float launch_speed;
    };

    struct EntityThrowingAbilities
    {
        std::vector<EntityID> entities;

        // does entity index -> data index, for both ammunition and properties (and entities)
        std::vector<unsigned> entity_to_data;

        std::vector<EntityTemplateID> ammunition_entities;
        std::vector<ThrowProperties> properties;

        void Add(EntityID entity, ThrowProperties properties, EntityTemplateID ammunition);
        void RemoveEntity(EntityID const & entity_id);
    };

    enum struct WielderStateType{ Idle, Readying, PreHolding, Holding, Striking, Recovering, Blocking, Invalid};
    enum struct WielderDirectionType{ Left, Right, Up, Down};

    struct EntityWieldingAbilities
    {
        // Parallel vectors of wielder and weapon entities
        std::vector<EntityID> entities;
        std::vector<EntityID> wielded_entities;

        // Entity index -> data index
        std::vector<unsigned> entity_to_data;
        std::vector<unsigned> wielded_entity_to_data;

        // Indexed by entity_to_data
        std::vector<unsigned> bone_indices;
        std::vector<Orientation> weapon_offsets;
        std::vector<WielderDirectionType> wielder_direction;

        // The current state the wielder is in
        // and  the remaining time before a state change occurs
        std::vector<WielderStateType> wielder_states;
        std::vector<float> wielder_state_times;

        void Add(EntityID const wielder_entity, EntityID const wielded_entity, unsigned const bone_index, Orientation const & weapon_offset);
        void RemoveEntity(EntityID const entity_id);
        void RemoveEntities( Range<EntityID const *> entities );
    };
}


