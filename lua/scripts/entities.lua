require "graphics_components"
require "physics_components"
require "logic_components"

Entities = {
    Slab = {
        graphics = GraphicsComponents.Slab;
        physics = PhysicsComponents.Slab;
        name = "Slab";
    },
    FakeSlab = {
        graphics = GraphicsComponents.Slab;
        physics = PhysicsComponents.FakeSlab;
        name = "FakeSlab";
    },
    AxesOrientation = {
        graphics = GraphicsComponents.AxesOrientation;
        physics = PhysicsComponents.NonCollidingKinematicBody;
        name = "AxesOrientation";
    },
    Sun = {
        physics = PhysicsComponents.NonCollidingKinematicBody;
        name = "Sun";
    },
    Terrain = {
        graphics = GraphicsComponents.Terrain;
        physics = PhysicsComponents.Terrain;
        name = "Terrain";
    },
    GeneratedTerrain3D = {
        graphics = GraphicsComponents.GeneratedTerrain3D;
        physics = PhysicsComponents.Terrain;
        name = "GeneratedTerrain";
    },
    GeneratedTerrain2D = {
        graphics = GraphicsComponents.GeneratedTerrain2D;
        physics = PhysicsComponents.Terrain;
        name = "GeneratedTerrain";
    },
    Tomato = {
        graphics = GraphicsComponents.Tomato;
        physics = PhysicsComponents.Tomato;
        logic = {damage_value = 1000};
        name = "Tomato";
    },
    GrassQuad = {
        graphics = GraphicsComponents.GrassQuad;
        physics = PhysicsComponents.GrassQuad;
        name = "GrassQuad";
    },
    Tree = {
        graphics = {GraphicsComponents.Tree, GraphicsComponents.Leaves};
        physics = PhysicsComponents.Tree;
        name = "Tree";
    },
    TomatoTree = {
        graphics = {GraphicsComponents.Tree, GraphicsComponents.Leaves, GraphicsComponents.TreeTomatoes};
        physics = PhysicsComponents.Tree;
        name = "TomatoTree";
    },
    RigidCube = {
        graphics = {GraphicsComponents.Cube};
        physics = PhysicsComponents.RigidCube;
        name = "RigidCube";
    },

    StaticCube = {
        graphics = {GraphicsComponents.Cube};
        physics = PhysicsComponents.StaticCube;
        name = "StaticCube";
    },

    RigidSphere = {
        graphics = {GraphicsComponents.Sphere};
        physics = PhysicsComponents.RigidSphere;
        name = "RigidSphere";
    },

    StaticRamp = {
        graphics = {GraphicsComponents.Ramp};
        physics = PhysicsComponents.StaticRamp;
        name = "StaticRamp";
    },

	StaticObstacleArc = {
        graphics = {GraphicsComponents.ObstacleArc};
        physics = PhysicsComponents.StaticObstacleArc;
        name = "StaticObstacleArc";
    },

	StaticObstacleGaps = {
        graphics = {GraphicsComponents.ObstacleGaps};
        physics = PhysicsComponents.StaticObstacleGaps;
        name = "StaticObstacleGaps";
    },

	StaticObstacleSlope = {
        graphics = {GraphicsComponents.ObstacleSlope};
        physics = PhysicsComponents.StaticObstacleSlope;
        name = "StaticObstacleSlope";
    },

	StaticObstacleStairs = {
        graphics = {GraphicsComponents.ObstacleStairs};
        physics = PhysicsComponents.StaticObstacleStairs;
        name = "ObstacleStairs";
    },

	StaticWall = {
        graphics = {GraphicsComponents.Wall};
        physics = PhysicsComponents.StaticWall;
        name = "Wall";
    },

	StaticWallCorner = {
        graphics = {GraphicsComponents.WallCorner};
        physics = PhysicsComponents.StaticWallCorner;
        name = "WallCorner";
    },

	StaticWallStairs = {
        graphics = {GraphicsComponents.WallStairs};
        physics = PhysicsComponents.StaticWallStairs;
        name = "WallStairs";
    },

	StaticWallGate = {
        graphics = {GraphicsComponents.WallGate};
        physics = PhysicsComponents.StaticWallGate;
        name = "WallGate";
    },

	StaticWallTower = {
        graphics = {GraphicsComponents.WallTower};
        physics = PhysicsComponents.StaticWallTower;
        name = "WallTower";
    },

    StaticMultitower = {
        graphics = {GraphicsComponents.Multitower};
        physics = PhysicsComponents.StaticMultitower;
        name = "Multitower";
    },

	NavigationMesh = {
        graphics = {GraphicsComponents.NavigationMesh};
        physics = PhysicsComponents.NonCollidingKinematicBody;
        name = "NavigationMesh";
    },

    StaticHedge = {
        graphics = {GraphicsComponents.Hedge};
        physics = PhysicsComponents.StaticHedge;
        name = "StaticHedge";
    },

    Sheep =
    {
        graphics = GraphicsComponents.Sheep;
        physics = PhysicsComponents.Sheep;
        name = "Sheep";
    }
}
