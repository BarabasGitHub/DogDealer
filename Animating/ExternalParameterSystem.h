#pragma once 
#include "AnimationBlenderContainer.h"

#include <Conventions\Velocity.h>
#include <Utilities\Range.h>

namespace Animating
{
    void UpdateCircleBlendersAndSpeedModifiers(AnimationBlenderContainer const & blender_container,
                                                // External input:
                                                IndexedOrientations const & indexed_orientations,
                                                IndexedVelocities const & indexed_velocities,
                                                Math::Float3 const camera_angles,
                                                // Animating components
                                                std::vector<EntityID> const & entity_ids,
                                                std::vector<AnimationInfo> const & animation_infos,
                                                std::vector<StateInfo> const & state_infos,
                                                std::vector<StateData>& state_datas,
                                                Range<float *> sequence_weights);
}