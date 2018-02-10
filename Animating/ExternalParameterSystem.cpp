#include "ExternalParameterSystem.h"

#include "CircleBlenderSystem.h"

#include <Math\MathFunctions.h>

#include <algorithm>

namespace
{
    // Returns speed relative to current rotation
    Math::Float3 CalculateDirectionalSpeed(Orientation const & orientation, Velocity const & velocity)
    {
        return Math::Rotate(velocity, Conjugate(orientation.rotation));
    }
}


namespace Animating
{

    // Use the input data to set a speed factor on the input StateData,
    // depending on what kind of input data is used
    void ApplySpeedModifier(Math::Float3 const velocity,
							float const rotational_offset,
                            StateData& state_data)
    {
        auto external_input_type = state_data.external_parameter_type;

        switch (external_input_type)
        {
            case ExternalParameterType::HorizontalMovement:
            {
                auto norm = Math::Norm(Math::Float2(velocity.x, velocity.y));
                state_data.speed_factor = state_data.external_parameter_factor * norm;
                break;
            }
            case ExternalParameterType::CameraAngle:
			case ExternalParameterType::RotationalOffset:
			{
				state_data.speed_factor = 1.0f + state_data.external_parameter_factor * rotational_offset;
				break;
			}
            case ExternalParameterType::None:
            {
                // This one has no actual effect...
                state_data.speed_factor = 1.0f;
                break;
            }
            default:
            {
                assert( false );
            }
        }
    }


    // Calculate a 2d position in the CircleBlender
    // based on the external input of the used type
    Math::Float2 CalculateSamplePosition(ExternalParameterType const parameter_type,
                                        Math::Float3 const velocity,
                                        Math::Float3 const camera_angles,
										float const rotational_offset)
    {
        Math::Float2 sample_position = 0;

        // Convert the external input into a 2d format for the sampling
        switch (parameter_type)
        {
            case ExternalParameterType::HorizontalMovement:
            {
                sample_position = Math::Float2(velocity.x, velocity.y);
                break;
            }
            case ExternalParameterType::CameraAngle:
            {
                auto vertical_angle = camera_angles.x - float(Math::c_PI_div_2);
                sample_position = Math::Float2(vertical_angle, 0.0f);
                break;
            }
			case ExternalParameterType::RotationalOffset:
			{
				sample_position = Math::Float2(rotational_offset, 0.0f);
				break;
			}
            case ExternalParameterType::None:
            {
                sample_position = 0;
                break;
            }
            default:
            {
                assert( false && "Circle blender sampling not implemented for parameter type");
            }
        }

        return sample_position;
    }


    // Use the input data to sample the CircleBlender of the AnimationState
    // and set the individual sequence weights accordingly
    void SampleCircleBlender(AnimationBlenderContainer const & blender_container,
                            AnimationBlenderID const & blender_id,
                            // External input
                            Math::Float3 const velocity,
                            Math::Float3 const camera_angles,
							float const rotational_offset,
                            // Resulting sequence weights
                            Range<float *> blender_sequence_weights)
    {
        // Get circle blender info and update otherwise
        auto blender_info = blender_container.m_blender_infos[blender_id.index];
        assert( Size( blender_sequence_weights ) == blender_info.node_count );
        auto blender_nodes = CreateRange(blender_container.m_nodes.data() + blender_info.node_offset, blender_info.node_count);

        // Get used input parameter for blender
        auto blender_parameter = blender_info.parameter_type;
        assert(blender_parameter != ExternalParameterType::None && "Blender used without input parameter type");

        // Use external input to calculate sample position
        auto sample_position = CalculateSamplePosition(blender_parameter, velocity, camera_angles, rotational_offset);

        // Sample the circle blender to determine the sequence weights of the AnimationState
        SampleCircleBlender(blender_nodes, sample_position, blender_sequence_weights);

        // Ensure that any weight was applied at all
        assert(!std::all_of(begin(blender_sequence_weights), end(blender_sequence_weights), [](float a) { return a == 0; }));
    }


    // Use the input data to sample the circle blenders
    // and calculate speed modifiers to update the AnimationStates with
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
                                                Range<float *> sequence_weights)
    {
        assert(entity_ids.size() == animation_infos.size());

        // Iterate over animating components
		for (auto i = 0u; i < entity_ids.size(); i++)
		{
			// Get entity and animation info for component
			auto entity_id = entity_ids[i];
			auto animation_info = animation_infos[i];

			// Get motion parameters
			auto velocity = GetOptionalVelocity(indexed_velocities, entity_id, 0);
			auto orientation = indexed_orientations.orientations[indexed_orientations.indices[entity_id.index]];
			auto rotated_velocity = CalculateDirectionalSpeed(orientation, velocity);

			// TODO: This should be done in the logic, and ideally only when actually used
			auto old_orientation = indexed_orientations.previous_orientations[indexed_orientations.indices[entity_id.index]];

			auto axis = Math::Float3(1.0f, 0.0f, 0.0f);
			auto old_rotated = Math::Rotate(axis, old_orientation.rotation);
			auto new_rotated = Math::Rotate(axis, orientation.rotation);
			auto rotational_offset_z = Math::ZRotationBetweenAxes(old_rotated, new_rotated);

            // Iterate over animation states of current component
            auto states_end = animation_info.offset + animation_info.count;
            for (auto j = animation_info.offset; j < states_end; j++)
            {
                // Get state info and data
                auto& state_info = state_infos[j];
                auto& state_data = state_datas[j];

                // Use optional speed modifier based on input
                ApplySpeedModifier(rotated_velocity, rotational_offset_z, state_data);

                // Get blender id and info
                auto blender_id = state_data.blender;

                // Skip if no valid blend tree
                if (blender_id.index != c_invalid_entity_id.index)
                {
                    // Get weights of sequences for current AnimationState
                    auto blender_sequence_weights = CreateRange(begin(sequence_weights) + state_info.offset, state_info.count);

                    SampleCircleBlender(blender_container,
                                        blender_id,
                                        rotated_velocity,
                                        camera_angles,
										rotational_offset_z,
                                        blender_sequence_weights);
                }
            }
        }
    }
}