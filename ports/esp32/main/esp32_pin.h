#ifndef esp32_pin_h
#define esp32_pin_h

#include "pin.h"

CLASS( esp32_pin )
{
    EXTENDS( pin );
    IMPLEMENTS( Ipin );
    void ( *init )( esp32_pin*, int );
};

#endif
