#ifndef fs_h
#define fs_h

#include "lw_oopc.h"

#define MAXSUBITM 10

enum t_fs_item
{
    FILE,
    DIR
};

CLASS(fs_item)
{
    enum t_fs_item mType;
    const char* mName;
    fs_item* mSubitem[MAXSUBITM];
    void (*init)(fs_item*, const char*, enum t_fs_item);
};

ABS_CLASS(fs)
{
    void (*init)(fs);
    int (*cd)(fs , const char*);
    int (*mkdir)(fs , const char*);
    int (*mv)(fs , const char* , ...);
    int (*cp)(fs , const char* , ...);
    int (*del)(fs , const char* , ...);
    int (*touch)(fs , const char* );
    int (*list)(fs , const char* );

    fs_item mRoot;
};

INTERFACE(Ifs)
{
    void (*setup)();
};

#endif