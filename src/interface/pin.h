#ifndef pin_H
#define pin_H

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "board.h"

#ifndef PIN_NUM
#define PIN_NUM 32
#endif

/**
 * @brief pin table
 * 
 */
struct pin_table
{
    struct pin* table[PIN_NUM];
    int size;
};

/**
 * @brief 初始化表
 * 
 * @param self pin table 对象
 */
void pin_table_init(struct pin_table* self);

/**
 * @brief 设置指定下标的 pin
 * 
 * @param self  pin table 对象
 * @param index  下标
 * @param value  值
 */
void pin_table_set_index(struct pin_table* self, size_t index, bool value);

/**
 * @brief pin struct
 * 
 */
struct pin
{
    int mIndex;
    bool mValue;
};

/**
 * @brief 初始化一个 pin
 * 
 * @param self pin 指针
 */
void pin_init(struct pin* self, int index);

/**
 * @brief 为一个 pin 设置值
 * 
 * @param self 
 * @param value 
 */
void pin_value(struct pin* self, bool value);

/**
 * @brief 设一个 pin 为高电平
 * 
 * @param self 
 */
void pin_on(struct pin* self);

/**
 * @brief 设一个 pin 为低电平
 * 
 * @param self 
 */
void pin_off(struct pin* self);

#endif
