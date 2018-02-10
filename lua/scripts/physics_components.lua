require "table_functions"

PhysicsComponents = {

    StaticBody = {
        static = true;
        collision_file = "some_file"
        -- optional, for collision with rigid bodies:
        -- bounciness = 0 - 1; default = 1
        -- friction = 0 - 1; default = 0.5
    },

    KinematicBody = {
        collision_file = "some_file"
        -- no mass or mass == 0
        -- optional, for collision with rigid bodies:
        -- bounciness = 0 - 1; default = 1
        -- friction = 0 - 1; default = 0.5
    },

    RigidBody = {
        collision_file = "some_file";
        mass = 10 -- mass > 0
        -- optional:
        -- bounciness = 0 - 1; default = 1
        -- friction = 0 - 1; default = 0.5
    },

    MultiBody = {
        bodies = { body1, body2 } -- must all be the same type
    },

    MultiRigidBody = {
        bodies = { body1, body2 }; -- must all be the same type
        -- optional
        connection = { connection1, connection2 };  -- connections between the bodies
    },

    NonCollidingStaticBody =
    {
        static = true;
        -- no collision_file or collision_file = ""
    },

    NonCollidingKinematicBody =
    {
        -- no collision_file or collision_file = ""
    },
}

PhysicsComponents.Slab = {
        static = true;
        collision_file = "slab";
        bounciness = 0.5;
    }

PhysicsComponents.Terrain = {
        static = true;
        collision_file = "terrain";
        bounciness = 0.5;
        friction = 1;
    }

PhysicsComponents.Tomato = {
        collision_file = "collision_tomato";
        mass = 0.12;
        bounciness = 0.3;
        friction = 0.2;
    }

PhysicsComponents.GrassQuad = {
        static = true;
    }

PhysicsComponents.Tree = {
        static = true;
        collision_file = "tree";
    }

PhysicsComponents.StaticCube = {
        static = true;
        collision_file = "collision_cube";
        -- collision_file = "boxes";
        bounciness = 0.5;
    }

PhysicsComponents.RigidCube = {
        collision_file = "collision_cube";
        -- collision_file = "boxes";
        mass = 30;
        bounciness = 0.5;
        friction = 0.5;
    }

PhysicsComponents.RigidSphere = {
        collision_file = "collision_sphere";
        mass = 30;
        bounciness = 0.9;
        friction = 0.5;
    }

PhysicsComponents.StaticRamp = {
        static = true;
        collision_file = "ramp";
    }
PhysicsComponents.StaticObstacleArc = {
        static = true;
        collision_file = "obstacle_arc_collision";
    }
PhysicsComponents.StaticObstacleGaps = {
        static = true;
        collision_file = "obstacle_gaps_collision";
    }
PhysicsComponents.StaticObstacleSlope = {
        static = true;
        collision_file = "obstacle_slope_collision";
    }
PhysicsComponents.StaticObstacleStairs = {
        static = true;
        collision_file = "obstacle_stairs_collision";
    }
PhysicsComponents.StaticWall = {
        static = true;
        collision_file = "wall_collision";
    }
PhysicsComponents.StaticWallCorner = {
        static = true;
        collision_file = "wall_corner_collision";
    }
PhysicsComponents.StaticWallStairs = {
        static = true;
        collision_file = "wall_stairs_collision";
    }
PhysicsComponents.StaticWallGate = {
        static = true;
        collision_file = "wall_gate_collision";
    }
PhysicsComponents.StaticWallTower = {
        static = true;
        collision_file = "wall_tower_collision";
    }
PhysicsComponents.StaticMultitower = {
        static = true;
        collision_file = "multitower_collision";
    }
PhysicsComponents.StaticHedge= {
        static = true;
        collision_file = "hedge";
    }

PhysicsComponents.Sheep = {
        collision_file = "collision_sheep";
        mass = 60;
        bounciness = 0.3;
    }


function GetAllCollisionNames()
    collision_files = {}
    for k,v in pairs(PhysicsComponents) do
        if v.collision_file ~= nil then
            collision_files[#collision_files + 1] = v.collision_file
        elseif v.bodies ~= nil then
            for _,b in ipairs(v.bodies) do
                collision_files[#collision_files + 1] = b.collision_file
            end
        end
    end
    return collision_files
end
