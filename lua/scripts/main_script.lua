require "entities"
require "keycodes"
require "scenarios"

function main()

    doggy = DogWorld.new()
    SetConfiguration(doggy)

    doggy:CreateAndShowWindow("LuaDog")

    -- local terrain_id = Generate3DTerrain();
    local terrain_id = Generate2DTerrain();

    local sun_id = doggy:SpawnLight(Entities.Sun, {color = {x=1, y=1, z=.3}}, {x= math.pi / 3, y= math.pi / 3, z=math.pi/3});

	local axis_template = doggy:RegisterEntityTemplate(Entities.AxesOrientation) --Used by the hit registration debug thingy stuff

    doggy:SetCameraPositionAndAngles({ x = 20, y = 20, z = 10 }, {x = 1.3, y = 0, z = 2.3})


    -- SpawnATower(5, Entities.RigidCube);
    -- SpawnATower(1, Entities.Sheep);
    -- MakePlayer(nil, {x=0,y=5,z=0})
    SpawnPhysicsDemo();
    -- SpawnATowerBlock(47, 8, Entities.RigidCube)
    -- TestParallelConstraintSolve();
    -- SpawnObstacleCourse(terrain_id);
    doggy:Run() -- =D

end


function SetConfiguration()
    -- set graphics configuration
    doggy:SetGraphicsConfiguration({
        render_sample_count = 4,
        maximum_texture_filtering_quality = 16,
        -- display_bounding_boxes = true,
        -- force_wire_frame = true,
        -- render_external_debug_components = true,
        });
    local arrow_keys_control = { up = KeyCode.Up, down = KeyCode.Down, left = KeyCode.Left, right = KeyCode.Right };
    local wasd_keys_control = { up = KeyCode.W, down = KeyCode.S, left = KeyCode.A, right = KeyCode.D };
    -- set camera controls
    doggy:SetCameraKeys(arrow_keys_control);
    -- set player controls
    doggy:SetPlayerKeys(wasd_keys_control);
    -- set physics configuration
    doggy:SetPhysicsConfiguration( {
        position_correction_iterations = 10,
        velocity_correction_iterations = 15,
        position_correction_fraction = 0.9,
        penetration_depth_tolerance = 1e-3,
        angular_velocity_correction_fraction = 0.7,
        fixed_fraction_velocity_loss_per_second = 0.1,
        warm_start_factor = 0.75,
        solve_parallel = true,
        minimal_island_size = 32,
        relaxation_factor = {max = 1.5, min = 1},
        -- relaxation_factor = 1,
        persitent_contact_expiry_age = 5,
        } );

    local gravity = {x = 0, y = 0, z = -9.81};
    doggy:SetGravity(gravity);
    doggy:SetTimeStep(1/60);
end


function SetGrassParameters()

	local grass_types = { GraphicsComponents.GrassSubtle, GraphicsComponents.GrassUpright}
	local grass_counts = {{1.0, 0.0, 0.0, 0.0, 0.0}, {1.0, 1.0, 0.0, 0.0, 0.0}}

	if DEBUG then
		grass_counts = {{0.1, 0.0, 0.0, 0.0, 0.0},{0.1, 0.0, 0.0, 0.0, 0.0}}
    end

    doggy:SetTerrainGrassTypes(grass_types, grass_counts)
end


function Get3DNorm(value)
	local squared_value = value * value;
	local norm = math.sqrt(3 * squared_value);

	return math.ceil(norm);
end


function Get2DTerrainLODDistance(patch_size, lod_level)
    return math.sqrt(patch_size.x * patch_size.x + patch_size.y * patch_size.y) * 4 * math.pow(2, lod_level)
end


function Generate3DTerrain()

    local noise_parameters ={
        { offset = {x=0.0, y= 0.0, z=0.0}, frequency = {x=0.001, y=0.001, z=0.001}, amplitude = -10. },
        { offset = {x=233.0, y= 233.0, z=233.0}, frequency = {x=0.003, y=0.003, z=0.003}, amplitude = 5. },
        { offset = {x=-32.0, y= -13.0, z=-12.0}, frequency = {x=0.015, y=0.015, z=0.015}, amplitude = 1.5 },
    }
    local terrain_entity_description = Entities.GeneratedTerrain3D

    local terrain_center = {x = 0.0, y = 0.0, z = 0.0}    --original position of central terrain block
    local update_distance = 25.0                --minimal distance of player to terrain center to trigger an update

    local terrain_block_count = {x = 81, y = 81, z = 3}    --number of terrain blocks visible at a time
    -- local terrain_block_count = {x = 51, y = 51, z = 3}

    if DEBUG then
        terrain_block_count  = {x = 5, y = 5, z = 3}
    end

    local terrain_block_dimensions = {x = 20.0, y = 20.0, z = 40.0}    --edge lengths per terrain block
    local terrain_block_cube_count = {x = 10, y = 10, z = 10}            --number of cubes per terrain block

    -- local terrain_block_dimensions = {x = 80.0, y = 80.0, z = 160.0}
    -- local terrain_block_cube_count = {x = 40, y = 40, z = 40}

    --local degradation_thresholds = {70, 104 * 1.5, 139 * 2, 174 * 3 }    --distance thresholds for LOD levels, based on spherical metric
    -- local degradation_thresholds =
    --     {
    --         --Get3DNorm(40),
    --         Get3DNorm(60),
    --         Get3DNorm(80),
    --         Get3DNorm(100),
    --         Get3DNorm(120)
    --     }
    local degradation_thresholds =
        {
            Get2DTerrainLODDistance({x=5, y=5}, 0),
            Get2DTerrainLODDistance({x=5, y=5}, 1),
            Get2DTerrainLODDistance({x=5, y=5}, 2),
            Get2DTerrainLODDistance({x=5, y=5}, 3)
        }

    SetGrassParameters();

    local terrain_id = doggy:Create3DTerrain(terrain_entity_description, terrain_center, update_distance, terrain_block_count, terrain_block_dimensions, terrain_block_cube_count, degradation_thresholds, noise_parameters )


    return terrain_id
end


function Generate2DTerrain()

    local noise_parameters ={
        --{ offset = {x=0.0, y= 0.0}, frequency = {x=0.001, y=0.001}, amplitude = 0 },

        --{ offset = {x=0.0, y= 0.0}, frequency = {x=0.001, y=0.001}, amplitude = -10. },
        --{ offset = {x=233.0, y= 233.0}, frequency = {x=0.003, y=0.003}, amplitude = 5. },
        --{ offset = {x=-32.0, y= -13.0}, frequency = {x=0.015, y=0.015}, amplitude = 3 },

        --{ offset = {x=142.0, y= 435.0}, frequency = {x=0.05, y=0.15}, amplitude = 1 },
        --{ offset = {x=4e3, y= 4e5}, frequency = {x=0.1, y=0.1}, amplitude = 0.2 },
    }
    local terrain_entity_description = Entities.GeneratedTerrain2D

    local terrain_center = {x = 0.0, y = 0.0, z = 0.0}    --original position of central terrain block

    local patch_dimensions = {x = 15, y = 15 } --number of vertices in each patch for every dimension (should be odd and at least 3)
    local patch_size = {x = 5, y = 5} --edge lengths for each patch
    local number_of_lod_levels = 5

    local terrain_id = doggy:Create2DTerrain(terrain_entity_description, terrain_center, patch_dimensions, patch_size, number_of_lod_levels, noise_parameters )

    SetGrassParameters();

    return terrain_id
end

