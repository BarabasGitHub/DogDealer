#pragma once
#include "EntityID.h"

#include <Math\FloatTypes.h>
#include <Math\Identity.h>

#include <vector>

// An object with an orientation has its centre at 'position' and rotated around its centre according to 'rotation'
struct Orientation
{
	Math::Float3 position;
	Math::Quaternion rotation;
    Orientation() = default;
    Orientation( Math::Float3 position, Math::Quaternion rotation ) :
        position( position ),
        rotation( rotation )
    {}
    Orientation( Math::Identity i ) :
        Orientation( 0, i )
    {}


};

struct IndexedOrientations{
	std::vector<Orientation> orientations;
    std::vector<Orientation> previous_orientations;
	std::vector<unsigned> indices;
};


struct EntityOrientations
{
    std::vector<Orientation> orientations;
    std::vector<EntityID> entity_ids;
};


// SCAMP
// this should obviously go somewhere else

struct EntityPositions
{
    std::vector<Math::Float3> positions;
    std::vector<EntityID> entity_ids;
};

struct EntityRotations
{
    std::vector<Math::Quaternion> rotations;
    std::vector<EntityID> entity_ids;
};