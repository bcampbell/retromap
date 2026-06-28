#include "scripting.h"

#include <stdio.h>
extern "C"{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include "model.h"

static void pushmodel(lua_State* L, Model const& model);
static void pushmap(lua_State* L, Tilemap const& m);
static void pushcell(lua_State* L, Cell const& c);
static void pushent(lua_State* L, Ent const& c);

static void pushmodel(lua_State* L, Model const& model)
{
    lua_newtable(L);
    lua_pushstring(L, "filename");
    lua_pushstring (L, model.mapFilename.c_str());
    lua_settable(L, -3);

    // New array to hold maps
    lua_pushstring(L, "maps");
    lua_newtable(L);
    Proj const& proj = model.proj;
    int i = 0;
    for (Tilemap const& m : proj.maps) {
        // New table for this map.
        pushmap(L, m);
        lua_rawseti(L, -2, ++i);
    }
    lua_settable(L, -3);
}


static void pushmap(lua_State* L, Tilemap const& m)
{
    lua_newtable(L);
    lua_pushstring(L, "w");
    lua_pushnumber(L, m.w);
    lua_settable(L, -3);
    lua_pushstring(L, "h");
    lua_pushnumber(L, m.h);
    lua_settable(L, -3);

    lua_pushstring(L, "rows");
    lua_newtable(L);
    auto cell = m.cells.cbegin();
    for (int y = 0; y < m.h; ++y) {
        lua_newtable(L);
        for (int x = 0; x < m.w; ++x) {
            pushcell(L, *cell);
            ++cell;
            lua_rawseti(L, -2, x+1);
        } 
        lua_rawseti(L, -2, y+1);
    }
    lua_settable(L, -3);

    lua_pushstring(L, "ents");
    lua_newtable(L);
    int i = 0;
    for (auto const& ent : m.ents) {
        pushent(L, ent);
        lua_rawseti(L, -2, ++i);
    }
    lua_settable(L, -3);
}


static void pushcell(lua_State* L, Cell const& c)
{
    lua_newtable(L);
    lua_pushstring(L, "tile");
    lua_pushnumber(L, c.tile);
    lua_settable(L, -3);
    lua_pushstring(L, "ink");
    lua_pushnumber(L, c.ink);
    lua_settable(L, -3);
    lua_pushstring(L, "paper");
    lua_pushnumber(L, c.paper);
    lua_settable(L, -3);
}


static void pushent(lua_State* L, Ent const& ent)
{
    lua_newtable(L);
    for (auto const& attr : ent.attrs) {
        lua_pushstring(L, attr.name.c_str());
        lua_pushstring(L, attr.value.c_str());
        lua_settable(L, -3);
    }
}



void RunScript(const char* script, Model const& model)
{
    lua_State *L = luaL_newstate(); // Create new Lua state
    luaL_openlibs(L);               // Load Lua libraries

    pushmodel(L, model);
    lua_setglobal(L, "proj");

    // Execute Lua script
    if (luaL_dofile(L, script)) {
        printf("Error: %s\n", lua_tostring(L, -1));
    }

    lua_close(L); // Close Lua state
}



