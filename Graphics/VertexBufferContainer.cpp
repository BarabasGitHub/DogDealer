#include "VertexBufferContainer.h"

#include <Utilities\ContainerHelpers.h>

using namespace Graphics;

VertexBufferID VertexBufferContainer::Add( VertexBufferInfo info, ComPtr<ID3D11Buffer> buffer )
{
    VertexBufferID id;
    ::Add( id, std::move( buffer ), m_vertex_buffers, m_vertex_generations, m_free_list );
    // slightly scampy?
    if( id.index >= m_vertex_buffer_infos.size( ) )
    {
        m_vertex_buffer_infos.emplace_back( info );
    }
    else
    {
        m_vertex_buffer_infos[id.index] = info;
    }
    return id;

}

VertexBufferID VertexBufferContainer::Add( VertexBufferInfo info, VertexBufferID other_buffer_id )
{
    assert( IsValid( other_buffer_id ) );
    return Add( info, m_vertex_buffers[other_buffer_id.index] );
}


void VertexBufferContainer::UpdateInfo( VertexBufferInfo info, VertexBufferID buffer_id )
{
    assert(IsValid(buffer_id));
    m_vertex_buffer_infos[buffer_id.index] = info;
}


void VertexBufferContainer::Remove( VertexBufferID buffer_id )
{
    if( IsValid( buffer_id ) )
    {
        // we have to reset the buffer for it to actually get removed on the device
        m_vertex_buffers[buffer_id.index].Reset();
        m_vertex_generations[buffer_id.index] += 1;
        m_free_list.push_back( buffer_id.index );
    }
}


void VertexBufferContainer::Remove( Range<VertexBufferID const *> vertex_buffer_ids )
{
    for(auto& buffer : vertex_buffer_ids)
    {
        Remove(buffer);
    }
}
