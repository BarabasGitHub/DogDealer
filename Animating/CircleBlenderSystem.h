#pragma once

#include "Structures.h"

#include <Conventions\Velocity.h>

#include <Utilities\Range.h>


namespace Animating
{
    class AnimationBlenderContainer;

    void SampleCircleBlender(Range<CircleBlenderNode const *> blender_nodes, const Math::Float2 parameters, Range<float *> weights);

    void ApplyMotionRules(Math::Float3 velocity, Range<CircleBlenderNode const *> blender_nodes, Range<float *> weights);
    void ApplyCameraRules(Math::Float3 const camera_angles, Range<CircleBlenderNode const *> blender_nodes, Range<float *> weights);
    void UpdateCircleBlenders(std::vector<EntityID> const & entity_ids, std::vector<AnimationInfo> const & animation_infos, std::vector<StateInfo>& state_infos, std::vector<StateData>& state_datas, Range<float *> sequence_weights, IndexedOrientations const & indexed_orientations, IndexedVelocities const & indexed_velocities, AnimationBlenderContainer const & blender_container, Math::Float3 const camera_angles);
}