#include "IndexBufferContainer.h"

#include <Utilities\ContainerHelpers.h>

using namespace Graphics;

IndexBufferID IndexBufferContainer::AddNewIndexBuffer( IndexBufferInfo info, ComPtr<ID3D11Buffer> buffer )
{
    IndexBufferID id;
    Add( id, std::move( buffer ), m_index_buffers, m_index_generations, m_free_list );
    // slightly scampy?
    if( id.index >= m_index_buffer_infos.size() )
    {
        m_index_buffer_infos.emplace_back( info );
    }
    else
    {
        m_index_buffer_infos[id.index] = info;
    }
    return id;
}


void IndexBufferContainer::RemoveIndexBuffer( IndexBufferID buffer_id )
{
    if( IsValid( buffer_id ) )
    {
        // we have to reset the buffer for it to actually get removed on the device
        m_index_buffers[buffer_id.index].Reset();
        m_index_generations[buffer_id.index] += 1;
        m_free_list.push_back( buffer_id.index );
    }
}