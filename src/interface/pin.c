#include "pin.h"
#include <assert.h>

void pin_init( pin* self, int index )
{
    assert( self );
    self->mIndex = index;
    self->mValue = 0;
}

ABS_CTOR( pin )
FUNCTION_SETTING( init, pin_init );
END_ABS_CTOR
