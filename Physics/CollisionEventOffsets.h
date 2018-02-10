#pragma once

#include <Utilities\Range.h>

#include <vector>
#include <cstdint>

namespace Physics
{
    struct CollisionEvents;

    // offsets indicating where the different kind of collisions start
    //
    struct CollisionEventOffsets
    {
        // where each category starts
        uint32_t
            kinematic_static = 0,
            kinematic_kinematic = 0,
            rigid_kinematic = 0,
            rigid_static = 0,
            rigid_rigid = 0;
    };


    template<typename DataType>
    Range<DataType *> CreateKinematicStaticRange(CollisionEventOffsets const & ceo, std::vector<DataType> & v)
    {
        return CreateRange(v, ceo.kinematic_static, ceo.kinematic_kinematic);
    }


    template<typename DataType>
    Range<DataType const *> CreateKinematicStaticRange(CollisionEventOffsets const & ceo, std::vector<DataType> const & v)
    {
        return CreateRange(v, ceo.kinematic_static, ceo.kinematic_kinematic);
    }


    template<typename DataType>
    Range<DataType *> CreateKinematicKinematicRange(CollisionEventOffsets const & ceo, std::vector<DataType> & v)
    {
        return CreateRange(v, ceo.kinematic_kinematic, ceo.rigid_kinematic);
    }


    template<typename DataType>
    Range<DataType const *> CreateKinematicKinematicRange(CollisionEventOffsets const & ceo, std::vector<DataType> const & v)
    {
        return CreateRange(v, ceo.kinematic_kinematic, ceo.rigid_kinematic);
    }


    template<typename DataType>
    Range<DataType *> CreateRigidKinematicRange(CollisionEventOffsets const & ceo, std::vector<DataType> & v)
    {
        return CreateRange(v, ceo.rigid_kinematic, ceo.rigid_static);
    }


    template<typename DataType>
    Range<DataType const *> CreateRigidKinematicRange(CollisionEventOffsets const & ceo, std::vector<DataType> const & v)
    {
        return CreateRange(v, ceo.rigid_kinematic, ceo.rigid_static);
    }


    template<typename DataType>
    Range<DataType *> CreateRigidStaticRange(CollisionEventOffsets const & ceo, std::vector<DataType> & v)
    {
        return CreateRange(v, ceo.rigid_static, ceo.rigid_rigid);
    }


    template<typename DataType>
    Range<DataType const *> CreateRigidStaticRange(CollisionEventOffsets const & ceo, std::vector<DataType> const & v)
    {
        return CreateRange(v, ceo.rigid_static, ceo.rigid_rigid);
    }


    template<typename DataType>
    Range<DataType *> CreateRigidRigidRange(CollisionEventOffsets const & ceo, std::vector<DataType> & v)
    {
        return CreateRange(v, ceo.rigid_rigid, Size(v));
    }


    template<typename DataType>
    Range<DataType const *> CreateRigidRigidRange(CollisionEventOffsets const & ceo, std::vector<DataType> const & v)
    {
        return CreateRange(v, ceo.rigid_rigid, Size(v));
    }


    void SortAndCatagorize(
        CollisionEvents & collision_events,
        CollisionEventOffsets& collision_event_offsets,
        Range<uint32_t const *> body_to_index,
        uint32_t kinematic_body_start_index,
        uint32_t rigid_body_start_index );


    void MergeEventManifolds(
        CollisionEvents & current_collision_events,
        CollisionEventOffsets const current_collision_event_offsets,
        CollisionEvents const & previous_collision_events,
        CollisionEventOffsets const previous_collision_event_offsets
        );
}
