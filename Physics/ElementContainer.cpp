#include "ElementContainer.h"

#include "Algorithms.h"
#include "InertiaFunctions.h"

#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\OrientedBox.h>
#include <Utilities\StdVectorFunctions.h>
#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>
#include <Math\FloatMatrixOperators.h>

void Physics::AddRigidBodyComponent(
        BodyID body_id,
        Orientation orientation,
        Math::Float3 center_of_mass,
        Movement movement,
        Inertia inverse_inertia,
        BoundingShapes::AxisAlignedBox aabox,
        float bounciness,
        float friction_factor,
        ElementContainer & self )
{
    auto index = RigidBodyEnd(self.offsets);

    Insert(body_id, index, self.storage.common.body_ids);
    Insert(orientation, index, self.storage.common.orientations);
    Insert(orientation, index, self.storage.common.previous_orientations);
    Insert(aabox, index, self.storage.common.broad_bounds);
    Insert(TransformByOrientation(aabox, orientation), index, self.storage.common.transformed_broad_bounds);
    Insert(bounciness, index, self.storage.common.bounce_factors);
    Insert(friction_factor, index, self.storage.common.friction_factors);

    auto rigid_index = index - RigidBodyStart(self.offsets);
    Insert(center_of_mass, rigid_index, self.storage.rigid_body.centers_of_mass);
    Insert(movement, rigid_index, self.storage.rigid_body.movements);
    Insert(inverse_inertia, rigid_index, self.storage.rigid_body.inverse_inertias);
    Insert(Math::Float3(0), rigid_index, self.storage.rigid_body.forces);

    InsertIndexInIndices( self.storage.body_to_element, body_id.index, index );

    ChangedRigidBodyCount(1, self.offsets);
    UpdatePointers(self);
}


void Physics::AddKinematicComponent(
    BodyID body_id,
    Orientation orientation,
    BoundingShapes::AxisAlignedBox aabox,
    float bounciness,
    float friction_factor,
    ElementContainer & self)
{
    auto index = KinematicBodyEnd(self.offsets);

    Insert(body_id, index, self.storage.common.body_ids);
    Insert(orientation, index, self.storage.common.orientations);
    Insert(orientation, index, self.storage.common.previous_orientations);
    Insert(aabox, index, self.storage.common.broad_bounds);
    Insert(TransformByOrientation(aabox, orientation), index, self.storage.common.transformed_broad_bounds);
    Insert(bounciness, index, self.storage.common.bounce_factors);
    Insert(friction_factor, index, self.storage.common.friction_factors);

    InsertIndexInIndices( self.storage.body_to_element, body_id.index, index );

    ChangedKinematicBodyCount(1, self.offsets);
    UpdatePointers(self);
}


void Physics::AddStaticComponent(
    BodyID body_id,
    Orientation orientation,
    BoundingShapes::AxisAlignedBox aabox,
    float bounciness,
    float friction_factor,
    Physics::ElementContainer & self)
{
    aabox = BoundingShapes::TransformByOrientation( aabox, orientation );

    auto index = StaticBodyEnd(self.offsets);

    Insert(body_id, index, self.storage.common.body_ids);
    Insert(orientation, index, self.storage.common.orientations);
    Insert(orientation, index, self.storage.common.previous_orientations);
    Insert(aabox, index, self.storage.common.broad_bounds);
    Insert(aabox, index, self.storage.common.transformed_broad_bounds);
    Insert(bounciness, index, self.storage.common.bounce_factors);
    Insert(friction_factor, index, self.storage.common.friction_factors);

    InsertIndexInIndices( self.storage.body_to_element, body_id.index, index );

    ChangedStaticBodyCount(1, self.offsets);
    UpdatePointers(self);
}


void Physics::ReplaceWithRigidBodyComponent(
    BodyID body_id,
    Math::Float3 center_of_mass,
    Inertia inverse_inertia,
    BoundingShapes::AxisAlignedBox aabox,
    float bounciness,
    float friction_factor,
    ElementContainer & self )
{
    auto index = self.pointers.body_to_element[body_id.index];
    auto orientation = self.pointers.orientations[index];
    Movement movement;
    if( IsRigidBody( body_id, self ) )
    {
        // preserve the velocity instead of the momentum
        // mostly because it goes wrong when transforming from a 'locked' body to a non-'locked' body.
        movement = self.pointers.movements[index];
        auto current_inverse_inertia = self.pointers.inverse_inertias[index];
        auto new_inertia = Invert( inverse_inertia );
        movement.momentum = new_inertia.mass * current_inverse_inertia.mass * movement.momentum;
        movement.angular_momentum = current_inverse_inertia.moment * ( current_inverse_inertia.moment * movement.angular_momentum );

        // correct for the possible change of the center of mass
        auto old_center_of_mass = self.pointers.centers_of_mass[index];
        CorrectForCenterOfMassBackward( CreateRange( &old_center_of_mass, 1 ), CreateRange( &orientation, 1 ) );
    }
    else
    {
        movement.momentum = 0;
        movement.angular_momentum = 0;
    }

    CorrectForCenterOfMassForward( CreateRange( &center_of_mass, 1 ), CreateRange( &orientation, 1 ) );
    RemoveBodies(CreateRange (&body_id, 1), self);
    AddRigidBodyComponent(body_id, orientation, center_of_mass, movement, inverse_inertia, aabox, bounciness, friction_factor, self);
}


void Physics::ReplaceWithKinematicComponent(
    BodyID body_id,
    BoundingShapes::AxisAlignedBox aabox,
    float bounciness,
    float friction_factor,
    ElementContainer & self)
{
    auto index = self.pointers.body_to_element[body_id.index];
    auto orientation = self.pointers.orientations[index];
    if( IsRigidBody( body_id, self ) )
    {
        auto center_of_mass = self.pointers.centers_of_mass[index];
        CorrectForCenterOfMassBackward( CreateRange( &center_of_mass, 1 ), CreateRange( &orientation, 1 ) );
    }
    RemoveBodies(CreateRange (&body_id, 1), self);
    AddKinematicComponent(body_id, orientation, aabox, bounciness, friction_factor, self);
}


void Physics::ReplaceWithStaticComponent(
    BodyID body_id,
    BoundingShapes::AxisAlignedBox aabox,
    float bounciness,
    float friction_factor,
    ElementContainer & self)
{
    auto index = self.pointers.body_to_element[body_id.index];
    auto orientation = self.pointers.orientations[index];
    if( IsRigidBody( body_id, self ) )
    {
        auto center_of_mass = self.pointers.centers_of_mass[index];
        CorrectForCenterOfMassBackward( CreateRange( &center_of_mass, 1 ), CreateRange( &orientation, 1 ) );
    }
    RemoveBodies(CreateRange (&body_id, 1), self);
    AddStaticComponent(body_id, orientation, aabox, bounciness, friction_factor, self);
}


void Physics::RemoveBodies( Range<BodyID const *> body_ids_to_be_removed, ElementContainer & self )
{
    if(IsEmpty(body_ids_to_be_removed)) return;
    auto indices = RemoveIndices( self.storage.body_to_element, body_ids_to_be_removed );

    std::sort( begin( indices ), end( indices ) );

    RemoveEntries(self.storage.common.body_ids, indices);
    RemoveEntries(self.storage.common.orientations, indices);
    RemoveEntries(self.storage.common.previous_orientations, indices);
    RemoveEntries(self.storage.common.broad_bounds, indices);
    RemoveEntries(self.storage.common.transformed_broad_bounds, indices);
    RemoveEntries(self.storage.common.bounce_factors, indices);
    RemoveEntries(self.storage.common.friction_factors, indices);

    auto indices_begin = begin( indices );

    uint32_t removed_static_components;
    // remove static components
    {
        assert(StaticBodyStart(self.offsets) == 0);
        // auto start = std::lower_bound(begin(indices), end(indices), StaticDensityBodyStart(self.offsets)) - begin(indices);
        auto start = 0;
        // find the end and update the begin of where to search for indices
        indices_begin = std::lower_bound(indices_begin, end(indices), StaticBodyEnd(self.offsets));
        auto stop = indices_begin - begin(indices);
        // record how many we removed
        removed_static_components = uint32_t(stop - start);
    }

    // remove dynamic components
    {
        assert( DynamicBodyStart( self.offsets ) >= StaticBodyStart( self.offsets ) );
        auto start_offset = DynamicBodyStart( self.offsets );
        auto start = std::lower_bound( indices_begin, end( indices ), start_offset ) - begin( indices );
        // the dynamic bodies should be at the end, so all indices should be smaller
        assert( Last( indices ) <= DynamicBodyEnd( self.offsets ) );
        // update the begin position to search for the kinematic bodies
        indices_begin = begin( indices ) + start;
    }

    uint32_t removed_kinematic_components;
    // remove kinematic components
    {
        assert(KinematicBodyStart(self.offsets) == DynamicBodyStart(self.offsets));
        // auto start_offset = KinematicBodyStart(self.offsets);
        // offset is zero, because the indices are already adjusted for the dynamic bodies (because they overlap and start at the same place as kinematic bodies)
        //auto start = std::lower_bound(indices_begin, end(indices), 0u ) - begin(indices);
        auto start = indices_begin - begin(indices);
        // find the end and update the begin of where to search for indices
        indices_begin = std::lower_bound(indices_begin, end(indices), KinematicBodyEnd(self.offsets) );
        auto stop = indices_begin - begin(indices);
        // nothing to adjust for kinematic, because there are no special kinematic components
        // record how many we removed
        removed_kinematic_components = uint32_t( stop - start );
    }

    uint32_t removed_rigid_components;
    // remove rigid body components
    {
        assert(RigidBodyStart(self.offsets) >= KinematicBodyStart(self.offsets));
        // calculate the offset
        auto start_offset = RigidBodyStart( self.offsets );
        auto start = std::lower_bound(indices_begin, end(indices), start_offset ) - begin(indices);
        // the rigid bodies should be at the end, so all indices should be smaller
        assert( Last(indices) < RigidBodyEnd(self.offsets));
        // auto stop = std::lower_bound(indices_begin, end(indices), RigidBodyEnd(self.offsets)) - begin(indices);
        // adjust all rigid body indices (which go to the end)
        // such that they're correct for the rigid body storage
        auto indices_range = CreateRange( indices, start, Size(indices) );
        for( auto & i : indices_range )
        {
            i -= start_offset;
        }
        RemoveEntries(self.storage.rigid_body.centers_of_mass, indices_range);
        RemoveEntries(self.storage.rigid_body.movements, indices_range);
        RemoveEntries(self.storage.rigid_body.inverse_inertias, indices_range);
        RemoveEntries(self.storage.rigid_body.forces, indices_range);
        // record how many we removed
        removed_rigid_components = uint32_t( Size(indices) - start );
    }

    ChangedStaticBodyCount( -int32_t(removed_static_components), self.offsets );
    ChangedKinematicBodyCount( -int32_t(removed_kinematic_components), self.offsets );
    ChangedRigidBodyCount( -int32_t(removed_rigid_components), self.offsets );

    UpdatePointers(self);
}


void Physics::UpdatePointers(ElementContainer & self)
{
    self.pointers.body_to_element = CreateRange(self.storage.body_to_element);
    self.pointers.body_ids = self.storage.common.body_ids.data();
    self.pointers.orientations = self.storage.common.orientations.data();
    self.pointers.previous_orientations = self.storage.common.previous_orientations.data();
    self.pointers.broad_bounds = self.storage.common.broad_bounds.data();
    self.pointers.transformed_broad_bounds = self.storage.common.transformed_broad_bounds.data();
    self.pointers.bounce_factors = self.storage.common.bounce_factors.data();
    self.pointers.friction_factors = self.storage.common.friction_factors.data();
    auto rigid_offset = RigidBodyStart(self.offsets);
    self.pointers.centers_of_mass = self.storage.rigid_body.centers_of_mass.data() - rigid_offset;
    self.pointers.movements = self.storage.rigid_body.movements.data() - rigid_offset;
    self.pointers.inverse_inertias = self.storage.rigid_body.inverse_inertias.data() - rigid_offset;
    self.pointers.forces = self.storage.rigid_body.forces.data() - rigid_offset;
}


bool Physics::IsStaticBody( BodyID id, ElementContainer const & self )
{
    auto index = self.storage.body_to_element[id.index];
    return (index >= StaticBodyStart(self.offsets)) & (index < StaticBodyEnd(self.offsets));
}


bool Physics::IsRigidBody(BodyID id, ElementContainer const & self)
{
    auto index = self.storage.body_to_element[id.index];
    return (index >= RigidBodyStart(self.offsets)) & (index < RigidBodyEnd(self.offsets));
}


bool Physics::IsKinematicBody(BodyID id, ElementContainer const & self)
{
    auto index = self.storage.body_to_element[id.index];
    return (index >= KinematicBodyStart(self.offsets)) & (index < KinematicBodyEnd(self.offsets));
}


bool Physics::IsDynamicBody(BodyID id, ElementContainer const & self)
{
    auto index = self.storage.body_to_element[id.index];
    return (index >= DynamicBodyStart(self.offsets)) & (index < DynamicBodyEnd(self.offsets));
}


uint32_t Physics::TotalBodyCount(ElementContainer const & self)
{
    return uint32_t(Size(self.storage.common.body_ids));
}


uint32_t Physics::TotalBodyCount(ElementContainer::Offsets const & self)
{
    return RigidBodyEnd(self);
}


uint32_t Physics::DynamicBodyStart(ElementContainer::Offsets const & self)
{
    return KinematicBodyStart(self);
}

uint32_t Physics::DynamicBodyEnd(ElementContainer::Offsets const & self)
{
    return RigidBodyEnd(self);
}

uint32_t Physics::KinematicBodyStart(ElementContainer::Offsets const & self)
{
    return StaticBodyEnd(self);
}

uint32_t Physics::KinematicBodyEnd(ElementContainer::Offsets const & self)
{
    return self.kinematic_body_end_index;
}

uint32_t Physics::RigidBodyStart(ElementContainer::Offsets const & self)
{
    return KinematicBodyEnd(self);
}

uint32_t Physics::RigidBodyEnd(ElementContainer::Offsets const & self)
{
    return self.rigid_body_end_index;
}

uint32_t Physics::StaticBodyStart(ElementContainer::Offsets const & /*self*/)
{
    return 0;
}

uint32_t Physics::StaticBodyEnd(ElementContainer::Offsets const & self)
{
    return self.static_body_end_index;
}


void Physics::ChangedRigidBodyCount(int32_t change, ElementContainer::Offsets & offsets)
{
    // offsets.static_body_end_index += change;
    // offsets.kinematic_body_end_index += change;
    offsets.rigid_body_end_index += change;
}

void Physics::ChangedKinematicBodyCount(int32_t change, ElementContainer::Offsets & offsets)
{
    // offsets.static_body_end_index += change;
    offsets.kinematic_body_end_index += change;
    offsets.rigid_body_end_index += change;
}

void Physics::ChangedStaticBodyCount(int32_t change, ElementContainer::Offsets & offsets)
{
    offsets.static_body_end_index += change;
    offsets.kinematic_body_end_index += change;
    offsets.rigid_body_end_index += change;
}
