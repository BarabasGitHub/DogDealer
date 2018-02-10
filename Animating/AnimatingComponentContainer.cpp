
#include "AnimatingComponentContainer.h"
#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>

namespace Animating
{
    AnimatingComponentContainer::AnimatingComponentContainer() :
        pose_indices( 1, 0 )
    {
    }


    void AnimatingComponentContainer::RemoveEntities( Range<EntityID const *> entities_to_be_removed )
    {
        auto indices = RemoveIndices( entity_to_element, entities_to_be_removed );

        std::sort( begin( indices ), end( indices ) );

        RemoveEntries( entity_ids, indices );
        RemoveEntries( skeleton_infos, indices );
        RemoveEntries( infos, indices );
        RemoveEntries( bone_states, pose_indices, indices );
        RemoveOffsets( pose_indices, indices );

        assert( entity_ids.size() == skeleton_infos.size() );
        assert( entity_ids.size() == infos.size() );
        assert( entity_ids.size() + 1 == pose_indices.size() );
        assert( pose_indices[0] == 0 );
    }


    void AnimatingComponentContainer::AddComponent( EntityID entity_id, SkeletonInfo skeleton_info, AnimationInfo animation_info, Range<BoneState const * > pose )
    {
        auto index = static_cast<unsigned>( entity_ids.size() );
        AddIndexToIndices( entity_to_element, entity_id.index, index );

        entity_ids.push_back( entity_id );
        skeleton_infos.push_back( skeleton_info );
        infos.push_back( animation_info );

        auto pose_offset = AddPose( pose );
        (void) pose_offset;
        assert( pose_offset + 1 == entity_ids.size() );
        assert( entity_ids.size() == skeleton_infos.size() );
        assert( entity_ids.size() == infos.size() );
    }


    unsigned AnimatingComponentContainer::AddPose( Range<BoneState const * > pose )
    {
        auto bone_count = unsigned( Size( pose ) );
        auto offset = unsigned( pose_indices.back() );
        auto indices_offset = unsigned( pose_indices.size() - 1 );

        pose_indices.push_back( offset + bone_count );
        bone_states.insert( end( bone_states ), begin( pose ), end( pose ) );

        return indices_offset;
    }
}