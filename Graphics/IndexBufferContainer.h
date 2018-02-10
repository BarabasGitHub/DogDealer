#pragma once

#include "DirectX\Direct3D11.h"
#include "IndexBufferType.h"
#include "IDs.h"

#include <deque>
#include <vector>
#include <cstdint>

#include <Utilities\ComPtr.h>
#include <Utilities\Handle.h>


namespace Graphics
{

    struct IndexBufferInfo
    {
        IndexBufferType type;
        D3D_PRIMITIVE_TOPOLOGY topology;
        unsigned index_count;
        unsigned offset;
    };

    struct IndexBufferContainer
    {

    private:
        std::vector<ComPtr<ID3D11Buffer>> m_index_buffers;
        std::vector<IndexBufferInfo> m_index_buffer_infos;
        std::vector<IndexBufferID::generation_t> m_index_generations;
        std::deque<unsigned> m_free_list;

    public:

        bool IsValid( IndexBufferID id ) const;

        IndexBufferInfo GetIndexBufferInfo( IndexBufferID buffer_id ) const;

        ID3D11Buffer* GetIndexBuffer( IndexBufferID buffer_id ) const;

        IndexBufferID AddNewIndexBuffer( IndexBufferInfo info, ComPtr<ID3D11Buffer> buffer );

        void RemoveIndexBuffer( IndexBufferID index_buffer_id );
    };


    inline bool IndexBufferContainer::IsValid( IndexBufferID id ) const
    {
        assert( m_index_buffer_infos.size( ) == m_index_generations.size( ) );
        assert( m_index_buffer_infos.size( ) == m_index_buffers.size( ) );
        return id.index < m_index_generations.size( ) &&
            id.generation == m_index_generations[id.index];
    }

    inline IndexBufferInfo IndexBufferContainer::GetIndexBufferInfo( IndexBufferID buffer_id ) const
    {
        assert( IsValid( buffer_id ) );
        return m_index_buffer_infos[buffer_id.index];
    }

    inline ID3D11Buffer* IndexBufferContainer::GetIndexBuffer( IndexBufferID buffer_id ) const
    {
        assert( IsValid( buffer_id ) );
        return m_index_buffers[buffer_id.index];
    }
}