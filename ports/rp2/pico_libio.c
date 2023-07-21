#ifndef pico_libio_c
#define pico_libio_c

#include "lua.h"

#include "lauxlib.h"
#include "lfs.h"
#include "pico_hal.h"

static int io_open( lua_State* L )
{
    const char* filename = luaL_checkstring( L, 1 );
    const char* mode     = luaL_optstring( L, 2, "r" );
    luaL_Stream* p       = newfile( L );
    const char* md       = mode; /* to traverse/check mode */
    luaL_argcheck( L, l_checkmode( md ), 2, "invalid mode" );
    int lfs_mode = LFS_O_RDONLY;
    if ( !strcmp( md, "w" ) )
    {
        lfs_mode = LFS_O_WRONLY;
    }
    else if ( !strcmp( md, "r" ) )
    {
        lfs_mode = LFS_O_RDONLY;
    }
    else if ( !strcmp( md, "a" ) )
    {
        lfs_mode = LFS_O_APPEND;
    }
    else if ( !strcmp( md, "w+" ) )
    {
        lfs_mode = LFS_O_RDWR | LFS_O_CREAT;
    }
    else if ( !strcmp( md, "r+" ) )
    {
        lfs_mode = LFS_O_RDWR;
    }
    else if ( !strcmp( md, "a+" ) )
    {
        lfs_mode = LFS_O_APPEND | LFS_O_RDWR | LFS_O_CREAT;
    }
    else
    {
        return -1;
    }
    p->f = ( FILE* )pico_open( filename, lfs_mode );
    return ( p->f == NULL ) ? luaL_fileresult( L, 0, filename ) : 1;
}

static int f_read( lua_State* L )
{
    printf( "This method is not currently supported!" );
    return 0;
}

static int f_write( lua_State* L )
{
    printf( "This method is not currently supported!" );
    return 0;
}

static int f_lines( lua_State* L )
{
    printf( "This method is not currently supported!" );
    return 0;
}

static int f_flush( lua_State* L )
{
    return luaL_fileresult( L, pico_fflush( tofile( L ) ) == 0, NULL );
}

static int f_seek( lua_State* L )
{
    static const int mode[]              = { SEEK_SET, SEEK_CUR, SEEK_END };
    static const char* const modenames[] = { "set", "cur", "end", NULL };
    FILE* f                              = tofile( L );
    int op                               = luaL_checkoption( L, 2, "cur", modenames );
    lua_Integer p3                       = luaL_optinteger( L, 3, 0 );
    l_seeknum offset                     = ( l_seeknum )p3;
    luaL_argcheck( L, ( lua_Integer )offset == p3, 3, "not an integer in proper range" );
    op = pico_lseek( ( lfs_file_t* )f, offset, mode[op] );
    if ( l_unlikely( op ) )
        return luaL_fileresult( L, 0, NULL ); /* error */
    else
    {
        lua_pushinteger( L, ( lua_Integer )l_ftell( f ) );
        return 1;
    }
}

static int f_close( lua_State* L )
{
    tofile( L ); /* make sure argument is an open stream */
    return aux_close( L );
}

static int f_setvbuf( lua_State* L )
{
    printf( "This method is not currently supported!" );
    return 0;
}

#endif
