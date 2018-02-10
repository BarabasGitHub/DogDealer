#pragma once

#include <Utilities\Handle.h>

template <typename ObjectType>
struct HandleIDPair
{
    Handle<ObjectType> id1, id2;

    HandleIDPair() = default;
    HandleIDPair(Handle<ObjectType> id1, Handle<ObjectType> id2) : id1(id1), id2(id2){}
};


template<typename ObjectType>
inline bool operator==(HandleIDPair<ObjectType> pair1, HandleIDPair<ObjectType> pair2)
{
    return (pair1.id1 == pair2.id1) & (pair1.id2 == pair2.id2);
}


template<typename ObjectType>
inline bool operator<(HandleIDPair<ObjectType> pair1, HandleIDPair<ObjectType> pair2)
{
    if( pair1.id1 == pair2.id1 )
    {
        return pair1.id2 < pair2.id2;
    }
    else
    {
        return pair1.id1 < pair2.id1;
    }
}
