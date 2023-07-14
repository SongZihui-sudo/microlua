#include "stm_pin.h"

void pin_value( pin* self, bool value ) {}

void pin_on( pin* self ) {}

void pin_off( pin* self ) {}

void pin_init( pin* self, int index )
{
    assert( self );
    self->mIndex = index;
    self->mValue = 0;
}

CTOR( pico_pin )
SUPER_CTOR( pin );
FUNCTION_SETTING( Ipin.value, pin_value );
FUNCTION_SETTING( Ipin.on, pin_on );
FUNCTION_SETTING( Ipin.off, pin_off );
FUNCTION_SETTING( init, pin_init );
END_CTOR
