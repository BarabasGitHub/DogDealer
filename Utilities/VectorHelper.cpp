#include "VectorHelper.h"

void RemoveOffsets( std::vector<uint32_t>& offsets, Range<uint32_t const *> indices )
{
    if( Size( indices ) == 0 ) return;
    auto offset_destination = begin( offsets ) + First(indices) + 1;
    auto offset_source = offset_destination + 1;
    uint32_t adjust = offsets[First(indices) +1] - offsets[First(indices)];
    PopFirst(indices);

    auto move_offsets_till = [&offset_destination, &offset_source]( std::vector<uint32_t>::const_iterator last, uint32_t adjust )
    {
        while( offset_source < last )
        {
            *offset_destination = *offset_source - adjust;
            ++offset_destination;
            ++offset_source;
        }
    };

    for( auto i : indices )
    {
        auto data_offset = offsets[i];
        move_offsets_till( begin( offsets ) + i + 1, adjust );
        auto data_end = offsets[i + 1];
        adjust += data_end - data_offset;
        ++offset_source;
    }
    move_offsets_till( end( offsets ), adjust );

    offsets.erase( offset_destination, end( offsets ) );
}



void InvertReordering(Range<uint32_t const * __restrict> original, Range<uint32_t * __restrict> inverted)
{
    assert(Size(original) == Size(inverted));
    auto size = Size(original);
    for( auto i = 0u; i < size; ++i )
    {
        inverted[original[i]] = i;
    }
}
