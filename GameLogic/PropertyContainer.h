#pragma once

#include <Conventions\EntityID.h>
#include <Utilities\Range.h>
#include <Math\FloatTypes.h>
#include <vector>

namespace Logic
{
    // For things like hitpoints etc
    struct NumericPropertyContainer
    {
        std::vector<EntityID> entity_ids;
        std::vector<unsigned> entity_to_property;

        std::vector<float> current_hitpoints;
        std::vector<float> maximum_hitpoints;

        void AddComponent(EntityID const entity_id, float const in_maximum_hitpoints);
        void RemoveEntity(EntityID const entity_id);
        void RemoveEntities(Range<EntityID const*> entities_to_be_removed);
    };

    struct PropertyContainer
    {
        std::vector<EntityID> entity_ids;
        std::vector<unsigned> entity_to_property;

        std::vector<float> motion_powers;
        std::vector<float> motion_target_speeds;

        std::vector<float> torques;

		std::vector<Math::Float2> motion_biases_positive;
		std::vector<Math::Float2> motion_biases_negative;

		void AddComponent(EntityID const id, float const motion_power, float const torque, Math::Float2 const motion_bias_positive, Math::Float2 const motion_bias_negative, float const target_speed);
		void RemoveEntity(EntityID const entity_id);
        void RemoveEntities(Range<EntityID const*> entities_to_be_removed);
        bool Contains(EntityID const id) const;
	};
}