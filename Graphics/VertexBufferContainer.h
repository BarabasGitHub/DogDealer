#pragma once
#include "DirectX\Direct3D11.h"
#include "VertexBufferType.h"
#include "IDs.h"

#include <Utilities\ComPtr.h>
#include <Utilities\Range.h>

#include <deque>
#include <vector>
#include <cassert>

namespace Graphics
{

    struct VertexBufferInfo
    {
        VertexBufferType type;
        unsigned vertex_count;
        unsigned offset;
    };

    struct VertexBufferContainer
    {

        std::vector<ComPtr<ID3D11Buffer>> m_vertex_buffers;
        std::vector<VertexBufferInfo> m_vertex_buffer_infos;
        std::vector<uint8_t> m_vertex_generations;
        std::deque<unsigned> m_free_list;

    public:

        bool IsValid( VertexBufferID id ) const;

        VertexBufferInfo GetBufferInfo( VertexBufferID buffer_id ) const;

        ID3D11Buffer* GetBuffer( VertexBufferID buffer_id ) const;

        VertexBufferID Add( VertexBufferInfo info, ComPtr<ID3D11Buffer> buffer );

        VertexBufferID Add( VertexBufferInfo info, VertexBufferID other_buffer_id );

        void UpdateInfo( VertexBufferInfo info, VertexBufferID buffer_id );

        void Remove( VertexBufferID vertex_buffer_id );

        void Remove( Range<VertexBufferID const *> vertex_buffer_ids );
    };


    inline bool VertexBufferContainer::IsValid( VertexBufferID id ) const
    {
        assert( m_vertex_buffer_infos.size( ) == m_vertex_generations.size( ) );
        assert( m_vertex_buffer_infos.size( ) == m_vertex_buffers.size( ) );
        return id.index < m_vertex_generations.size() &&
            id.generation == m_vertex_generations[id.index];
    }

    inline VertexBufferInfo VertexBufferContainer::GetBufferInfo( VertexBufferID buffer_id ) const
    {
        assert( IsValid( buffer_id ) );
        return m_vertex_buffer_infos[buffer_id.index];
    }

    inline ID3D11Buffer* VertexBufferContainer::GetBuffer( VertexBufferID buffer_id ) const
    {
        assert( IsValid( buffer_id ) );
        return m_vertex_buffers[buffer_id.index];
    }

}
