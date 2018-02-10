#include "Structures.h"
#include "AnimationTemplates.h"

#include <Conventions\AnimatingInstructions.h>

#include <Utilities\Range.h>

namespace Animating{

	class StateContainer;
	class AnimationBlenderContainer;

	void UpdateAnimationInfos(std::vector<AnimationInfo> & animation_infos, StateContainer & state_container, const float time_step);

    void AdvanceStateProgress(std::vector<StateData>& state_datas, std::vector<uint32_t>& expired_states, const float time_step);
    void AdvanceStateWeights(std::vector<StateLogic>& state_logics, std::vector<StateData>& state_datas, std::vector<uint32_t>& faded_states, const float time_step);

    void FadeExpiredStates(std::vector<StateData>& state_datas, std::vector<StateLogic>& state_logics, std::vector<uint32_t> const & expired_states);

    void PruneStateInfos( Range<AnimationInfo *> animation_infos, Range<uint32_t *> faded_states, StateContainer & state_container );

    void AddStates(EntityAnimatingInstructions const & instructions, std::vector<uint32_t> const & entity_to_data, std::vector<AnimationInfo> & animation_infos, StateContainer & state_container, std::vector<AnimationTemplate> const & templates);

    void FadeOutStates(EntityAnimatingInstructions const & instructions, std::vector<uint32_t> const & entity_to_data, std::vector<AnimationInfo> const & animation_infos, std::vector<StateData> const & state_datas, std::vector<AnimationTemplate> const & templates, std::vector<StateLogic> & state_logics);
}