#pragma once

#include "Movement.h"
#include "Inertia.h"
#include "BodyID.h"

#include <BoundingShapes\AxisAlignedBoxHierarchy.h>
#include <BoundingShapes\AxisAlignedBoxHierarchyMesh.h>

#include <Conventions\Orientation.h>
#include <Math\FloatTypes.h>
#include <Utilities\Range.h>

#include <vector>

namespace BoundingShapes
{
    struct AxisAlignedBox;
    struct OrientedBox;
}

namespace Physics
{
    struct ElementContainer
    {
        struct Storage
        {
            // mapping from body id index to element position
            std::vector<uint32_t> body_to_element;

            // storage for common elements
            struct Common
            {
                std::vector<BodyID> body_ids;
                std::vector<Orientation> orientations;
                std::vector<Orientation> previous_orientations;
                std::vector<BoundingShapes::AxisAlignedBox> broad_bounds; // local broad bounds, except for static, then they're already transformed
                std::vector<BoundingShapes::AxisAlignedBox> transformed_broad_bounds; // broad bounds transformed to the current orientation of the bodies
                BoundingShapes::AxisAlignedBoxHierarchy transformed_broad_bounds_hierarchy; // TODO: Not sure if it should be here
                std::vector<float> bounce_factors;
                std::vector<float> friction_factors;
            };

            // storage for rigid elements
            struct RigidBody
            {
                std::vector<Math::Float3> centers_of_mass;
                std::vector<Movement> movements;
                std::vector<Inertia> inverse_inertias;
                // contains the last used forces
                std::vector<Math::Float3> forces;
            };

            Common common;
            RigidBody rigid_body;
        };


        struct Pointers
        {
            // mapping from body id index to the data elements
            Range<uint32_t const *> body_to_element = {nullptr, nullptr};
            // ids of the entities
            BodyID const * body_ids = nullptr;
            // orientations of the bodies
            Orientation * orientations = nullptr;
            // previous orientations of bodies
            Orientation * previous_orientations = nullptr;
            // simple bounds that contain the entire body, but are usually not very accurate
            BoundingShapes::AxisAlignedBox * broad_bounds = nullptr;
            // simple bounds that contain the entire body, but are usually not very accurate, updated to the current orientation
            BoundingShapes::AxisAlignedBox * transformed_broad_bounds = nullptr;
            // factors that determine how much the bodies bounce when they hit something
            float * bounce_factors = nullptr;
            // factors that determine how much friction the bodies cause when they're in contact with something
            float * friction_factors = nullptr;
            // centres of mass for the rigid bodies
            Math::Float3 * centers_of_mass = nullptr;
            // movements (momentum and angular momentum) for the rigid bodies
            Movement * movements = nullptr;
            // inverse inertias for rigid bodies
            Inertia * inverse_inertias = nullptr;
            // contains the last used forces for rigid bodies
            Math::Float3 * forces = nullptr;
        };


        // offsets to the start of each component type
        struct Offsets
        {
            // start of the kinematic bodies and end of the static bodies
            uint32_t static_body_end_index = 0;
            // start of the rigid bodies and end of the kinematic bodies
            uint32_t kinematic_body_end_index = 0;
            // end of the rigid bodies
            uint32_t rigid_body_end_index = 0;
        };


        // where the actual data is stored
        Storage storage;
        // Pointers that are all at an offset such that the body mapping works equally on all of them.
        // This means that they may point to memory locations that are not where the actual stored data starts.
        Pointers pointers;
        // offsets to where the different body types start and end
        Offsets offsets;
    };


    void AddRigidBodyComponent(
        BodyID body_id,
        Orientation orientation,
        Math::Float3 center_of_mass,
        Movement movement,
        Inertia inverse_inertia,
        BoundingShapes::AxisAlignedBox aabox,
        float bounciness,
        float friction_factor,
        ElementContainer & self );


    void AddKinematicComponent(
        BodyID body_id,
        Orientation orientation,
        BoundingShapes::AxisAlignedBox aabox,
        float bounciness,
        float friction_factor,
        ElementContainer & self);


    void AddStaticComponent(
        BodyID body_id,
        Orientation orientation,
        BoundingShapes::AxisAlignedBox aabox,
        float bounciness,
        float friction_factor,
        ElementContainer & self);


    void ReplaceWithRigidBodyComponent(
        BodyID body_id,
        Math::Float3 center_of_mass,
        Inertia inverse_inertia,
        BoundingShapes::AxisAlignedBox aabox,
        float bounciness,
        float friction_factor,
        ElementContainer & self );


    void ReplaceWithKinematicComponent(
        BodyID body_id,
        BoundingShapes::AxisAlignedBox aabox,
        float bounciness,
        float friction_factor,
        ElementContainer & self);


    void ReplaceWithStaticComponent(
        BodyID body_id,
        BoundingShapes::AxisAlignedBox aabox,
        float bounciness,
        float friction_factor,
        ElementContainer & self );


    void RemoveBodies( Range<BodyID const *> body_ids_to_be_removed, ElementContainer & self );

    void UpdatePointers(ElementContainer & self);

    bool IsStaticBody(BodyID id, ElementContainer const & self);
    bool IsRigidBody( BodyID id, ElementContainer const & self );
    bool IsKinematicBody( BodyID id, ElementContainer const & self );
    bool IsDynamicBody( BodyID id, ElementContainer const & self );

    uint32_t TotalBodyCount(ElementContainer const & self);
    uint32_t TotalBodyCount(ElementContainer::Offsets const & offsets);

    uint32_t DynamicBodyStart(ElementContainer::Offsets const & offsets);
    uint32_t DynamicBodyEnd(ElementContainer::Offsets const & offsets);
    uint32_t KinematicBodyStart(ElementContainer::Offsets const & offsets);
    uint32_t KinematicBodyEnd(ElementContainer::Offsets const & offsets);
    uint32_t RigidBodyStart(ElementContainer::Offsets const & offsets);
    uint32_t RigidBodyEnd(ElementContainer::Offsets const & offsets);
    uint32_t StaticBodyStart(ElementContainer::Offsets const & offsets);
    uint32_t StaticBodyEnd(ElementContainer::Offsets const & offsets);

    // functions to update the offsets when changing the number of bodies
    void ChangedRigidBodyCount(int32_t change, ElementContainer::Offsets & offsets);
    void ChangedKinematicBodyCount(int32_t change, ElementContainer::Offsets & offsets);
    void ChangedStaticBodyCount(int32_t change, ElementContainer::Offsets & offsets);


    template<typename DataType>
    Range<DataType *> CreateAllBodyDataRange( ElementContainer::Offsets const & offsets, DataType * data )
    {
        return CreateRange( data, data + TotalBodyCount(offsets) );
    }

    template<typename DataType>
    Range<DataType *> CreateStaticDataRange( ElementContainer::Offsets const & offsets, DataType * data )
    {
        return CreateRange( data + StaticBodyStart(offsets), data + StaticBodyEnd(offsets) );
    }

    template<typename DataType>
    Range<DataType *> CreateKinematicDataRange( ElementContainer::Offsets const & offsets, DataType * data )
    {
        return CreateRange( data + KinematicBodyStart(offsets), data + KinematicBodyEnd(offsets) );
    }

    template<typename DataType>
    Range<DataType *> CreateRigidDataRange( ElementContainer::Offsets const & offsets, DataType * data )
    {
        return CreateRange( data + RigidBodyStart(offsets), data + RigidBodyEnd(offsets) );
    }

    template<typename DataType>
    Range<DataType *> CreateDynamicDataRange( ElementContainer::Offsets const & offsets, DataType * data )
    {
        return CreateRange( data + DynamicBodyStart(offsets), data + DynamicBodyEnd(offsets) );
    }
}
