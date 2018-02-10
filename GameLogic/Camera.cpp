#pragma once
#include "Camera.h"

#include <Math\TransformFunctions.h> // temp for camera transform
#include <Math\MathFunctions.h>

#include <Input\InterfaceInput.h>
#include <Conventions\Orientation.h>


using namespace Logic;

namespace
{
    Math::Quaternion RotationFromCameraAngles( Math::Float3 angles )
    {
        auto rotation_x = Math::XAngleToQuaternion( angles.x );
        auto rotation_z = Math::ZAngleToQuaternion( angles.z );

        return rotation_z * rotation_x;
    }
}


Camera::Camera()
{
    m_perspective_view.field_of_view = Math::DegreesToRadians( 45 );
    m_perspective_view.aspect_ratio = 16.f / 9.f;
    m_perspective_view.far_z = 1000;
    m_perspective_view.near_z = .1f;
}


void Camera::Update(const float time_step, CameraInput const & input, IndexedOrientations const & indexed_orientations)
{
    // Camera zoom
    this->m_targeting_offset.z -= input.zoom;

    Math::Float3 angles;
    if(this->m_angle_control)
    {
        angles = input.angles;
    }
    else
    {
        angles = 0;
    }

    auto camera_target = this->m_target_entity;
    if (camera_target == c_invalid_entity_id)
    {
        this->Update( time_step, input.movement, angles );
    }
    else
    {
        auto orientation_index = indexed_orientations.indices[camera_target.index];
        auto target_position = indexed_orientations.orientations[orientation_index].position;
        this->UpdateFollow(time_step, angles, target_position);
    }
}

void Camera::Update(const float time_step, Math::Float3 movement, Math::Float3 angles )
{
	using namespace Math;

	// Keyboard Movement
    movement *= 3 * time_step;

    auto local_movement = Math::Rotate( movement, m_rotation );
    m_position += local_movement;

    // Mouse Rotation
    angles *= 10 * time_step;

    m_angles = AddAngles(m_angles, angles);
    m_rotation = RotationFromCameraAngles(m_angles);
}


void Camera::UpdateFollow(const float time_step, Math::Float3 angles, Math::Float3 target_position)
{
	// Mouse Rotation
	angles *= 10 * time_step;

	m_angles = AddAngles(m_angles, angles);
	m_rotation = RotationFromCameraAngles(m_angles);

	m_position = target_position + m_targeting_pivot_offset;
	m_position += Math::Rotate(m_targeting_offset, m_rotation);
}

void Camera::SetAngles( Math::Float3 angles )
{
    m_angles = AddAngles(Math::Float3(0), angles);
    m_rotation = RotationFromCameraAngles(m_angles);
}

void Camera::SetTarget(EntityID entity_id, Math::Float3 position_offset, Math::Float3 pivot_offset)
{
	m_target_entity = entity_id;
	m_targeting_offset = position_offset;
	m_targeting_pivot_offset = pivot_offset;
}

void Camera::ClearTarget()
{
	m_target_entity = c_invalid_entity_id;
}


Orientation Camera::GetOrientation() const
{
    return {m_position, m_rotation};
}
