#include "pico_posix.h"
#include "lfs.h"
#include "pico_hal.h"
#include <string.h>
#include <stdio.h>

FILE* lfs_fopen( const char* filename, const char* mode )
{
    int lfs_mode = LFS_O_RDONLY;
    if ( !strcmp( mode, "w" ) )
    {
        lfs_mode = LFS_O_WRONLY;
    }
    else if ( !strcmp( mode, "r" ) )
    {
        lfs_mode = LFS_O_RDONLY;
    }
    else if ( !strcmp( mode, "a" ) )
    {
        lfs_mode = LFS_O_APPEND;
    }
    else if ( !strcmp( mode, "w+" ) )
    {
        lfs_mode = LFS_O_RDWR | LFS_O_CREAT;
    }
    else if ( !strcmp( mode, "r+" ) )
    {
        lfs_mode = LFS_O_RDWR;
    }
    else if ( !strcmp( mode, "a+" ) )
    {
        lfs_mode = LFS_O_APPEND | LFS_O_RDWR | LFS_O_CREAT;
    }
    else
    {
        return NULL;
    }
    /* 尝试打开文件 */
    lfs_file_t* fp = ( lfs_file_t* )pico_open( filename, lfs_mode );

    /* 如果打开失败，则输出错误信息 */
    if ( fp != LFS_ERR_OK )
    {
        printf( "Can not Open file! %s\n", filename );
        return NULL;
    }

    /* 返回文件指针 */
    return fp;
}

int lfs_fclose( FILE* stream ) { return pico_close( stream ); }

size_t lfs_fread( void* ptr, size_t size, size_t nitems, FILE* stream )
{
    return pico_read( stream, ptr, size );
}

size_t lfs_fwrite( void* ptr, size_t size, size_t nitems, FILE* stream )
{
    return pico_write( stream, ptr, size );
}

void funlockfile( FILE* file ) {}

void lfs_flockfile( FILE* file ) {}
