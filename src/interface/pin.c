#include "pin.h"
#include <assert.h>

#include "lua.h"
#include "lauxlib.h"

void pin_init( pin* self, bool index )
{
    assert( self );
    self->mIndex = index;
    self->mValue = 0;
}

bool pin_status( pin* self ) { return self->mValue; }

ABS_CTOR( pin )
FUNCTION_SETTING( init, pin_init );
FUNCTION_SETTING( status, pin_status );
END_ABS_CTOR

/* Lua lib */

int gpio_new( lua_State* l )
{
    lua_Integer _pin_index = lua_tointeger( l, 1 );
    return 0;
}

int gpio_on( lua_State* l )
{
    lua_Integer _pin_index = lua_tointeger( l, 1 );
    return 0;
}

int gpio_off( lua_State* l )
{
    lua_Integer _pin_index = lua_tointeger( l, 1 );
    return 0;
}

int gpio_value( lua_State* l )
{
    lua_Integer _pin_index = lua_tointeger( l, 1 );
    return 0;
}

static luaL_Reg gpio[]
= { { "gpio", gpio_new }, { "on", gpio_on }, { "off", gpio_off }, { "value", gpio_value }, { NULL, NULL } };

int luaopen_gpio( lua_State* L )
{
    luaL_newlib( L, gpio );
    return 1;
}
