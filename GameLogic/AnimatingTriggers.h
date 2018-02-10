#pragma once
#include <vector>

//#include <Math\FloatTypes.h>

#include <Conventions\EntityID.h>


namespace Logic{

    struct AnimatingTriggers
    {
		// An entity is killed and plays its dying animation.
		// Once the dying animation finishes, it is dead.
        std::vector<EntityID> killed_entities;
		std::vector<EntityID> died_entities;

        std::vector<EntityID> throwing_entities;

        std::vector<EntityID> now_idle_entities;
        std::vector<EntityID> now_moving_entities;

        std::vector<EntityID> strike_rebounding_entities;
        std::vector<EntityID> strike_cancelling_entities;

		std::vector<EntityID> struck_entities;

		std::vector<EntityID> now_turning_entities;
		std::vector<EntityID> no_longer_turning_entities;
        /*
        // These two are parallel
        std::vector<EntityID> strike_readying_entities;
        std::vector<Math::Float2> strike_readying_directions;

        // These two are parallel
        std::vector<EntityID> strike_holding_entities;
        std::vector<Math::Float2> strike_holding_directions;

        // These two are parallel
        std::vector<EntityID> strike_releasing_entities;
        std::vector<Math::Float2> strike_releasing_directions;

        // These two are parallel
        std::vector<EntityID> blocking_entities;
        std::vector<Math::Float2> blocking_directions;
        */
    };
}
