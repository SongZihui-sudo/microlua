#ifndef pin_H
#define pin_H

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "lw_oopc.h"

/**
 * @brief pin class
 *
 */
ABS_CLASS( pin )
{
    int mIndex;
    bool mValue;
    void ( *init )( pin*, bool );
    bool ( *status )( pin* );
};

INTERFACE( Ipin )
{
    void ( *value )( pin*, bool );
    void ( *on )( pin* );
    void ( *off )( pin* );
};

#endif
