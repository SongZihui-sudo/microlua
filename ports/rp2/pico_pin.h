#ifndef C7FF4F38_DAD9_4DE2_9F53_26D0EACE115E
#define C7FF4F38_DAD9_4DE2_9F53_26D0EACE115E
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

#endif /* C7FF4F38_DAD9_4DE2_9F53_26D0EACE115E */
