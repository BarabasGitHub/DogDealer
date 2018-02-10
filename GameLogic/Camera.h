#pragma once

#include "DLL.h"

#include <Math\FloatTypes.h>
#include <Math\FloatMatrixTypes.h>

#include <Conventions\EntityID.h>
#include <Conventions\Orientation.h>
#include <Conventions\PerspectiveViewParameters.h>

struct IndexedOrientations;
struct CameraInput;

namespace Logic{

	struct GAMELOGIC_DLL Camera
    {
        Camera();

        void Update(const float time_step, CameraInput const & input, IndexedOrientations const & indexed_orientations);

        void Update(const float time_step, Math::Float3 movement, Math::Float3 angles);
		void UpdateFollow(const float time_step, Math::Float3 angles, Math::Float3 target_position);

        void SetAngles( Math::Float3 angles );
        void SetTarget( EntityID entity_id, Math::Float3 position_offset, Math::Float3 pivot_offset );

		void ClearTarget();

        Orientation GetOrientation() const;

        Math::Float3     m_angles = 0;
        Math::Float3     m_position = 0;
        Math::Quaternion m_rotation = Math::Identity();

        PerspectiveViewParameters m_perspective_view;

		// Targeting
        EntityID		m_target_entity = c_invalid_entity_id;
		Math::Float3	m_targeting_offset = 0;
		Math::Float3	m_targeting_pivot_offset = 0;

        // SCAMP
        bool m_angle_control = true;
	};

}
