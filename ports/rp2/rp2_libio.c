#include <assert.h>
#define liolib_c
#define LUA_LIB

#include "lprefix.h"

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include "lfs.h"
#include "pico_hal.h"

#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int f_close( lua_State* L );

#define l_seeknum int64_t
#define IO_PREFIX "_IO_"
#define IO_INPUT ( IO_PREFIX "input" )
#define IO_OUTPUT ( IO_PREFIX "output" )
#define IOPREF_LEN ( sizeof( IO_PREFIX ) / sizeof( char ) - 1 )
#define L_MAXLENNUM 200
#define MAXARGLINE 250
#define l_lockfile( f ) ( ( void )0 )
#define l_unlockfile( f ) ( ( void )0 )
#define DEBUG( format, ... )                                                               \
    printf(                                                                                \
    "File: "__FILE__                                                                       \
    ", Line: %05d: " format "/n",                                                          \
    __LINE__,                                                                              \
    ##__VA_ARGS__ )

static int io_readline( lua_State* L );

typedef struct lfs_luaL_Stream
{
    int f;                /* stream (NULL for incompletely created streams) */
    lua_CFunction closef; /* to close stream (NULL for closed streams) */
} lfs_luaL_Stream;

typedef struct
{
    int f;                      /* file being read */
    int c;                      /* current character (look ahead) */
    int n;                      /* number of elements in buffer 'buff' */
    char buff[L_MAXLENNUM + 1]; /* +1 for ending '\0' */
} RN;

typedef lfs_luaL_Stream LStream;

#define tolstream( L ) ( ( LStream* )luaL_checkudata( L, 1, LUA_FILEHANDLE ) )

#if !defined( l_checkmode )
/* accepted extensions to 'mode' in 'fopen' */
#if !defined( L_MODEEXT )
#define L_MODEEXT "b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
static int l_checkmode( const char* mode )
{
    return ( *mode != '\0' && strchr( "rwa", *( mode++ ) ) != NULL
             && ( *mode != '+' || ( ( void )( ++mode ), 1 ) ) && /* skip if char is '+' */
             ( strspn( mode, L_MODEEXT ) == strlen( mode ) ) );  /* check extensions */
}

#endif

#if !defined( l_checkmodep )
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep( m )                                                                  \
    ( ( m[0] == 'r' || m[0] == 'w' )                                                       \
      && ( m[1] == '\0' || ( ( m[1] == 'b' || m[1] == 't' ) && m[2] == '\0' ) ) )
#endif

#define isclosed( p ) ( ( p )->closef == NULL )

/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
static int aux_close( lua_State* L )
{
    LStream* p                = tolstream( L );
    volatile lua_CFunction cf = p->closef;
    p->closef                 = NULL; /* mark stream as closed */
    return ( *cf )( L );              /* close it */
}

static int f_gc( lua_State* L )
{
    LStream* p = tolstream( L );
    if ( !isclosed( p ) && p->f != 0 )
        aux_close( L ); /* ignore closed and incompletely open files */
    return 0;
}

static int f_tostring( lua_State* L )
{
    LStream* p = tolstream( L );
    if ( isclosed( p ) )
        lua_pushliteral( L, "file (closed)" );
    else
        lua_pushfstring( L, "file (%p)", p->f );
    return 1;
}

/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
static LStream* newprefile( lua_State* L )
{
    LStream* p = ( LStream* )lua_newuserdatauv( L, sizeof( LStream ), 0 );
    p->closef  = NULL; /* mark file handle as 'closed' */
    luaL_setmetatable( L, LUA_FILEHANDLE );
    return p;
}

/*
** function to close regular files
*/
static int io_fclose( lua_State* L )
{
    LStream* p = tolstream( L );
    int res    = pico_close( p->f );
    return luaL_fileresult( L, ( res == 0 ), NULL );
}

static LStream* newfile( lua_State* L )
{
    LStream* p = newprefile( L );
    p->f       = 0;
    p->closef  = &io_fclose;
    return p;
}

static int tofile( lua_State* L )
{
    LStream* p = tolstream( L );
    if ( l_unlikely( isclosed( p ) ) )
        luaL_error( L, "attempt to use a closed file" );
    lua_assert( p->f );
    return p->f;
}

static int read_chars( lua_State* L, int f, size_t n )
{
    size_t nr; /* number of chars actually read */
    char* p;
    luaL_Buffer b;
    luaL_buffinit( L, &b );
    p  = luaL_prepbuffsize( &b, n );            /* prepare buffer to read whole block */
    nr = pico_read( f, p, sizeof( char ) * n ); /* try to read 'n' chars */
    luaL_addsize( &b, nr );
    luaL_pushresult( &b ); /* close buffer */
    return ( nr > 0 );     /* true iff read something */
}

static int read_line( lua_State* L, int f, int chop )
{
    luaL_Buffer b;
    int c;
    luaL_buffinit( L, &b );
    do
    { /* may need to read several chunks to get whole line */
        char* buff = luaL_prepbuffer( &b ); /* preallocate buffer space */
        int i      = 0;
        l_lockfile( f ); /* no memory errors can happen inside the lock */
        while ( i < LUAL_BUFFERSIZE && ( c = pico_getc( f ) ) != EOF && c != '\n' )
        {
            if ( c <= 0 )
            {
                return c;
            }
            buff[i++] = c; /* read up to end of line or buffer limit */
        }
        l_unlockfile( f );
        luaL_addsize( &b, i );
    } while ( c != EOF && c != '\n' ); /* repeat until end of line */
    if ( !chop && c == '\n' )          /* want a newline and have one? */
        luaL_addchar( &b, c );         /* add ending newline to result */
    luaL_pushresult( &b );             /* close buffer */
    /* return ok if read something (either a newline or something else) */
    return ( c == '\n' || lua_rawlen( L, -1 ) > 0 );
}

/*
** Add current char to buffer (if not out of space) and read next one
*/
static int nextc( RN* rn )
{
    if ( l_unlikely( rn->n >= L_MAXLENNUM ) )
    {                       /* buffer overflow? */
        rn->buff[0] = '\0'; /* invalidate result */
        return 0;           /* fail */
    }
    else
    {
        rn->buff[rn->n++] = rn->c;              /* save current char */
        rn->c             = pico_getc( rn->f ); /* read next one */
        return 1;
    }
}

static void read_all( lua_State* L, int f )
{
    size_t nr;
    luaL_Buffer b;
    luaL_buffinit( L, &b );
    do
    { /* read file in chunks of LUAL_BUFFERSIZE bytes */
        char* p = luaL_prepbuffer( &b );
        nr      = pico_read( f, p, sizeof( char ) * LUAL_BUFFERSIZE );
        luaL_addsize( &b, nr );
    } while ( nr == LUAL_BUFFERSIZE );
    luaL_pushresult( &b ); /* close buffer */
}

static int test_eof( lua_State* L, int f )
{
    int c = pico_getc( f );
    pico_ungetc( c, f ); /* no-op when c == EOF */
    lua_pushliteral( L, "" );
    return ( c != EOF );
}

/*
** Read a sequence of (hex)digits
*/
static int readdigits( RN* rn, int hex )
{
    int count = 0;
    while ( ( hex ? isxdigit( rn->c ) : isdigit( rn->c ) ) && nextc( rn ) )
        count++;
    return count;
}

/*
** Accept current char if it is in 'set' (of size 2)
*/
static int test2( RN* rn, const char* set )
{
    if ( rn->c == set[0] || rn->c == set[1] )
        return nextc( rn );
    else
        return 0;
}

/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
static int read_number( lua_State* L, int f )
{
    RN rn;
    int count = 0;
    int hex   = 0;
    char decp[2];
    rn.f    = f;
    rn.n    = 0;
    decp[0] = lua_getlocaledecpoint(); /* get decimal point from locale */
    decp[1] = '.';                     /* always accept a dot */
    l_lockfile( rn.f );
    do
    {
        rn.c = pico_getc( rn.f );
    } while ( isspace( rn.c ) ); /* skip spaces */
    test2( &rn, "-+" );          /* optional sign */
    if ( test2( &rn, "00" ) )
    {
        if ( test2( &rn, "xX" ) )
            hex = 1; /* numeral is hexadecimal */
        else
            count = 1; /* count initial '0' as a valid digit */
    }
    count += readdigits( &rn, hex );     /* integral part */
    if ( test2( &rn, decp ) )            /* decimal point? */
        count += readdigits( &rn, hex ); /* fractional part */
    if ( count > 0 && test2( &rn, ( hex ? "pP" : "eE" ) ) )
    {                         /* exponent mark? */
        test2( &rn, "-+" );   /* exponent sign */
        readdigits( &rn, 0 ); /* exponent digits */
    }
    pico_ungetc( rn.c, rn.f ); /* unread look-ahead char */
    l_unlockfile( rn.f );
    rn.buff[rn.n] = '\0'; /* finish string */
    if ( l_likely( lua_stringtonumber( L, rn.buff ) ) )
        return 1; /* ok, it is a valid number */
    else
    {                     /* invalid format */
        lua_pushnil( L ); /* "result" to be removed */
        return 0;         /* read fails */
    }
}

static int g_read( lua_State* L, int f, int first )
{
    int nargs = lua_gettop( L ) - 1;
    int n, success;
    if ( nargs == 0 )
    { /* no arguments? */
        success = read_line( L, f, 1 );
        n       = first + 1; /* to return 1 result */
    }
    else
    {
        /* ensure stack space for all results and for auxlib's buffer */
        luaL_checkstack( L, nargs + LUA_MINSTACK, "too many arguments" );
        success = 1;
        for ( n = first; nargs-- && success; n++ )
        {
            if ( lua_type( L, n ) == LUA_TNUMBER )
            {
                size_t l = ( size_t )luaL_checkinteger( L, n );
                success  = ( l == 0 ) ? test_eof( L, f ) : read_chars( L, f, l );
            }
            else
            {
                const char* p = luaL_checkstring( L, n );
                if ( *p == '*' )
                    p++; /* skip optional '*' (for compatibility) */
                switch ( *p )
                {
                    case 'n': /* number */
                        success = read_number( L, f );
                        break;
                    case 'l': /* line */
                        success = read_line( L, f, 1 );
                        break;
                    case 'L': /* line with end-of-line */
                        success = read_line( L, f, 0 );
                        break;
                    case 'a':             /* file */
                        read_all( L, f ); /* read entire file */
                        success = 1;      /* always success */
                        break;
                    default:
                        return luaL_argerror( L, n, "invalid format" );
                }
            }
        }
    }
    if ( strcmp( pico_errmsg( success ), "Unknown error" ) )
    {
        return luaL_fileresult( L, 0, NULL );
    }
    if ( !success )
    {
        lua_pop( L, 1 );    /* remove last result */
        luaL_pushfail( L ); /* push nil instead */
    }
    return n - first;
}

static int g_write( lua_State* L, int f, int arg )
{
    int nargs  = lua_gettop( L ) - arg;
    int status = 1;
    for ( ; nargs--; arg++ )
    {
        if ( lua_type( L, arg ) == LUA_TNUMBER )
        {
            char buffer[64];
            /* optimization: could be done exactly as for strings */
            int len = lua_isinteger( L, arg ) ?
                      sprintf( buffer, LUA_INTEGER_FMT, ( LUAI_UACINT )lua_tointeger( L, arg ) ) :
                      sprintf( buffer, LUA_NUMBER_FMT, ( LUAI_UACNUMBER )lua_tonumber( L, arg ) );
            pico_write( f, buffer, strlen( buffer ) );
            status = status && ( len > 0 );
        }
        else
        {
            size_t l;
            const char* s = luaL_checklstring( L, arg, &l );
            status = status && ( pico_write( f, s, sizeof( char ) * strlen( s ) ) == l );
        }
    }
    if ( l_likely( status ) )
        return 1; /* file handle already on stack top */
    else
        return luaL_fileresult( L, status, NULL );
}

/*
** Iteration function for 'lines'.
*/
static int io_readline( lua_State* L )
{
    LStream* p = ( LStream* )lua_touserdata( L, lua_upvalueindex( 1 ) );
    int i;
    int n = ( int )lua_tointeger( L, lua_upvalueindex( 2 ) );
    if ( isclosed( p ) ) /* file is already closed? */
        return luaL_error( L, "file is already closed" );
    lua_settop( L, 1 );
    luaL_checkstack( L, n, "too many arguments" );
    for ( i = 1; i <= n; i++ ) /* push arguments to 'g_read' */
        lua_pushvalue( L, lua_upvalueindex( 3 + i ) );
    n = g_read( L, p->f, 2 );     /* 'n' is number of results */
    lua_assert( n > 0 );          /* should return at least a nil */
    if ( lua_toboolean( L, -n ) ) /* read at least one value? */
        return n;                 /* return them */
    else
    { /* first result is false: EOF or error */
        if ( n > 1 )
        { /* is there error information? */
            /* 2nd result is error message */
            return luaL_error( L, "%s", lua_tostring( L, -n + 1 ) );
        }
        if ( lua_toboolean( L, lua_upvalueindex( 3 ) ) )
        {                                              /* generator created file? */
            lua_settop( L, 0 );                        /* clear stack */
            lua_pushvalue( L, lua_upvalueindex( 1 ) ); /* push file at index 1 */
            aux_close( L );                            /* close it */
        }
        return 0;
    }
}

/*
** Auxiliary function to create the iteration function for 'lines'.
** The iteration function is a closure over 'io_readline', with
** the following upvalues:
** 1) The file being read (first value in the stack)
** 2) the number of arguments to read
** 3) a boolean, true iff file has to be closed when finished ('toclose')
** *) a variable number of format arguments (rest of the stack)
*/
static void aux_lines( lua_State* L, int toclose )
{
    int n = lua_gettop( L ) - 1; /* number of arguments to read */
    luaL_argcheck( L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments" );
    lua_pushvalue( L, 1 );         /* file */
    lua_pushinteger( L, n );       /* number of arguments to read */
    lua_pushboolean( L, toclose ); /* close/not close file when finished */
    lua_rotate( L, 2, 3 );         /* move the three values to their positions */
    lua_pushcclosure( L, io_readline, 3 + n );
}
static int io_close( lua_State* L )
{
    if ( lua_isnone( L, 1 ) )                            /* no argument? */
        lua_getfield( L, LUA_REGISTRYINDEX, IO_OUTPUT ); /* use default output */
    return f_close( L );
}

static void opencheck( lua_State* L, const char* fname, const char* mode )
{
    LStream* p = newfile( L );
    int _mode  = lfs_mode( mode );
    p->f       = pico_open( fname, _mode );
    if ( l_unlikely( p->f == 0 ) )
        luaL_error( L, "cannot open file '%s' (%s)", fname, strerror( errno ) );
}

static int getiofile( lua_State* L, const char* findex )
{
    LStream* p;
    lua_getfield( L, LUA_REGISTRYINDEX, findex );
    p = ( LStream* )lua_touserdata( L, -1 );
    if ( l_unlikely( isclosed( p ) ) )
        luaL_error( L, "default %s file is closed", findex + IOPREF_LEN );
    return p->f;
}

static int g_iofile( lua_State* L, const char* f, const char* mode )
{
    if ( !lua_isnoneornil( L, 1 ) )
    {
        const char* filename = lua_tostring( L, 1 );
        if ( filename )
            opencheck( L, filename, mode );
        else
        {
            tofile( L ); /* check that it's a valid file handle */
            lua_pushvalue( L, 1 );
        }
        lua_setfield( L, LUA_REGISTRYINDEX, f );
    }
    /* return current value */
    lua_getfield( L, LUA_REGISTRYINDEX, f );
    return 1;
}

/*
** function to close 'popen' files
*/
static int io_pclose( lua_State* L )
{
    LStream* p = tolstream( L );
    errno      = 0;
    return luaL_execresult( L, pico_close( p->f ) );
}

static int io_flush( lua_State* L )
{
    int res = luaL_fileresult( L, pico_fflush( getiofile( L, IO_OUTPUT ) ) == 0, NULL );
    return res;
}

static int io_input( lua_State* L ) { return g_iofile( L, IO_INPUT, "r" ); }

/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
static int io_lines( lua_State* L )
{
    int toclose;
    if ( lua_isnone( L, 1 ) )
        lua_pushnil( L ); /* at least one argument */
    if ( lua_isnil( L, 1 ) )
    {                                                   /* no file name? */
        lua_getfield( L, LUA_REGISTRYINDEX, IO_INPUT ); /* get default input */
        lua_replace( L, 1 );                            /* put it at index 1 */
        tofile( L ); /* check that it's a valid file handle */
        toclose = 0; /* do not close it after iteration */
    }
    else
    { /* open a new file */
        const char* filename = luaL_checkstring( L, 1 );
        opencheck( L, filename, "r" );
        lua_replace( L, 1 ); /* put file at index 1 */
        toclose = 1;         /* close it after iteration */
    }
    aux_lines( L, toclose ); /* push iteration function */
    if ( toclose )
    {
        lua_pushnil( L );      /* state */
        lua_pushnil( L );      /* control */
        lua_pushvalue( L, 1 ); /* file is the to-be-closed variable (4th result) */
        return 4;
    }
    else
        return 1;
}

static int io_output( lua_State* L )
{
    int res = g_iofile( L, IO_OUTPUT, "w" );
    return res;
}

static int io_popen( lua_State* L )
{
    const char* filename = luaL_checkstring( L, 1 );
    const char* mode     = luaL_optstring( L, 2, "r" );
    LStream* p           = newprefile( L );
    luaL_argcheck( L, l_checkmodep( mode ), 2, "invalid mode" );
    int _mode = lfs_mode( mode );
    p->f      = pico_open( filename, _mode );
    p->closef = &io_pclose;
    return ( p->f == 0 ) ? luaL_fileresult( L, 0, filename ) : 1;
}

static int io_read( lua_State* L )
{
    int res = g_read( L, getiofile( L, IO_INPUT ), 1 );
    return res;
}

static int io_tmpfile( lua_State* L )
{
    printf( "This method is not currently supported!\n" );
    return 0;
}

static int io_type( lua_State* L )
{
    LStream* p;
    luaL_checkany( L, 1 );
    p = ( LStream* )luaL_testudata( L, 1, LUA_FILEHANDLE );
    if ( p == NULL )
        luaL_pushfail( L ); /* not a file */
    else if ( isclosed( p ) )
        lua_pushliteral( L, "closed file" );
    else
        lua_pushliteral( L, "file" );
    return 1;
}

static int io_write( lua_State* L ) { return g_write( L, getiofile( L, IO_OUTPUT ), 1 ); }

static int io_open( lua_State* L )
{
    const char* filename = luaL_checkstring( L, 1 );
    const char* mode     = luaL_optstring( L, 2, "r" );
    LStream* p           = newfile( L );
    const char* md       = mode; /* to traverse/check mode */
    luaL_argcheck( L, l_checkmode( md ), 2, "invalid mode" );
    int _mode = lfs_mode( md );
    p->f      = pico_open( filename, _mode );
    return ( p->f == 0 ) ? luaL_fileresult( L, 0, filename ) : 1;
}

static int f_read( lua_State* L )
{
    int res = g_read( L, tofile( L ), 2 );
    return res;
}

static int f_write( lua_State* L )
{
    int f = tofile( L );
    lua_pushvalue( L, 1 ); /* push file at the stack top (to be returned) */
    return g_write( L, f, 2 );
}

static int f_lines( lua_State* L )
{
    tofile( L ); /* check that it's a valid file handle */
    aux_lines( L, 0 );
    return 1;
}

static int f_flush( lua_State* L )
{
    int res = luaL_fileresult( L, pico_fflush( ( int )tofile( L ) ) == 0, NULL );
    return res;
}

static int f_seek( lua_State* L )
{
    static const int mode[]              = { LFS_SEEK_SET, LFS_SEEK_CUR, LFS_SEEK_END };
    static const char* const modenames[] = { "set", "cur", "end", NULL };
    int f                                = tofile( L );
    lfs_file_t* fp                       = ( lfs_file_t* )f;
    int op                               = luaL_checkoption( L, 2, "cur", modenames );
    lua_Integer p3                       = luaL_optinteger( L, 3, 0 );
    l_seeknum offset                     = ( l_seeknum )p3;
    luaL_argcheck( L, ( lua_Integer )offset == p3, 3, "not an integer in proper range" );
    op                 = pico_lseek( f, offset, mode[op] );
    const char* errmsg = pico_errmsg( op );
    if ( strcmp( errmsg, "Unknown error" ) )
    {
        return luaL_fileresult( L, 0, NULL ); /* error */
    }
    else
    {
        lua_pushinteger( L, ( lua_Integer )pico_tell( f ) );
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
    printf( "This method is not currently supported!\n" );
    return 0;
}

/*
** functions for 'io' library
*/
static const luaL_Reg iolib[]
= { { "close", io_close }, { "flush", io_flush }, { "input", io_input },
    { "lines", io_lines }, { "open", io_open },   { "output", io_output },
    { "popen", io_popen }, { "read", io_read },   { "tmpfile", io_tmpfile },
    { "type", io_type },   { "write", io_write }, { NULL, NULL } };

/*
** methods for file handles
*/
static const luaL_Reg meth[]
= { { "read", f_read },       { "write", f_write }, { "lines", f_lines },
    { "flush", f_flush },     { "seek", f_seek },   { "close", f_close },
    { "setvbuf", f_setvbuf }, { NULL, NULL } };

/*
** metamethods for file handles
*/
static const luaL_Reg metameth[] = { { "__index", NULL }, /* place holder */
                                     { "__gc", f_gc },
                                     { "__close", f_gc },
                                     { "__tostring", f_tostring },
                                     { NULL, NULL } };

static void createmeta( lua_State* L )
{
    luaL_newmetatable( L, LUA_FILEHANDLE ); /* metatable for file handles */
    luaL_setfuncs( L, metameth, 0 );        /* add metamethods to new metatable */
    luaL_newlibtable( L, meth );            /* create method table */
    luaL_setfuncs( L, meth, 0 );            /* add file methods to method table */
    lua_setfield( L, -2, "__index" );       /* metatable.__index = method table */
    lua_pop( L, 1 );                        /* pop metatable */
}

/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
static int io_noclose( lua_State* L )
{
    LStream* p = tolstream( L );
    p->closef  = &io_noclose; /* keep file opened */
    luaL_pushfail( L );
    lua_pushliteral( L, "cannot close standard file" );
    return 2;
}

static void createstdfile( lua_State* L, const char* k, const char* fname )
{
    LStream* p = newprefile( L );
    p->f       = pico_open( fname, LFS_O_WRONLY | LFS_O_CREAT );
    pico_write( p->f, fname, strlen( fname ) );
    pico_close( p->f );
    p->closef = &io_noclose;
    if ( k != NULL )
    {
        lua_pushvalue( L, -1 );
        lua_setfield( L, LUA_REGISTRYINDEX, k ); /* add file to registry */
    }
    lua_setfield( L, -2, fname ); /* add file to module */
}

LUAMOD_API int luaopen_io( lua_State* L )
{
    luaL_newlib( L, iolib ); /* new module */
    createmeta( L );
    /* create (and set) default files */
    createstdfile( L, IO_INPUT, "stdin" );
    createstdfile( L, IO_OUTPUT, "stdout" );
    createstdfile( L, NULL, "stderr" );
    return 1;
}
