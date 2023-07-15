#ifndef pico_pin_h
#define pico_pin_h

#include "pin.h"

CLASS( pico_pin )
{
    EXTENDS( pin );
    IMPLEMENTS( Ipin );
    void ( *init )( pico_pin*, int );
};

#endif
