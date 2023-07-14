#include "pin.h"
#include <assert.h>

void pin_table_init( pin_table* self )
{
    assert( self );
    self->size = PIN_NUM;
}

void pin_table_value_index( pin_table* self, size_t index, bool value )
{
    assert( self );
    if ( index < 0 || index > self->size )
    {
        printf( "index is out of the range!\n" );
        return;
    }
    self->table[index]->value( self->table[index], value );
}

ABS_CTOR( pin_table )
FUNCTION_SETTING( init, pin_table_init );
FUNCTION_SETTING( item_value, pin_table_value_index )
END_ABS_CTOR

void pin_init( pin* self, int index )
{
    assert( self );
    self->mIndex = index;
    self->mValue = 0;
}

ABS_CTOR( pin )
FUNCTION_SETTING( init, pin_init );
END_ABS_CTOR
