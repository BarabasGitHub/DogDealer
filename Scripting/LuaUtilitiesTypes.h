#pragma once

#include "LuaMathTypes.h" // for the noise parameters

#include <Utilities\Handle.h>
#include <Utilities\MinMax.h>
#include <Utilities\SimplexNoise.h>

#include <lua\include\luawrapperutil.hpp>

template<typename Type>
struct luaU_Impl<MinMax<Type>>
{
    static MinMax<Type> luaU_check( lua_State* L, int index )
    {
        return{ luaU_checkfield<Type>( L, index, "min" ),
            luaU_checkfield<Type>( L, index, "max" ) };
    }


    static MinMax<Type> luaU_to( lua_State* L, int index )
    {
        return{ luaU_getfield<Type>( L, index, "min" ),
            luaU_getfield<Type>( L, index, "max" ) };
    }


    static void luaU_push( lua_State* L, MinMax<Type> const & val )
    {
        lua_newtable( L );
        luaU_setfield( L, -1, "min", val.min );
        luaU_setfield( L, -1, "max", val.max );
    }


    static bool luaU_is( lua_State * L, int index )
    {
        return lua_istable( L, index ) &&
            luaU_fieldis<Type>( L, index, "min" ) &&
            luaU_fieldis<Type>( L, index, "max" );
    }
};


// needed for the luaU_to luaU_push functions
template<typename Type>
struct luaU_Impl<Handle<Type>>
{
    static Handle<Type> luaU_check( lua_State* L, int index )
    {
        return{ luaU_checkfield<typename Handle<Type>::index_t>( L, index, "index" ),
            luaU_checkfield<typename Handle<Type>::generation_t>( L, index, "generation" ) };
    }


    static Handle<Type> luaU_to( lua_State* L, int index )
    {
        return{ luaU_getfield<typename Handle<Type>::index_t>( L, index, "index" ),
            luaU_getfield<typename Handle<Type>::generation_t>( L, index, "generation" ) };
    }


    static void luaU_push( lua_State* L, const Handle<Type>& val )
    {
        lua_newtable( L );
        luaU_setfield( L, -1, "index", val.index );
        luaU_setfield( L, -1, "generation", val.generation );
    }


    static bool luaU_is( lua_State * L, int index )
    {
        return lua_istable( L, index ) &&
            luaU_fieldis<typename Handle<Type>::index_t>( L, index, "index" ) &&
            luaU_fieldis<typename Handle<Type>::generation_t>( L, index, "generation" );
    }
};


template<typename Type>
struct luaU_Impl< NoiseParameters<Type> >
{
    static NoiseParameters<Type> luaU_check( lua_State* L, int index )
    {
        return{ luaU_checkfield<Type>( L, index, "offset" ),
            luaU_checkfield<Type>( L, index, "frequency" ),
            luaU_checkfield<float>( L, index, "amplitude" ) };
    }

    static NoiseParameters<Type> luaU_to( lua_State* L, int index )
    {
        return{ luaU_getfield<Type>( L, index, "offset" ),
            luaU_getfield<Type>( L, index, "frequency" ),
            luaU_getfield<float>( L, index, "amplitude" ) };
    }

    static void luaU_push( lua_State* L, NoiseParameters<Type>&val )
    {
        lua_newtable( L );
        luaU_setfield( L, -1, "offset", val.offset );
        luaU_setfield( L, -1, "frequency", val.frequency );
        luaU_setfield( L, -1, "amplitude", val.amplitude );
    }

    static bool luaU_is( lua_State * L, int index )
    {
        return lua_istable( L, index ) &&
            luaU_fieldis<Type>( L, index, "offset" ) &&
            luaU_fieldis<Type>( L, index, "frequency" ) &&
            luaU_fieldis<float>( L, index, "amplitude" );
    }
};
