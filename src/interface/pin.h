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
    void ( *init )( pin*, bool );
};

INTERFACE( Ipin )
{
    void ( *value )( pin*, bool );
    void ( *on )( pin* );
    void ( *off )( pin* );
};

#endif
