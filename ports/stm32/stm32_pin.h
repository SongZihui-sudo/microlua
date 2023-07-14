#ifndef stm32_pin_h
#define stm32_pin_h

#include "pin.h"

CLASS( pico_pin )
{
    EXTENDS( pin );
    IMPLEMENTS( Ipin );
    void ( *init )( pico_pin*, int );
};

#endif
