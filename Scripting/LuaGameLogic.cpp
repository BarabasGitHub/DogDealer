#pragma once

#include "LuaGameLogic.h"
#include "LuaMathTypes.h"
#include "LuaUtilitiesTypes.h"

#include <Math\MathFunctions.h>

using namespace Logic;



ThrowProperties luaU_Impl<ThrowProperties>::luaU_check( lua_State * L, int index )
{
    ThrowProperties p;
    p.bone = luaU_checkfield<uint32_t>( L, index, "bone" );
    p.launch_speed = luaU_checkfield<float>( L, index, "launch_speed" );
    p.upward_launch_offset = luaU_checkfield<float>( L, index, "upward_launch_offset" );
    auto position = luaU_checkfield<Math::Float3>( L, index, "position" );
    auto euler_rotation = luaU_checkfield<Math::Float3>( L, index, "rotation" );
    p.offset = { position, EulerToQuaternion( euler_rotation ) };
    return p;
}


ThrowProperties luaU_Impl<ThrowProperties>::luaU_to( lua_State * L, int index )
{
    ThrowProperties p;
    p.bone = luaU_getfield<uint32_t>( L, index, "bone" );
    p.launch_speed = luaU_getfield<float>( L, index, "launch_speed" );
    p.upward_launch_offset = luaU_getfield<float>( L, index, "upward_launch_offset" );
    auto position = luaU_getfield<Math::Float3>( L, index, "position" );
    auto euler_rotation = luaU_getfield<Math::Float3>( L, index, "rotation" );
    p.offset = { position, EulerToQuaternion( euler_rotation ) };
    return p;
}

char const luaU_Impl<AttachmentPoint>::c_bone_index[] = "bone_index";
char const luaU_Impl<AttachmentPoint>::c_position[] = "position";
char const luaU_Impl<AttachmentPoint>::c_rotation[] = "rotation";


AttachmentPoint luaU_Impl<AttachmentPoint>::luaU_check(lua_State* L, int index)
{
    AttachmentPoint attachment_point;
    // required
    attachment_point.bone_index = luaU_checkfield<unsigned>(L, index, c_bone_index);
    attachment_point.offset.position = luaU_checkfield<Math::Float3>(L, index, c_position);
    attachment_point.offset.rotation = Math::EulerToQuaternion(luaU_checkfield<Math::Float3>(L, index, c_rotation));

    return attachment_point;
}


AttachmentPoint luaU_Impl<AttachmentPoint>::luaU_to(lua_State* L, int index)
{
    AttachmentPoint attachment_point;
    // these are required
    attachment_point.bone_index = luaU_getfield<unsigned>(L, index, c_bone_index);
    attachment_point.offset.position = luaU_getfield<Math::Float3>(L, index, c_position);
    attachment_point.offset.rotation = Math::EulerToQuaternion(luaU_getfield<Math::Float3>(L, index, c_rotation));

    return attachment_point;
}


bool luaU_Impl<AttachmentPoint>::luaU_is(lua_State* L, int index)
{
    return
        lua_istable(L, index) &&
        luaU_fieldis<unsigned>(L, index, c_bone_index) &&
        luaU_fieldis<Math::Float3>(L, index, c_position) &&
        luaU_fieldis<Math::Float3>(L, index, c_rotation);
}

char const luaU_Impl<MobileComponentDescription>::c_motion_power[] = "motion_power";
char const luaU_Impl<MobileComponentDescription>::c_maximum_torque[] = "maximum_torque";
char const luaU_Impl<MobileComponentDescription>::c_motion_bias_right[] = "motion_bias_right";
char const luaU_Impl<MobileComponentDescription>::c_motion_bias_forwards[] = "motion_bias_forwards";
char const luaU_Impl<MobileComponentDescription>::c_motion_bias_left[] = "motion_bias_left";
char const luaU_Impl<MobileComponentDescription>::c_motion_bias_backwards[] = "motion_bias_backwards";
char const luaU_Impl<MobileComponentDescription>::c_target_speed[] = "target_speed";
char const luaU_Impl<MobileComponentDescription>::c_attachment_point[] = "attachment_point";

MobileComponentDescription luaU_Impl<MobileComponentDescription>::luaU_check( lua_State* L, int index )
{
    MobileComponentDescription description;
    // required
    description.motion_power = luaU_checkfield<float>( L, index, c_motion_power );
    description.rotation_power = luaU_checkfield<float>( L, index, c_maximum_torque );
    description.target_speed = luaU_checkfield<float>( L, index, c_target_speed );

    // optional
    description.motion_bias_positive.x = luaU_optfield<float>(L, index, c_motion_bias_right, 1);
    description.motion_bias_positive.y = luaU_optfield<float>(L, index, c_motion_bias_forwards, 1);
    description.motion_bias_negative.x = luaU_optfield<float>(L, index, c_motion_bias_left, 1);
    description.motion_bias_negative.y = luaU_optfield<float>(L, index, c_motion_bias_backwards, 1);

    lua_getfield(L, index, c_attachment_point);
    if (lua_istable(L, -1))
    {
        AttachmentPoint attachment_point = ::luaU_check<AttachmentPoint>(L, -1);
        description.weapon_attachment_points.push_back(attachment_point);
    }

    return description;
}


MobileComponentDescription luaU_Impl<MobileComponentDescription>::luaU_to( lua_State* L, int index )
{
    MobileComponentDescription description;
    // these are required
    description.motion_power = luaU_getfield<float>( L, index, c_motion_power );
    description.rotation_power = luaU_getfield<float>(L, index, c_maximum_torque );
    description.target_speed = luaU_getfield<float>( L, index, c_target_speed );

    // these are optional
    description.motion_bias_positive.x = luaU_optfield<float>(L, index, c_motion_bias_right, 1);
    description.motion_bias_positive.y = luaU_optfield<float>(L, index, c_motion_bias_forwards, 1);
    description.motion_bias_negative.x = luaU_optfield<float>(L, index, c_motion_bias_left, 1);
    description.motion_bias_negative.y = luaU_optfield<float>(L, index, c_motion_bias_backwards, 1);

    return description;
}


void luaU_Impl<MobileComponentDescription>::luaU_push( lua_State* L, MobileComponentDescription const & val )
{
    lua_newtable( L );
    luaU_setfield( L, -1, c_motion_power, val.motion_power );
    luaU_setfield( L, -1, c_maximum_torque, val.rotation_power );
    luaU_setfield( L, -1, c_target_speed, val.target_speed );
    luaU_setfield( L, -1, c_motion_bias_right, val.motion_bias_positive.x );
    luaU_setfield( L, -1, c_motion_bias_forwards, val.motion_bias_positive.y );
    luaU_setfield( L, -1, c_motion_bias_left, val.motion_bias_negative.x );
    luaU_setfield( L, -1, c_motion_bias_backwards, val.motion_bias_negative.y );
}


bool luaU_Impl<MobileComponentDescription>::luaU_is( lua_State* L, int index )
{
    return
        lua_istable( L, index ) &&
        luaU_fieldis<float>(L, index, c_motion_power) &&
        luaU_fieldis<float>(L, index, c_maximum_torque) &&
        luaU_fieldis<float>(L, index, c_target_speed);
}


char const luaU_Impl<ItemComponentDescription>::c_dropped_template_id[] = "dropped_template_id";
char const luaU_Impl<ItemComponentDescription>::c_equipped_template_id[] = "equipped_template_id";


ItemComponentDescription luaU_Impl<ItemComponentDescription>::luaU_check( lua_State* L, int index )
{
    Logic::ItemComponentDescription description;
    description.dropped_template = luaU_checkfield<EntityTemplateID>( L, index, c_dropped_template_id );
    description.equipped_template = luaU_checkfield<EntityTemplateID>( L, index, c_equipped_template_id );
    return description;
}


ItemComponentDescription luaU_Impl<ItemComponentDescription>::luaU_to( lua_State* L, int index )
{
    Logic::ItemComponentDescription description;
    description.dropped_template = luaU_getfield<EntityTemplateID>( L, index, c_dropped_template_id );
    description.equipped_template = luaU_getfield<EntityTemplateID>( L, index, c_equipped_template_id );
    return description;
}


void luaU_Impl<ItemComponentDescription>::luaU_push( lua_State* L, ItemComponentDescription const & val )
{
    lua_newtable( L );
    luaU_setfield( L, -1, c_dropped_template_id, val.dropped_template );
    luaU_setfield( L, -1, c_equipped_template_id, val.equipped_template );
}


bool luaU_Impl<ItemComponentDescription>::luaU_is( lua_State* L, int index )
{
    return
        lua_istable( L, index ) &&
        luaU_fieldis<EntityTemplateID>(L, index, c_dropped_template_id) &&
        luaU_fieldis<EntityTemplateID>(L, index, c_equipped_template_id);
}
