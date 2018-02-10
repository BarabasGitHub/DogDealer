#include "Controllers.h"

#include <Utilities\VectorHelper.h>

#include <Math\MathFunctions.h>

using namespace Logic;

namespace
{
	// Loop over vector, search for entity with given index
	// and execute Swap and prune if found
	void SearchAndPruneEntityEntry(EntityID const & entity_id, std::vector<EntityID> & entities)
	{
		for (auto i = 0u; i < entities.size(); i++)
		{
			auto entry_id = entities[i].index;
			if (entry_id == entity_id.index)
			{
				SwapAndPrune(i, entities);
			}
		}
	}

}

void Controllers::AddkinematicCameraRotationController( EntityID entity_id )
{
    m_kinematic_camera_rotation_controllers.emplace_back( entity_id );
}

void Controllers::AddTorqueCameraRotationController(EntityID entity_id)
{
	m_torque_camera_rotation_controllers.push_back(entity_id);
}

void Controllers::AddArrowKeyController(EntityID entity_id)
{
	m_arrow_key_controllers.push_back(entity_id);
}

void Controllers::AddMotionKeyForceController( EntityID entity_id )
{
    m_motion_key_force_controllers.push_back( entity_id );
}


void Controllers::AddThrowController( EntityID entity_id )
{
    m_throw_controllers.push_back( entity_id );
}

void Controllers::AddStrikeController(EntityID entity_id)
{
    m_strike_controllers.push_back(entity_id);
}

// Search in all controller vectors for entity and remove found entries
void Controllers::RemoveEntityControllers(EntityID const & entity_id)
{
	SearchAndPruneEntityEntry(entity_id, m_kinematic_camera_rotation_controllers);
	SearchAndPruneEntityEntry(entity_id, m_torque_camera_rotation_controllers);

	SearchAndPruneEntityEntry(entity_id, m_arrow_key_controllers);
	SearchAndPruneEntityEntry(entity_id, m_motion_key_force_controllers);

	SearchAndPruneEntityEntry(entity_id, m_throw_controllers);
    SearchAndPruneEntityEntry(entity_id, m_strike_controllers);
}