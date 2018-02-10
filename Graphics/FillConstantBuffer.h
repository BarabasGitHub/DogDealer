#pragma once

#include "DirectX\Direct3D11.h"
#include "DeviceContext.h"

#include <functional>

namespace Graphics
{
    /// Allows you to fill up a constant buffer without worrying about mapping/unmapping. Just provide a function/lambda/functor in which the buffer gets filled.
    template<typename MappedDataStructType, typename CallbackType>
    void FillConstantBuffer( ID3D11Buffer * const buffer, ID3D11DeviceContext* const context, CallbackType const & callback, bool const discard = true )
    {
        D3D11_MAPPED_SUBRESOURCE d3d_mapped_subresource;
        ThrowIfFailed( context->Map( buffer, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &d3d_mapped_subresource ) );
        callback( static_cast<MappedDataStructType*>( d3d_mapped_subresource.pData ) );
        context->Unmap( buffer, 0 );
    }


    // same as above but you can provide the data directly
    template<typename MappedDataStructType>
    void FillConstantBufferWithData( ID3D11Buffer * const buffer, ID3D11DeviceContext* const context, MappedDataStructType const & data, bool const discard = true )
    {
        FillConstantBuffer<MappedDataStructType>(buffer, context, [&data](MappedDataStructType * pData)
        {
            *pData = data;
        }, discard);
    }
}