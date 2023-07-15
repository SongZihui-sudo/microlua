/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#include "lprefix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <signal.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"

#if !defined( LUA_PROGNAME )
#define LUA_PROGNAME "lua"
#endif

#if !defined( LUA_INIT_VAR )
#define LUA_INIT_VAR "LUA_INIT"
#endif

#define LUA_INITVARVERSION LUA_INIT_VAR LUA_VERSUFFIX

static lua_State* globalL = NULL;

static const char* progname = LUA_PROGNAME;

#if defined( LUA_USE_POSIX ) /* { */

/*
** Use 'sigaction' when available.
*/
static void setsignal( int sig, void ( *handler )( int ) )
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags   = 0;
    sigemptyset( &sa.sa_mask ); /* do not mask any signal */
    sigaction( sig, &sa, NULL );
}

#endif /* } */

/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void l_message( const char* pname, const char* msg )
{
    if ( pname )
        lua_writestringerror( "%s: ", pname );
    lua_writestringerror( "%s\n", msg );
}

/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static int report( lua_State* L, int status )
{
    if ( status != LUA_OK )
    {
        const char* msg = lua_tostring( L, -1 );
        l_message( progname, msg );
        lua_pop( L, 1 ); /* remove message */
    }
    return status;
}

/*
** Message handler used to run all chunks
*/
static int msghandler( lua_State* L )
{
    const char* msg = lua_tostring( L, 1 );
    if ( msg == NULL )
    {                                               /* is error object not a string? */
        if ( luaL_callmeta( L, 1, "__tostring" ) && /* does it have a metamethod */
             lua_type( L, -1 ) == LUA_TSTRING )     /* that produces a string? */
            return 1;                               /* that is the message */
        else
            msg = lua_pushfstring( L, "(error object is a %s value)", luaL_typename( L, 1 ) );
    }
    luaL_traceback( L, L, msg, 1 ); /* append a standard traceback */
    return 1;                       /* return the traceback */
}

/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static int docall( lua_State* L, int narg, int nres )
{
    int status;
    int base = lua_gettop( L ) - narg;  /* function index */
    lua_pushcfunction( L, msghandler ); /* push message handler */
    lua_insert( L, base );              /* put it under function and args */
    globalL = L;                        /* to be available to 'laction' */
    status  = lua_pcall( L, narg, nres, base );
    lua_remove( L, base ); /* remove message handler from the stack */
    return status;
}

static void print_version( void )
{
    lua_writestring( LUA_COPYRIGHT, strlen( LUA_COPYRIGHT ) );
    lua_writeline();
}

/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#if !defined( LUA_PROMPT )
#define LUA_PROMPT "> "
#define LUA_PROMPT2 ">> "
#endif

#if !defined( LUA_MAXINPUT )
#define LUA_MAXINPUT 512
#endif

/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#if !defined( lua_stdin_is_tty ) /* { */

#if defined( LUA_USE_POSIX ) /* { */

#include <unistd.h>
#define lua_stdin_is_tty() isatty( 0 )

#elif defined( LUA_USE_WINDOWS ) /* }{ */

#include <io.h>
#include <windows.h>

#define lua_stdin_is_tty() _isatty( _fileno( stdin ) )

#else                        /* }{ */

/* ISO C definition */
#define lua_stdin_is_tty() 1 /* assume stdin is a tty */

#endif /* } */

#endif /* } */

/*
** lua_readline defines how to show a prompt and then read a line from
** the standard input.
** lua_saveline defines how to "save" a read line in a "history".
** lua_freeline defines how to free a line read by lua_readline.
*/
#if !defined( lua_readline ) /* { */

#if defined( LUA_USE_READLINE ) /* { */

#include <readline/readline.h>
#include <readline/history.h>
#define lua_initreadline( L ) ( ( void )L, rl_readline_name = "lua" )
#define lua_readline( L, b, p ) ( ( void )L, ( ( b ) = readline( p ) ) != NULL )
#define lua_saveline( L, line ) ( ( void )L, add_history( line ) )
#define lua_freeline( L, b ) ( ( void )L, free( b ) )

#else /* }{ */

#define lua_initreadline( L ) ( ( void )L )
#define lua_readline( L, b, p )                                                            \
    ( ( void )L,                                                                           \
      fputs( p, stdout ),                                                                  \
      fflush( stdout ),        /* show prompt */                                           \
      linenoise( b ) != NULL ) /* get line */
#define lua_saveline( L, line )                                                            \
    {                                                                                      \
        ( void )L;                                                                         \
        ( void )line;                                                                      \
    }
#define lua_freeline( L, b )                                                               \
    {                                                                                      \
        ( void )L;                                                                         \
        ( void )b;                                                                         \
    }

#endif /* } */

#endif /* } */

/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
static const char* get_prompt( lua_State* L, int firstline )
{
    if ( lua_getglobal( L, firstline ? "_PROMPT" : "_PROMPT2" ) == LUA_TNIL )
        return ( firstline ? LUA_PROMPT : LUA_PROMPT2 ); /* use the default */
    else
    { /* apply 'tostring' over the value */
        const char* p = luaL_tolstring( L, -1, NULL );
        lua_remove( L, -2 ); /* remove original value */
        return p;
    }
}

/**
 ** Initialize the serial console of ESP32
 ** Using fgets will always get null, so use esp-idf's linnose to read the line
 ** doc: https://docs.espressif.com/projects/esp-idf/zh_CN/v4.4.2/esp32/api-reference/system/console.html#id1
 **
 */
static void initialize_console( void )
{
    /* Disable buffering on stdin */
    setvbuf( stdin, NULL, _IONBF, 0 );

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_port_set_rx_line_endings( CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR );
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_port_set_tx_line_endings( CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF );

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config
    = {.baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
       .data_bits = UART_DATA_8_BITS,
       .parity    = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
       .source_clk = UART_SCLK_REF_TICK,
#else
       .source_clk = UART_SCLK_XTAL,
#endif
      };
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install( CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0 ) );
    ESP_ERROR_CHECK( uart_param_config( CONFIG_ESP_CONSOLE_UART_NUM, &uart_config ) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver( CONFIG_ESP_CONSOLE_UART_NUM );

    /* Initialize the console */
    esp_console_config_t console_config
    = {.max_cmdline_args   = 8,
       .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
       .hint_color = atoi( LOG_COLOR_CYAN )
#endif
      };
    ESP_ERROR_CHECK( esp_console_init( &console_config ) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine( 1 );

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback( &esp_console_get_completion );
    linenoiseSetHintsCallback( ( linenoiseHintsCallback* )&esp_console_get_hint );

    /* Set command history size */
    linenoiseHistorySetMaxLen( 100 );

    /* Set command maximum length */
    linenoiseSetMaxLineLen( console_config.max_cmdline_length );

    /* Don't return empty lines */
    linenoiseAllowEmpty( false );

#if CONFIG_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad( HISTORY_PATH );
#endif
}

/* mark in error messages for incomplete statements */
#define EOFMARK "<eof>"
#define marklen ( sizeof( EOFMARK ) / sizeof( char ) - 1 )

/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
static int incomplete( lua_State* L, int status )
{
    if ( status == LUA_ERRSYNTAX )
    {
        size_t lmsg;
        const char* msg = lua_tolstring( L, -1, &lmsg );
        if ( lmsg >= marklen && strcmp( msg + lmsg - marklen, EOFMARK ) == 0 )
        {
            lua_pop( L, 1 );
            return 1;
        }
    }
    return 0; /* else... */
}

/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
static int pushline( lua_State* L, int firstline )
{
    char buffer[LUA_MAXINPUT];
    char* b = buffer;
    size_t l;
    const char* prmt = get_prompt( L, firstline );
    // int readstatus   = lua_readline( L, b, prmt );
    b = linenoise( prmt );
    if ( b == NULL )
        return 0;    /* no input (prompt will be popped by caller) */
    lua_pop( L, 1 ); /* remove prompt */
    l = strlen( b );
    if ( l > 0 && b[l - 1] == '\n' )              /* line ends with newline? */
        b[--l] = '\0';                            /* remove it */
    if ( firstline && b[0] == '=' )               /* for compatibility with 5.2, ... */
        lua_pushfstring( L, "return %s", b + 1 ); /* change '=' to 'return' */
    else
        lua_pushlstring( L, b, l );
    lua_freeline( L, b );
    return 1;
}

/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static int addreturn( lua_State* L )
{
    const char* line    = lua_tostring( L, -1 ); /* original line */
    const char* retline = lua_pushfstring( L, "return %s;", line );
    int status          = luaL_loadbuffer( L, retline, strlen( retline ), "=stdin" );
    if ( status == LUA_OK )
    {
        lua_remove( L, -2 );         /* remove modified line */
        if ( line[0] != '\0' )       /* non empty? */
            lua_saveline( L, line ); /* keep history */
    }
    else
        lua_pop( L, 2 ); /* pop result from 'luaL_loadbuffer' and modified line */
    return status;
}

/*
** Read multiple lines until a complete Lua statement
*/
static int multiline( lua_State* L )
{
    for ( ;; )
    { /* repeat until gets a complete statement */
        size_t len;
        const char* line = lua_tolstring( L, 1, &len );               /* get what it has */
        int status       = luaL_loadbuffer( L, line, len, "=stdin" ); /* try it */
        if ( !incomplete( L, status ) || !pushline( L, 0 ) )
        {
            lua_saveline( L, line ); /* keep history */
            return status;           /* cannot or should not try to add continuation line */
        }
        lua_pushliteral( L, "\n" ); /* add newline... */
        lua_insert( L, -2 );        /* ...between the two lines */
        lua_concat( L, 3 );         /* join them */
    }
}

/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
static int loadline( lua_State* L )
{
    int status;
    lua_settop( L, 0 );
    if ( !pushline( L, 1 ) )
        return -1;                               /* no input */
    if ( ( status = addreturn( L ) ) != LUA_OK ) /* 'return ...' did not work? */
        status = multiline( L ); /* try as command, maybe with continuation lines */
    lua_remove( L, 1 );          /* remove line from the stack */
    lua_assert( lua_gettop( L ) == 1 );
    return status;
}

/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print( lua_State* L )
{
    int n = lua_gettop( L );
    if ( n > 0 )
    { /* any result to be printed? */
        luaL_checkstack( L, LUA_MINSTACK, "too many results to print" );
        lua_getglobal( L, "print" );
        lua_insert( L, 1 );
        if ( lua_pcall( L, n, 0, 0 ) != LUA_OK )
            l_message( progname,
                       lua_pushfstring( L, "error calling 'print' (%s)", lua_tostring( L, -1 ) ) );
    }
}

/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
static void doREPL( lua_State* L )
{
    int status;
    const char* oldprogname = progname;
    progname                = NULL; /* no 'progname' on errors in interactive mode */
    lua_initreadline( L );
    while ( ( status = loadline( L ) ) != -1 )
    {
        if ( status == LUA_OK )
            status = docall( L, 0, LUA_MULTRET );
        if ( status == LUA_OK )
            l_print( L );
        else
            report( L, status );
    }
    lua_settop( L, 0 ); /* clear stack */
    lua_writeline();
    progname = oldprogname;
}

/* }================================================================== */

/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static int pmain( lua_State* L )
{
    luaL_checkversion( L );       /* check that interpreter has correct version */
    luaL_openlibs( L );           /* open standard libraries */
    lua_gc( L, LUA_GCRESTART );   /* start GC... */
    lua_gc( L, LUA_GCGEN, 0, 0 ); /* ...in generational mode */
    if ( lua_stdin_is_tty() )
    { /* running in interactive mode? */
        print_version();
        doREPL( L ); /* do read-eval-print loop */
        printf( "1" );
    }
    printf( "2" );
    lua_pushboolean( L, 1 ); /* signal no errors */
    return 1;
}

int app_main( int argc, char** argv )
{
    initialize_console();
    int status, result;
    lua_State* L = luaL_newstate(); /* create state */
    if ( L == NULL )
    {
        l_message( argv[0], "cannot create state: not enough memory" );
        return EXIT_FAILURE;
    }
    lua_gc( L, LUA_GCSTOP );          /* stop GC while building state */
    lua_pushcfunction( L, &pmain );   /* to call 'pmain' in protected mode */
    lua_pushinteger( L, argc );       /* 1st argument */
    lua_pushlightuserdata( L, argv ); /* 2nd argument */
    status = lua_pcall( L, 2, 1, 0 ); /* do the call */
    result = lua_toboolean( L, -1 );  /* get result */
    report( L, status );
    lua_close( L );
    return ( result && status == LUA_OK ) ? EXIT_SUCCESS : EXIT_FAILURE;
}