#include <Conventions\EntityID.h>

#include <Conventions\Orientation.h>
#include <Conventions\AnimationTemplateID.h>
#include <Conventions\AnimatingInstructions.h>
#include <Conventions\EntityTemplateID.h>

#include <Utilities\Range.h>

#include <vector>

namespace Logic{

    struct ReleaseThrowTimers
    {
        std::vector<EntityID> entity_ids;
        std::vector<float> remaining_times;
    };


    struct EntityDeadTimers
    {
        std::vector<EntityID> entity_ids;
        std::vector<float> remaining_times;
    };

    struct EntityPerishedTimers
    {
        std::vector<EntityID> entity_ids;
        std::vector<float> remaining_times;
    };

    void UpdateTimerProgress(
        float const time_step,
        ReleaseThrowTimers & timers,
        std::vector<EntityID> & expired_timer_entities);


    void UpdateTimerProgress(
        float const time_step,
        EntityDeadTimers & timers,
        std::vector<EntityID> & expired_timer_entities);

    void UpdateTimerProgress(
        float const time_step,
        EntityPerishedTimers & timers,
        std::vector<EntityID> & expired_timer_entities);

    void SortTimers(ReleaseThrowTimers & timers);
    void SortTimers(EntityDeadTimers & timers);
    void SortTimers(EntityPerishedTimers & timers);

    void RemoveEntityTimers(ReleaseThrowTimers & throwing_timers,
                            Range<EntityID const *> entity_ids);

	void RemoveEntityTimers(EntityDeadTimers & timers,
							Range<EntityID const *> entity_ids);

	void RemoveEntityTimers(EntityPerishedTimers & timers,
							Range<EntityID const *> entity_ids);
}