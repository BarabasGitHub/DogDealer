#include "EntityAbilities.h"



void Logic::EntityThrowingAbilities::Add( EntityID entity, ThrowProperties input_properties, EntityTemplateID ammunition )
{
    auto index = uint32_t( entities.size() );
    entities.emplace_back( entity );
    this->properties.emplace_back( input_properties );
    ammunition_entities.emplace_back( ammunition );

    assert( entities.size() == ammunition_entities.size() );

    AddIndexToIndices( entity_to_data, entity.index, index );
}


void Logic::EntityWieldingAbilities::Add( EntityID const wielder_entity, EntityID const wielded_entity, unsigned const bone_index, Orientation const & weapon_offset )
{
    auto index = uint32_t( entities.size() );
    entities.emplace_back( wielder_entity );
    wielded_entities.emplace_back( wielded_entity );

    bone_indices.emplace_back( bone_index );
    weapon_offsets.emplace_back( weapon_offset );
    wielder_direction.emplace_back( WielderDirectionType::Down );
    wielder_states.emplace_back( WielderStateType::Idle );
    wielder_state_times.emplace_back( -std::numeric_limits<float>::max() );

    assert( entities.size() == entities.size() );

    AddIndexToIndices( entity_to_data, wielder_entity.index, index );
    AddIndexToIndices(wielded_entity_to_data, wielded_entity.index, index );
}


void Logic::EntityThrowingAbilities::RemoveEntity( EntityID const & entity_id )
{
    if( entity_id.index < entity_to_data.size() )
    {
        auto index = entity_to_data[entity_id.index];
        if( index < ammunition_entities.size() )
        {
            entity_to_data[entity_id.index] = c_invalid_index;

            SwapAndPrune( index, ammunition_entities );
            SwapAndPrune( index, entities );
            SwapAndPrune( index, properties );

            ReplaceIndex( entity_to_data, uint32_t( entities.size() ), index );
        }
    }
}


namespace
{
    using namespace Logic;
    void RemoveData( uint32_t index, EntityWieldingAbilities & self )
    {
        SwapAndPrune(
            index,
            self.entities,
            self.wielded_entities,
            self.bone_indices,
            self.weapon_offsets,
            self.wielder_direction,
            self.wielder_states,
            self.wielder_state_times );
    }
}


void Logic::EntityWieldingAbilities::RemoveEntity( EntityID const entity_id )
{
    RemoveEntities( CreateRange( &entity_id, 1 ) );
}


void Logic::EntityWieldingAbilities::RemoveEntities( Range<EntityID const *> entities_to_be_removed )
{
    if( !IsEmpty( entities_to_be_removed ) )
    {
        for( auto entity_id : entities_to_be_removed )
        {
            auto data_index = GetOptional( entity_to_data, entity_id.index );
            // If entity is a wielder
            if( data_index < entities.size() )
            {
                RemoveData( data_index, *this );
            }
            else
            {
                data_index = GetOptional( wielded_entity_to_data, entity_id.index );
                if( data_index < entities.size() )
                {
                    RemoveData( data_index, *this );
                }
            }
        }

        CalculateIndices( CreateRange(entities), c_invalid_index, entity_to_data );
        CalculateIndices( CreateRange(wielded_entities), c_invalid_index, wielded_entity_to_data );
    }
}