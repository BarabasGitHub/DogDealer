#pragma once
#include <Conventions\EntityID.h> // for EntityID

#include <BoundingShapes\AxisAlignedBox.h>
#include <string>
#include <vector>

namespace Physics{
	
    enum struct BodyType { Static, Kinematic, Rigid };

    enum struct ConnectionType { Invalid, KeepPositions, KeepZRotation };
    
    inline ConnectionType ConnectionTypeFromString(std::string const & string)
    {
        if(string == "KeepPositions") return ConnectionType::KeepPositions;
        if(string == "KeepZRotation") return ConnectionType::KeepZRotation;
        return ConnectionType::Invalid;
    }

    struct Connection
    {
        ConnectionType type;
        // indices of the parent and child bodies
        uint32_t parent, child;
    };

	struct ComponentDescription
	{
        struct Body
        {
            float mass;
            float bounciness;
            float friction_factor;
            bool lock_rotation;
            std::string collision_file;
        };
        BodyType body_type;
        std::vector<Body> bodies;
        // actually only relevant for rigid bodies
        std::vector<Connection> connections;
	};
}