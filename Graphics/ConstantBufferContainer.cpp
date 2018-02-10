#include "ConstantBufferContainer.h"

#include <Utilities\ContainerHelpers.h>

using namespace Graphics;

ConstantBufferID ConstantBufferContainer::Add( ComPtr<ID3D11Buffer> constant_buffer )
{
    return Add(constant_buffer, 0);
}


ConstantBufferID ConstantBufferContainer::Add( ComPtr<ID3D11Buffer> constant_buffer, uint32_t offset )
{
    assert(offset == 0 && "Offsets are only possible in DX11.1");
    D3D11_BUFFER_DESC description;
    constant_buffer->GetDesc( &description );
    // we can use == because it may not be combined with any other flag
    assert(description.BindFlags == D3D11_BIND_CONSTANT_BUFFER);
    return InternalAdd( std::move(constant_buffer), description.ByteWidth - offset, offset );
}


bool Graphics::ConstantBufferContainer::IsValid(ConstantBufferID id) const
{
    assert(m_buffers.size() == m_generation.size());
    return id.index < m_buffers.size() && id.generation == m_generation[id.index];
}


ID3D11Buffer * Graphics::ConstantBufferContainer::GetBuffer(ConstantBufferID id) const
{
    assert(IsValid(id));
    return m_buffers[id.index];
}


uint32_t Graphics::ConstantBufferContainer::GetSize(ConstantBufferID id) const
{
    assert(IsValid(id));
    return m_sizes[id.index];
}


uint32_t Graphics::ConstantBufferContainer::GetOffset(ConstantBufferID id) const
{
    assert(IsValid(id));
    return m_offsets[id.index];
}


void Graphics::ConstantBufferContainer::Remove(ConstantBufferID id)
{
    if(IsValid(id))
    {
        m_buffers[id.index].Reset();
        ++m_generation[id.index];
        m_free_list.push_back(id.index);
    }
}


void Graphics::ConstantBufferContainer::Remove(Range<ConstantBufferID const *> ids)
{
    for(auto id : ids)
    {
        Remove(id);
    }
}


ConstantBufferID ConstantBufferContainer::InternalAdd( ComPtr<ID3D11Buffer> constant_buffer, uint32_t byte_size, uint32_t offset )
{
    ConstantBufferID id;
    ::Add( id, std::move(constant_buffer), m_buffers, m_generation, m_free_list );
    if( id.index >= m_sizes.size( ) )
    {
        m_sizes.emplace_back( byte_size );
        m_offsets.emplace_back( offset );
    }
    else
    {
        m_sizes[id.index] = byte_size;
        m_offsets[id.index] = offset;
    }
    return id;
}