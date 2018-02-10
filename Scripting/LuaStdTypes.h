#pragma once

#include <lua\include\luawrapperutil.hpp>
#include <string>
#include <array>

// std string specialization
template<>
struct luaU_Impl<std::string>
{
    static std::string luaU_check( lua_State* L, int index )
    {
        return luaL_checkstring( L, index );
    }

    static std::string luaU_to( lua_State* L, int index )
    {
        return lua_tostring( L, index );
    }

    static void luaU_push( lua_State* L, std::string const & val )
    {
        lua_pushlstring( L, val.data(), val.size() );
    }

    static bool luaU_is( lua_State *L, int index )
    {
        return lua_isstring( L, index ) != 0;
    }
};

// std nullptr specialization
template<>
struct luaU_Impl<std::nullptr_t>
{
    static void luaU_push( lua_State* L, std::nullptr_t const & )
    {
        lua_pushnil( L );
    }

    static bool luaU_is( lua_State *L, int index)
    {
        return lua_isnil( L, index );
    }
};


template<typename Type, size_t N>
struct luaU_Impl<std::array<Type, N>>
{
    static std::array<Type, N> luaU_check( lua_State* L, int index )
    {
        std::array<Type, N> result{};
        index = lua_absindex(L, index);
        if(N != lua_rawlen(L, index))
        {
            luaL_error( L, "Incorrect array size." );
        }
        for (auto i = 0u; i < N; ++i)
        {
            lua_rawgeti(L, index, i + 1u);
            result[i] = ::luaU_check<Type>(L, -1);
        }
        return result;
    }

    static std::array<Type, N> luaU_to( lua_State* L, int index )
    {
        std::array<Type, N> result{};
        index = lua_absindex(L, index);
        for (auto i = 0u; i < N; ++i)
        {
            lua_rawgeti(L, index, i + 1u);
            result[i] = ::luaU_to<Type>(L, -1);
        }
        return result;
    }

    static void luaU_push( lua_State* L, std::array<Type, N> const & val )
    {
        lua_createtable (L, N, 0);
        for (auto i = 0u; i < N; ++i)
        {
            ::luaU_push(val[i]);
            lua_rawseti(L, -2, i + 1);
        }
    }

    static bool luaU_is( lua_State *L, int index )
    {
        auto result = N != lua_rawlen(L, index);
        for (auto i = 0u; result && i < N; ++i)
        {
            result = ::luaU_is<Type>(L, index, i + 1u);
        }
        return result;
    }
};

