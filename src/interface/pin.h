#ifndef pin_H
#define pin_H

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "board.h"
#include "lw_oopc.h"

#ifndef PIN_NUM
#define PIN_NUM 32
#endif

/**
 * @brief pin class
 *
 */
ABS_CLASS( pin )
{
    int mIndex;
    bool mValue;
    void ( *init )( pin * t, bool index );
};

INTERFACE( Ipin )
{
    void ( *value )( pin * t, bool value );
    void ( *on )( pin * t );
    void ( *off )( pin * t );
};

#endif
