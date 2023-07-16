#include "esp32_pin.h"

#include "driver/gpio.h"

void pin_value( esp32_pin* self, bool value )
{
    assert( self );
    gpio_set_direction( self->pin.mIndex, value );
}

void pin_on( esp32_pin* self )
{
    assert( self );
    gpio_set_level( self->pin.mIndex, 1 );
}

void pin_off( esp32_pin* self )
{
    assert( self );
    gpio_set_level( self->pin.mIndex, 0 );
}

void esp32_pin_init( esp32_pin* self, bool index )
{
    assert( self );
    self->pin.mIndex = index;
    self->pin.mValue = 0;
}

CTOR( esp32_pin )
SUPER_CTOR( pin );
FUNCTION_SETTING( Ipin.value, pin_value );
FUNCTION_SETTING( Ipin.on, pin_on );
FUNCTION_SETTING( Ipin.off, pin_off );
FUNCTION_SETTING( init, esp32_pin_init );
END_CTOR
