function MakePlayer(ammo, position, rotation)
    if rotation == nil then
        rotation = {x=0,y=0,z=0}
    end
    local player = {}

    --player.id = doggy:SpawnEntity(Entities.RigidHumanoid, {x = 3, y = 0, z = 2}, {x=0, z = math.pi, y=0})
    player.alive = true
	player.id = doggy:SpawnEntity(Entities.RigidHumanoid, position, rotation)

    doggy:SetHitpoints(player.id, 80);

    if ammo then
	   doggy:SetAmmunition(player.id, {bone = 7, launch_speed = 10, upward_launch_offset = 0.5, position = { x=0, y=0.2, z=.2}, rotation = { x=-math.pi/2, y=0, z=0} }, ammo);
       doggy:CreatePlayerThrowController(player.id);
    end

    doggy:CreatePlayerStrikeController(player.id);

	-- doggy:AddComponent(player.id, Entities.PotHelmet);

	doggy:CreatePlayerController(player.id)
    doggy:FollowCameraRotation(player.id)
    doggy:SetCameraTarget(player.id)

	doggy:SetAfterDeathComponents(player.id, Entities.RigidHumanoidKilled);

	return player
end

function SpawnALot(entity_description)
    local area_size = 100
    local max_speed = 4
    local count = 1000
    for i=1,count do
        doggy:SpawnEntity(entity_description,
            {x = math.random() * area_size - area_size / 2, y = math.random() * area_size - area_size / 2, z = math.random() * area_size },
            {x = math.random() * math.pi, y= math.random() * math.pi, z = math.random() * math.pi * 2},
            {x = (math.random() * 2 - 1) * max_speed , y = (math.random() * 2 - 1) * max_speed, z = (math.random() * 2 - 1) * max_speed } )
    end
end


function SpawnAFew()
    local position = {x = 0.0, y = 0.0, z = 1.0}
    local rotation = {x = 0.0, y = 0.0, z = 0.0}
    doggy:SpawnEntity(Entities.FreeRefrigeratorWarrior, position, rotation)
    -- doggy:SpawnEntity(Entities.FreeRefrigeratorWarrior, MergeTables(position, {x = 3}), rotation, {x = -5, y = 0, z= 0})
    position.z = position.z + 2;
    doggy:SpawnEntity(Entities.FreeRefrigeratorWarrior, position, rotation)
    position.z = position.z + 2;
    -- doggy:SpawnEntity(Entities.FreeRefrigeratorWarrior, position, rotation)
end


function SpawnATower(number, entity_description, start_position)
    local position = {x = 0.0, y = 0.0, z = 1}
    if start_position ~= nil then
        position.x = start_position.x
        position.y = start_position.y
        position.z = start_position.z
    end
    local rotation = {x = 0.0, y = 0.0, z = 0.0}
    for i=1,number do
        -- if i < number/2 then
            -- doggy:SpawnEntity(Entities.StaticCube, position, rotation)
        -- else
            doggy:SpawnEntity(entity_description, position, rotation)
        -- end
        position.z = position.z + 2
    end
end

function SpawnATowerLine(height, length, entity_description, start_position)
    local position = {x = -length, y = 0.0, z = 1}
    if start_position ~= nil then
        position.x = start_position.x
        position.y = start_position.y
        position.z = start_position.z
    end
    local rotation = {x = 0.0, y = 0.0, z = 0.0}
    for i=1,length do
        SpawnATower(height, entity_description, position)
        position.y = position.y + 2
    end
end

function SpawnATowerBlock(height, block_size, entity_description, start_position)
    local position = {x = -block_size, y = -block_size, z = 1}
    if start_position ~= nil then
        position.x = start_position.x
        position.y = start_position.y
        position.z = start_position.z
    end
    local rotation = {x = 0.0, y = 0.0, z = 0.0}
    for i=1,block_size do
        SpawnATowerLine(height, block_size, entity_description, position)
        position.x = position.x + 2
    end
end


function SpawnDiagonalPyramid()
    local rotation = {x=0, y=0, z=0}
    local entity_description = Entities.StaticCube
    -- floor
    doggy:SpawnEntity(entity_description, {x=-2, y=-2, z=1}, rotation)
    doggy:SpawnEntity(entity_description, {x=-2, y=2, z=1}, rotation)
    doggy:SpawnEntity(entity_description, {x=-4, y=0, z=1}, rotation)
    doggy:SpawnEntity(entity_description, {x=0, y=-4, z=1}, rotation)
    doggy:SpawnEntity(entity_description, {x=0, y=4, z=1}, rotation)
    doggy:SpawnEntity(entity_description, {x=2, y=-2, z=1}, rotation)
    doggy:SpawnEntity(entity_description, {x=2, y=2, z=1}, rotation)
    doggy:SpawnEntity(entity_description, {x=4, y=0, z=1}, rotation)
    -- level 1
    doggy:SpawnEntity(entity_description, {x=0, y=-2, z=3}, rotation)
    doggy:SpawnEntity(entity_description, {x=-2, y=0, z=3}, rotation)
    doggy:SpawnEntity(entity_description, {x=2, y=0, z=3}, rotation)
    doggy:SpawnEntity(entity_description, {x=0, y=2, z=3}, rotation)
    -- level 2
    doggy:SpawnEntity(entity_description, {x=0, y=0, z=5}, rotation)
end

function SpawnObstacleCourse(terrain_id)

	-- PLAYER
	local tomato_template = doggy:RegisterEntityTemplate(Entities.Tomato);
	player = MakePlayer(tomato_template, {x = 10.0, y = 0.0, z = 0.0}, {x=0, z = 0, y=0});

	doggy:SpawnEntity(Entities.StaticObstacleArc, {x = 0, y = 0, z = 0}, {x=0, z=0, y=0})

	doggy:SpawnEntity(Entities.StaticObstacleStairs, {x = -10, y = 0, z = 0}, {x=0, z=0, y=0})

	doggy:SpawnEntity(Entities.StaticObstacleSlope, {x = -10, y = 10, z = 0}, {x=0, z=0, y=0})

	doggy:SpawnEntity(Entities.StaticObstacleGaps, {x = 10, y = 10, z = 0}, {x=0, z=0, y=0})
end

function SpawnOntoTerrain( entity, position, rotation, terrain_id)

    local position_3d = doggy:FindRestingPositionOnEntity( {x = position.x, y = position.y, z = 0}, terrain_id );
	doggy:SpawnEntity(entity, position_3d, rotation);

end

function SpawnHedgeLine( line_start, line_end, rotation, terrain_id )

	--Get length to to-be-spawned line
	local offset_x = line_end.x - line_start.x;
	local offset_y = line_end.y - line_start.y;
	local length = math.sqrt(math.abs(offset_x * offset_x + offset_y * offset_y));

	-- Get required count of hedges to be placed
	local hedge_length = 3.0;
	local hedge_count = math.floor(length / hedge_length) - 1;

	-- Get distance between hedge instances
	local step_x = offset_x / hedge_count;
	local step_y = offset_y / hedge_count;

	-- Scale distance to hedge-length
	local step_factor = hedge_length / math.sqrt(math.abs(step_x * step_x + step_y * step_y));
	step_x = step_x * step_factor;
	step_y = step_y * step_factor;

	--Spawn hedges with proper offset from one another
	for i=0,hedge_count do

		--Shift to current hedge position
		local position_x = line_start.x + (i + 0.5) * step_x;
		local position_y = line_start.y + (i + 0.5) * step_y;

       	local position_3d = doggy:FindRestingPositionOnEntity( {x = position_x, y = position_y, z = 0}, terrain_id );
		doggy:SpawnEntity(Entities.StaticHedge, position_3d, rotation);
    end
end


function SpawnHedges( terrain_id )
    local area_size = 20
    local count = 10
    for i=1,count do
        local position = doggy:FindRestingPositionOnEntity( {x = math.random() * area_size - area_size / 2, y = math.random() * area_size - area_size / 2, z = 0 }, terrain_id);
        doggy:SpawnEntity(Entities.StaticHedge, position, {x=0, y=0, z=0});
    end
end


function SpawnPhysicsDemo()
    -- SpawnAFew()
    -- SpawnALot(Entities.RigidCube)
    -- SpawnALot(Entities.RigidSphere)
    -- SpawnATower(2);
    -- SpawnATower(5, {x=0, y=0, z= 100});

    -- castle wall
    SpawnATowerLine(5, 30, Entities.RigidCube, {x=0, y=-30, z=1});
    SpawnATowerBlock(7, 3, Entities.RigidCube, {x=-3, y=-36, z=1});
    SpawnATowerBlock(7, 3, Entities.RigidCube, {x=-3, y=30, z=1});

    -- SpawnATowerBlock(5, 5, Entities.RigidCube, {x=-5, y=-5, z=1});
    -- SpawnATowerBlock(1, 7, Entities.RigidSphere, {x=-7, y=-7, z = 30});
    -- SpawnATowerBlock(1, 7, Entities.Sheep, {x=-7, y=-7, z = 30});
    -- SpawnATowerBlock(1, 7, Entities.RigidSphere);
    -- SpawnATowerBlock(1, 5, Entities.RigidSphere, {x=-5, y=-5, z=3});
    -- SpawnATowerBlock(1, 3, Entities.RigidSphere, {x=-3, y=-3, z=5});
    -- SpawnATowerBlock(1, 1, Entities.RigidSphere, {x=-1, y=-1, z=7});
    doggy:SpawnEntity(Entities.Sheep, {x = 25, y = -4.5, z = 4}, {x = 0, y = 0, z = math.pi}, {x = -120 , y = 0, z = 2});
    doggy:SpawnEntity(Entities.Sheep, {x = 20, y = 0, z = 4}, {x = 0, y = 0, z = math.pi}, {x = -200 , y = 0, z = 2});
    doggy:SpawnEntity(Entities.Sheep, {x = 60, y = 4.5, z = 4}, {x = 0, y = 0, z = math.pi}, {x = -2000, y = 0, z = 2});
    -- doggy:SpawnEntity(Entities.RigidSphere, {x = 25, y = -3, z = 7}, {x = 0, y = 0, z = 0}, {x = -120, y = 0, z = 2});
    -- doggy:SpawnEntity(Entities.RigidSphere, {x = 20, y = 0, z = 7}, {x = 0, y = 0, z = 0}, {x = -200, y = 0, z = 2});
    -- doggy:SpawnEntity(Entities.RigidSphere, {x = 60, y = 3, z = 7}, {x = 0, y = 0, z = 0}, {x = -500, y = 0, z = 2});
end


function TestParallelConstraintSolve()
    -- SpawnATowerBlock(3, 3, Entities.RigidCube, {x = -5, y = 0, z = 2});
    -- SpawnATowerBlock(3, 3, Entities.RigidCube, {x = 0, y = -5, z = 2});

    SpawnATowerBlock(5, 5, Entities.RigidCube, {x = -20, y = -20, z = 2});
    SpawnATowerBlock(5, 5, Entities.RigidCube, {x = -20 , y = -5, z = 2});
    SpawnATowerBlock(5, 5, Entities.RigidCube, {x =  -5, y = -20, z = 2});
    SpawnATowerBlock(5, 5, Entities.RigidCube, {x =  -5, y = -5, z = 2});
    SpawnATowerBlock(5, 5, Entities.RigidCube, {x =  10, y = -20, z = 2});
    SpawnATowerBlock(5, 5, Entities.RigidCube, {x = -20, y = 10, z = 2});
    SpawnATowerBlock(5, 5, Entities.RigidCube, {x =  10, y = -5, z = 2});
    SpawnATowerBlock(5, 5, Entities.RigidCube, {x =  -5, y = 10, z = 2});
end
