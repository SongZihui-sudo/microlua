#include "pico_pin.h"

void pin_value( pin* self, bool value ) {}

void pin_on( pin* self ) {}

void pin_off( pin* self ) {}

void pico_pin_init( pico_pin* self, int index )
{
    assert( self );
    self->pin.mIndex = index;
    self->pin.mValue = 0;
}

CTOR( pico_pin )
SUPER_CTOR( pin );
FUNCTION_SETTING( Ipin.value, pin_value );
FUNCTION_SETTING( Ipin.on, pin_on );
FUNCTION_SETTING( Ipin.off, pin_off );
FUNCTION_SETTING( init, pico_pin_init );
END_CTOR
