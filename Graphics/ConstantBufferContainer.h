#pragma once
#include <vector>
#include <deque>
#include <cassert>

#include "IDs.h"

#include "DirectX\Direct3D11.h"

#include <Utilities\Handle.h>
#include <Utilities\Range.h>

namespace Graphics
{
    struct Device;


    struct ConstantBufferContainer
    {
    private:
        std::vector<ComPtr<ID3D11Buffer>> m_buffers;
        std::vector<uint32_t> m_sizes;
        std::vector<uint32_t> m_offsets; // only for dx11.1
        std::vector<uint8_t> m_generation;
        std::deque<uint32_t> m_free_list;

    public:

        ConstantBufferID Add( ComPtr<ID3D11Buffer> constant_buffer );
        ConstantBufferID Add( ComPtr<ID3D11Buffer> constant_buffer, uint32_t offset );

        bool IsValid(ConstantBufferID id) const;

        ID3D11Buffer * GetBuffer(ConstantBufferID id) const;
        uint32_t GetSize(ConstantBufferID id) const;
        uint32_t GetOffset(ConstantBufferID id) const;

        void Remove(ConstantBufferID id);
        void Remove(Range<ConstantBufferID const*> ids);

    private:

        ConstantBufferID InternalAdd( ComPtr<ID3D11Buffer> constant_buffer, uint32_t byte_size, uint32_t offset );
    };


}