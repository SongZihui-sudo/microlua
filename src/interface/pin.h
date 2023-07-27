#ifndef pin_H
#define pin_H

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "lua.h"

/**
 * @brief pin class
 *
 */
struct pin
{
    int mIndex;
    bool mValue;
    int mode;
};

int pin_init( lua_State* L );

int pin_on( lua_State* L );

int pin_off( lua_State* L );

int pin_value( lua_State* L );

int pin_mode( lua_State* L );

int pin_status( lua_State* L );

#endif
