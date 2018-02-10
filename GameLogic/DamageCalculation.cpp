#include "DamageCalculation.h"

#include <Conventions\Velocity.h>
#include <Conventions\CollisionEvent.h>
#include <Math\FloatTypes.h>
#include <Math\MathFunctions.h>
#include <Utilities\VectorHelper.h>

using namespace Logic;


void Logic::CalculateDamage(
    Range<Math::Float3 const *> impact_velocities,
    Range<EntityID const *> damage_inflicting_entities,
    Range<uint32_t const*> entity_to_damage_parameters,
    Range<float const*> damage_parameters,
    Range<float *> damages)
{
    assert(Size(impact_velocities) == Size(damage_inflicting_entities));
    assert(Size(impact_velocities) == Size(damages));

    // first calculate the speeds and temporarily store them in the damage output
    for( auto i = 0u; i < Size(impact_velocities); ++i )
    {
        auto velocity = impact_velocities[i];
        auto speed = Norm(velocity);
        damages[i] = speed;
    }
    // then calculate the actual damages
    CalculateDamage(damages, damage_inflicting_entities, entity_to_damage_parameters, damage_parameters, damages);
}


void Logic::CalculateDamage(
    Range<float const *> impact_speeds,
    Range<EntityID const *> damage_inflicting_entities,
    Range<uint32_t const*> entity_to_damage_parameters,
    Range<float const*> damage_parameters,
    Range<float *> damages)
{
    assert(Size(impact_speeds) == Size(damage_inflicting_entities));
    assert(Size(impact_speeds) == Size(damages));

    for( auto i = 0u; i < Size(impact_speeds); ++i )
    {
        auto speed = impact_speeds[i];
        auto inflicter = damage_inflicting_entities[i];
        auto parameter_index = entity_to_damage_parameters[inflicter.index];
        auto damage_parameter = damage_parameters[parameter_index];
        auto damage = speed * damage_parameter;
        damages[i] = damage;
    }
}


void Logic::DamageTargets(
    Range<float const*> damages,
    Range<EntityID const *> target_entities,
    Range<uint32_t const *> entity_to_hitpoints,
    Range<float *> hitpoints,
    std::vector<EntityID> & killed_entities)
{
    assert(Size(damages) == Size(target_entities));

    for( auto i = 0u; i < Size(damages); ++i )
    {
        auto target = target_entities[i];
        auto hitpoint_index = GetOptional(entity_to_hitpoints, target.index);
        if( hitpoint_index != c_invalid_index )
        {
            auto damage = damages[i];
            auto target_hitpoints = hitpoints[hitpoint_index];
            target_hitpoints -= damage;
            hitpoints[hitpoint_index] = target_hitpoints;
            if(target_hitpoints <= 0)
            {
                killed_entities.push_back(target);
            }
        }
    }
}



void Logic::CalculateHitsAndDamageTargets(
    CollisionEvents const & collision_events,
    IndexedVelocities const & velocities,
    Range<uint32_t const *> entity_to_damage_parameters,
    Range<float const *> damage_parameters,
    Range<uint32_t const *> entity_to_hitpoints,
    Range<float *> hitpoints,
    std::vector<EntityID> & damage_dealing_entities,
    std::vector<EntityID> & damage_receiving_entities,
    std::vector<EntityID> & killed_entities)
{
    std::vector<float> impact_speeds;

    auto old_damage_dealing_entities_size = Size(damage_dealing_entities);
    auto old_damage_receiving_entities_size = Size(damage_receiving_entities);
    for( auto i = 0u; i < collision_events.entities.size(); ++i )
    {
        auto entity_pair = collision_events.entities[i];

        auto dealer_index1 = GetOptional(entity_to_damage_parameters, entity_pair.id1.index);
        auto dealer_index2 = GetOptional(entity_to_damage_parameters, entity_pair.id2.index);
        auto receiver_index1 = GetOptional(entity_to_hitpoints, entity_pair.id1.index);
        auto receiver_index2 = GetOptional(entity_to_hitpoints, entity_pair.id2.index);

        auto entity_2_damages_1 = (dealer_index2 != c_invalid_index) & (receiver_index1 != c_invalid_index);
        auto entity_1_damages_2 = (dealer_index1 != c_invalid_index) & (receiver_index2 != c_invalid_index);
        if( entity_1_damages_2 | entity_2_damages_1 )
        {
            auto position1 = GetAveragePosition(collision_events.manifolds[i]);
            auto position2 = position1 - collision_events.relative_positions[i];
            // calculate impact velocity
            auto velocity1 = CalculateVelocityAtPosition(velocities, entity_pair.id1, position1 );
            auto velocity2 = CalculateVelocityAtPosition(velocities, entity_pair.id2, position2 );
            auto relative_velocity = velocity2 - velocity1;
            
            auto direction = GetAverageSeparationDirection( collision_events.manifolds[i] );
            auto impact_speed = Dot( relative_velocity, direction );

            auto positive_impact_speed = impact_speed > 0;

            if( entity_1_damages_2 && positive_impact_speed )
            {
                damage_dealing_entities.push_back(entity_pair.id1);
                damage_receiving_entities.push_back(entity_pair.id2);
                impact_speeds.push_back(impact_speed);
            }
            if( entity_2_damages_1 && positive_impact_speed )
            {
                damage_dealing_entities.push_back(entity_pair.id2);
                damage_receiving_entities.push_back(entity_pair.id1);
                impact_speeds.push_back(impact_speed);
            }
        }
    }

    // calculate and apply damage
    std::vector<float> damages(Size(impact_speeds));
    CalculateDamage(
        impact_speeds,
        CreateRange(damage_dealing_entities, old_damage_dealing_entities_size, Size(damage_dealing_entities)),
        entity_to_damage_parameters,
        damage_parameters,
        damages);

    DamageTargets(
        damages,
        CreateRange(damage_receiving_entities, old_damage_receiving_entities_size, Size(damage_dealing_entities)),
        entity_to_hitpoints,
        hitpoints,
        killed_entities);
}