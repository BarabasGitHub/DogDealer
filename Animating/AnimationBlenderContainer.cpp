#include "AnimationBlenderContainer.h"

#include <FileLayout\VertexDataType.h>
#include <Utilities\StreamHelpers.h>

using namespace Animating;

AnimationBlenderID AnimationBlenderContainer::CreateCircleBlender(CircleBlenderDescription const & description)
{
    AnimationBlenderInfo info;
    info.node_count = unsigned(description.nodes.size());
    info.node_offset = unsigned(m_nodes.size());

    info.parameter_type = description.parameter_type;

    auto info_index = unsigned(m_blender_infos.size());

    m_blender_infos.emplace_back( info );

    for( auto & node : description.nodes )
    {
        m_nodes.emplace_back( CircleBlenderNode{ node.center, node.radius } );
    }

    return{ info_index, 0 };
}