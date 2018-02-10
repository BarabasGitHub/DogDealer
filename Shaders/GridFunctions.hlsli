
// i = x + count.x * y
uint2 Calculate2DindexFrom1D( uint i, uint count_x )
{
    uint x = i % count_x;
    uint y = i / count_x;
    return uint2(x, y);
}