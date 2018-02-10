#include "TimerSystem.h"

//#include <Utilities\ParallelIterator.h>
#include <Utilities\VectorHelper.h>
#include <algorithm>

namespace Logic{

    namespace{

        void RemoveEntityFromTimer(std::vector<EntityID>& timer_entities,
                                    std::vector<float>& remaining_times,
                                    Range<EntityID const *> entity_ids)
        {
            auto iterator_index = 0u;

            // Iterate over all timer entries,
            // swapping entries for the target entities to the back and pruning them
            while (iterator_index < timer_entities.size())
            {
                // Get target entity of timer and assume no deletion to be necessary
                auto timer_entity = timer_entities[iterator_index];

                // Find timer entity in the to-be-deleted entities
                auto delete_entry = std::find( begin( entity_ids ), end( entity_ids ), timer_entity ) != end( entity_ids );

                // Remove found entries or advance to next one
                if (delete_entry)
                {
                    // Swap with last entry and inspect the same position next
                    SwapAndPrune(iterator_index, timer_entities);
                    SwapAndPrune(iterator_index, remaining_times);
                }
                else
                {
                    ++iterator_index;
                }
            }
        }

        template<class VectorType>
        void RemoveEntityFromParameterTimer(std::vector<EntityID>& timer_entities,
                                    std::vector<float>& remaining_times,
                                    VectorType& timer_parameters,
									Range<EntityID const *> entity_ids)
        {
            auto iterator_index = 0;

            // Iterate over all timer entries,
            // swapping entries for the target entities to the back and pruning them
            while (iterator_index < timer_entities.size())
            {
                // Get target entity of timer and assume no deletion to be necessary
                auto timer_entity = timer_entities[iterator_index];

                // Find timer entity in the to-be-deleted entities
                auto delete_entry = std::find( begin( entity_ids ), end( entity_ids ), timer_entity ) != end( entity_ids );

                // Remove found entries or advance to next one
                if (delete_entry)
                {
                    // Swap with last entry and inspect the same position next
                    SwapAndPrune(iterator_index, timer_entities, remaining_times, timer_parameters);
                }
                else
                {
                    ++iterator_index;
                }
            }
        }

        // Subtract the time step from all remaining times and
        // return the index of the first expired timer.
        // Due to the sorting all later timers can be assumed to be expired as well.
        unsigned UpdateRemainingTimes(float const time_step, std::vector<float> & remaining_times)
        {
            // Advance all timers and check for
            auto i = 0u;
            while (i < remaining_times.size())
            {
                // Subtract from time
                remaining_times[i] -= time_step;
                if (remaining_times[i] <= 0.0f)
                {
                    break;
                }
                ++i;
            }

            return i;
        }

        void SortTimers(std::vector<EntityID> & entity_ids,
                        std::vector<float> & remaining_times)
        {
            // Create mapping of sorted remaining times
            auto mapping = std::vector<unsigned>(remaining_times.size());
            for (auto i = 0u; i < remaining_times.size(); i++) mapping[i] = i;

            // Create mapping according to sorted priorities
            std::sort(begin(mapping), end(mapping), [&remaining_times](unsigned a, unsigned b)
            {
                return remaining_times[a] > remaining_times[b];
            });

            entity_ids = Reorder(entity_ids, mapping);
            remaining_times = Reorder(remaining_times, mapping);
        }

        template<class VectorType>
        void SortTimers(std::vector<EntityID> & entity_ids,
            std::vector<float> & remaining_times,
                        VectorType& timer_parameters)
        {
            // Create mapping of sorted remaining times
            auto mapping = std::vector<unsigned>(remaining_times.size());
            for (auto i = 0u; i < remaining_times.size(); i++) mapping[i] = i;

            // Create mapping according to sorted priorities
            std::sort(begin(mapping), end(mapping), [&remaining_times](unsigned a, unsigned b)
            {
                return remaining_times[a] > remaining_times[b];
            });

            entity_ids = Reorder(entity_ids, mapping);
            remaining_times = Reorder(remaining_times, mapping);
            timer_parameters = Reorder(timer_parameters, mapping);
        }
    }

    void UpdateTimerProgress(
        float const time_step,
        ReleaseThrowTimers & timers,
        std::vector<EntityID> & expired_timer_entities)

	{
        // Update remaining times and get index of first expired timer
        auto i = UpdateRemainingTimes(time_step, timers.remaining_times);

        // Store expired timer parameters
        expired_timer_entities.insert(end(expired_timer_entities), begin(timers.entity_ids) + i, end(timers.entity_ids));

        timers.entity_ids.resize(i);
        timers.remaining_times.resize(i);
	}


    void UpdateTimerProgress(
        float const time_step,
        EntityDeadTimers & timers,
        std::vector<EntityID> & expired_timer_entities)
    {
        // Update remaining times and get index of first expired timer
        auto i = UpdateRemainingTimes(time_step, timers.remaining_times);

        // Store expired timer parameters
        expired_timer_entities.insert(end(expired_timer_entities), begin(timers.entity_ids) + i, end(timers.entity_ids));
     
        timers.entity_ids.resize(i);
        timers.remaining_times.resize(i);
    }


    void UpdateTimerProgress(
        float const time_step,
        EntityPerishedTimers & timers,
        std::vector<EntityID> & expired_timer_entities)
    {
        // Update remaining times and get index of first expired timer
        auto i = UpdateRemainingTimes(time_step, timers.remaining_times);

        // Store expired timer parameters
        expired_timer_entities.insert(end(expired_timer_entities), begin(timers.entity_ids) + i, end(timers.entity_ids));
    
        timers.entity_ids.resize(i);
        timers.remaining_times.resize(i);
    }


    void SortTimers(ReleaseThrowTimers& timers)
    {
        SortTimers(timers.entity_ids, timers.remaining_times);
    }


    void SortTimers(EntityDeadTimers& timers)
    {
        SortTimers(timers.entity_ids, timers.remaining_times);
    }


    void SortTimers(EntityPerishedTimers& timers)
    {
        SortTimers(timers.entity_ids, timers.remaining_times);
    }


    void RemoveEntityTimers(ReleaseThrowTimers & timers,                        
                        Range<EntityID const *> entity_ids)
    {
        RemoveEntityFromTimer(timers.entity_ids, timers.remaining_times, entity_ids);
       
        SortTimers(timers);
    }

	void RemoveEntityTimers(EntityDeadTimers & timers,
		Range<EntityID const *> entity_ids)
	{
		RemoveEntityFromTimer(timers.entity_ids, timers.remaining_times, entity_ids);
		
		SortTimers(timers);		
	}

	void RemoveEntityTimers(EntityPerishedTimers & timers,
		Range<EntityID const *> entity_ids)
	{
		RemoveEntityFromTimer(timers.entity_ids, timers.remaining_times, entity_ids);
		
		SortTimers(timers);
	}
}