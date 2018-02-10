#pragma once

#include <Conventions\EntityID.h>
#include <Conventions\Orientation.h>

#include <Utilities\Range.h>

#include <vector>

namespace Physics
{
    // holds both static and kinematic entities
    struct NonCollidingBodies
    {
        std::vector<EntityID> entity_ids;
        std::vector<uint32_t> entity_to_element;

        std::vector<Orientation> orientations;
        std::vector<Orientation> previous_orientations;

        uint32_t kinematic_body_start_index = 0;

        void AddStaticComponent( EntityID entity_id, Orientation orientation );
        void AddKinematicComponent( EntityID entity_id, Orientation orientation );

        void RemoveEntities( Range<EntityID const *> entity_ids_to_be_removed );
    };


    template<typename DataType>
    Range<DataType *> CreateStaticDataRange( NonCollidingBodies const & container, std::vector<DataType> & data )
    {
        return CreateRange( data, 0, container.kinematic_body_start_index );
    }

    template<typename DataType>
    Range<DataType const *> CreateStaticDataRange( NonCollidingBodies const & container, std::vector<DataType> const & data )
    {
        return CreateRange( data, 0, container.kinematic_body_start_index );
    }

    template<typename DataType>
    Range<DataType *> CreateKinematicDataRange( NonCollidingBodies const & container, std::vector<DataType> & data )
    {
        return CreateRange( data, container.kinematic_body_start_index, Size(data) );
    }

    template<typename DataType>
    Range<DataType const *> CreateKinematicDataRange( NonCollidingBodies const & container, std::vector<DataType> const & data )
    {
        return CreateRange( data, container.kinematic_body_start_index, Size(data) );
    }

}