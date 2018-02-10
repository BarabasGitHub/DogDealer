#include "CircleBlenderSystem.h"

#include "AnimationBlenderContainer.h"

#include <Math\VectorAlgorithms.h>
#include <Math\MathFunctions.h>

#include <numeric>

namespace Animating
{
    void SampleCircleBlender(Range<CircleBlenderNode const *> blender_nodes, const Math::Float2 parameters, Range<float *> weights)
    {
        assert(Size(weights) == Size(blender_nodes));
        auto circle_count = Size(blender_nodes);

        auto closest_circle_index = circle_count;
        auto smallest_distance = std::numeric_limits<float>::max();
        // Iterate over circles
        for (auto i = 0u; i < circle_count; i++)
        {
            // Get offset to circle center
            Math::Float2 offset = blender_nodes[i].center - parameters;

            auto distance = Math::Norm(offset);

            // keep track of the closest circle
            if (smallest_distance > distance)
            {
                smallest_distance = distance;
                closest_circle_index = i;
            }

            // Use relative distance to circle as weight, if inside
            weights[i] = Math::Max(1 - distance / blender_nodes[i].radius, 0.f);
        }

        // Normalize weights
        auto weight_sum = std::accumulate(begin(weights), end(weights), 0.f);
        // Normalize weights if in at least one circle
        if (weight_sum > 0)
        {
            Math::Divide(weights, weight_sum);
        }
        else // Use clostest circle with full weight
        {
            weights[closest_circle_index] = 1;
        }
    }
}