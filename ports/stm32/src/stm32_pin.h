#ifndef stm32_pin_h
#define stm32_pin_h

#include "pin.h"

CLASS( stm32_pin )
{
    EXTENDS( pin );
    IMPLEMENTS( Ipin );
    void ( *init )( stm32_pin*, int );
};

#endif
