#include "pin.h"
#include <assert.h>

#include "lauxlib.h"

int pin_init( lua_State* L ) {}

static luaL_Reg gpio[]
= { { "init", pin_init },   { "on", pin_on },         { "off", pin_off },
    { "value", pin_value }, { "status", pin_status }, { "set_mdoe", pin_mode },
    { NULL, NULL } };

int luaopen_gpio( lua_State* L )
{
    luaL_newlib( L, gpio );
    return 1;
}
