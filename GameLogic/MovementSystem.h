#include <Conventions\Velocity.h>

#include <Utilities\Range.h>

#include <map>
#include <vector>

namespace Logic{

	struct EntityAbilities;

	struct MovementSystem
	{
		
	public:
		
        // stores all idle entities followed by all moving entities
        std::vector<EntityID> idle_and_moving_entities;
        uint32_t moving_entities_start_index = 0;

        // stores the entities that are newly added and do not yet have any state 
        std::vector<EntityID> new_entities;

		void RemoveEntity(EntityID const & entity_id);
	};

    // separates the idle entities from the now moving entities and returns the index at which the moving entities start
    // apart from the idle/moving state, the entities stay in the same order
    uint32_t SparateIdleFromMovingEntities( std::vector<EntityID>& entities, IndexedVelocities const & velocities );
    
    // add the movement or idle animation to the instructions, also inserting the new entities
    // the entities start with all idle entities and are followed by all moving entities, they will be re-separated according to the new velocities
    // the last two inputs return the entities that went from moving to idle and from idle to moving
    uint32_t ProcessMovement( Range<EntityID const *> new_entities, std::vector<EntityID> & entities, unsigned const moving_entities_start_index,
                              IndexedVelocities const & velocities, float const start_moving_speed_threshold, float const stop_moving_speed_threshold,
                              Range<EntityID const *> & now_idle_entities, Range<EntityID const *> & now_moving_entities );

    // adds the new entities to the existing entities and returns the new moving entities start index
    uint32_t AddNewEntities( Range<EntityID const *> new_entities, std::vector<EntityID> & existing_entities, uint32_t moving_entities_start_index,
                             IndexedVelocities const & velocities, float const speed_threshold );
}