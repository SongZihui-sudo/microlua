#include "pin.h"
#include <assert.h>

void pin_table_init(struct pin_table* self)
{
    assert(self);
    self->size = PIN_NUM;
}

void pin_init(struct pin* self, int index)
{
    assert(self);
    self->mIndex = index;
    self->mValue = 0;
}