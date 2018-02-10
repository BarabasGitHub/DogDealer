#pragma once

#include <Conventions\EntityID.h>
#include <Conventions\Orientation.h>
#include <Conventions\Force.h>
#include <Conventions\Velocity.h>

#include <array>

namespace Logic{

	struct Controllers
    {
        std::vector<EntityID> m_kinematic_camera_rotation_controllers;
		std::vector<EntityID> m_torque_camera_rotation_controllers;
		std::vector<EntityID> m_arrow_key_controllers;
        std::vector<EntityID> m_motion_key_force_controllers;

        std::vector<EntityID> m_throw_controllers;
        std::vector<EntityID> m_strike_controllers;

        void AddkinematicCameraRotationController( EntityID entity_id );
        void AddTorqueCameraRotationController( EntityID entity_id );
        void AddArrowKeyController( EntityID entity_id );
        void AddMotionKeyForceController( EntityID entity_id );

        void AddThrowController( EntityID entity_id );
        void AddStrikeController(EntityID entity_id);

		void RemoveEntityControllers(EntityID const & entity_id);
	};
}
