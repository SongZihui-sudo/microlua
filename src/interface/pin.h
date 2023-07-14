#ifndef pin_H
#define pin_H

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "board.h"
#include "lw_oopc.h"

#ifndef PIN_NUM
#define PIN_NUM 32
#endif

/**
 * @brief pin class
 *
 */
ABS_CLASS( pin )
{
    int mIndex;
    bool mValue;
};

INTERFACE( Ipin )
{
    void ( *init )( pin * t, bool index );
    void ( *value )( pin * t, bool value );
    void ( *on )( pin * t );
    void ( *off )( pin * t );
};

ABS_CLASS( pin_table )
{
    pin* table[PIN_NUM];
    int size;
    void ( *init )( pin_table* );
    void ( *item_value )( pin_table, size_t, bool );
};

/**
 * @brief 初始化表
 *
 * @param self pin table 对象
 */
void pin_table_init( pin_table* self );

/**
 * @brief 设置指定下标的 pin
 *
 * @param self  pin table 对象
 * @param index  下标
 * @param value  值
 */
void pin_table_value_index( pin_table* self, size_t index, bool value );

#endif
