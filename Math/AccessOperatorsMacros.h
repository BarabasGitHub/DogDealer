#ifndef AccessOperators
#define AccessOperators(Type, element_count )                  \
inline decltype(auto) Type::operator[]( unsigned index )       \
{                                                              \
    assert( index < element_count );                           \
    return (&x)[index];                                        \
}                                                              \
constexpr inline decltype(auto) Type::operator[]( unsigned index ) const \
{                                                              \
    return (&x)[index];                                        \
}
#endif