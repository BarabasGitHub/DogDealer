#include "PersistentConstraints.h"

#include <Math\FloatTypes.h>
#include <Utilities\ContainerHelpers.h>

using namespace Physics;


void Physics::Remove(Range<BodyID const*> ids, PerstistentConstraints & self)
{
    for(auto id: ids)
    {
        Remove(id, self);
    }
}

void Physics::Remove(BodyID id, PerstistentConstraints & self)
{
    RemoveIDWithSwapAndPrune(
        id,
        self.distance_constraints.bodies,
        self.distance_constraints.attachment_points,
        self.distance_constraints.distances,
        self.distance_constraints.minmax_forces
        );

    RemoveIDWithSwapAndPrune(
        id,
        self.position_constraints.bodies,
        self.position_constraints.directions,
        self.position_constraints.attachment_points,
        self.position_constraints.distances,
        self.position_constraints.minmax_forces
        );

    RemoveIDWithSwapAndPrune(
        id,
        self.rotation_constraints.bodies,
        self.rotation_constraints.rotation_normals,
        self.rotation_constraints.target_angles,
        self.rotation_constraints.minmax_torques
        );

    RemoveIDWithSwapAndPrune(
        id,
        self.velocity_constraints.bodies,
        self.velocity_constraints.directions,
        self.velocity_constraints.attachment_points,
        self.velocity_constraints.target_speeds,
        self.velocity_constraints.minmax_forces
        );
}
