#pragma once

#include <cstdint>


// enable if you want to see how much padding the handle struct has
//#pragma warning( once : 4820 )

// Handle for a arbitrary object.
template<typename ObjectType = void>
struct Handle
{
    typedef ObjectType object_t;
    typedef uint8_t generation_t;
    typedef uint32_t index_t;

    // index into the vector of objects in the Container class
    index_t index;
    // generation, keeps track whether the handle is outdated
    generation_t generation;

    Handle() = default;
    constexpr Handle( index_t index, generation_t generation ) : index( index ), generation( generation ) {}
    constexpr operator Handle<void>() const
    {
        return{ index, generation };
    }
};
                   
typedef Handle<void> UniformHandle;

template<typename Type> 
inline Type ToHandle( UniformHandle uniform_handle ) 
{
    Type handle;
    handle.generation = uniform_handle.generation;
    handle.index = uniform_handle.index;
    return handle;
}


template<typename ObjectType>
inline bool constexpr operator==( Handle<ObjectType> const & first, Handle<ObjectType> const & second )
{
    return first.index == second.index && first.generation == second.generation;
}

template<typename ObjectType>
inline bool constexpr operator!=( Handle<ObjectType> const & first, Handle<ObjectType> const & second )
{
    return !(first == second);
}


template<typename ObjectType>
inline bool constexpr operator<( Handle<ObjectType> const & first, Handle<ObjectType> const & second )
{
    // constexpr can only have one return statement, so I cramped it all in one line
    return first.index < second.index ? true : first.index > second.index ? false : first.generation < second.generation;
    //if(first.index < second.index)
    //{
    //    return true;
    //}
    //else if(first.index > second.index)
    //{
    //    return false;
    //}
    //else
    //{
    //    return first.generation < second.generation;
    //}
}
